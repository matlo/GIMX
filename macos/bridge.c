/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "controller.h"
#include "ds4.h"
#include "ds4_feature.h"
#include "hidapi.h"
#include "psmove.h"
#include <IOKit/serial/ioss.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t stop;
static void on_signal(int n) { (void)n; stop = 1; }
static uint64_t millis(void) {
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000 + (uint64_t)t.tv_nsec / 1000000;
}
static int write_all(int fd, const void *data, size_t len) {
    const unsigned char *p = data;
    uint64_t deadline = millis() + 1000;
    while (len && !stop) {
        ssize_t n = write(fd, p, len);
        if (n > 0) { p += n; len -= (size_t)n; continue; }
        if (n < 0 && errno != EAGAIN && errno != EINTR) return -1;
        if (millis() >= deadline) return -1;
        struct pollfd f = { .fd = fd, .events = POLLOUT };
        if (poll(&f, 1, 20) < 0 && errno != EINTR) return -1;
    }
    return len ? -1 : 0;
}
static int packet(int fd, uint8_t type, const void *data, size_t len) {
    uint8_t frame[256];
    if (len > 254) return -1;
    frame[0] = type; frame[1] = (uint8_t)len;
    if (len) memcpy(frame + 2, data, len);
    return write_all(fd, frame, len + 2);
}
static int get_feature(void *d, unsigned char *b, size_t n) { return hid_get_feature_report(d, b, n); }
static int set_feature(void *d, const unsigned char *b, size_t n) { return hid_send_feature_report(d, b, n); }
static int handle_packet(int fd, hid_device *ds4, uint8_t type, const uint8_t *data, size_t len) {
    if (type == 0x33) { fprintf(stderr, "Adapter started (status %u).\n", len ? data[0] : 0); return 0; }
    if (type == 0x44) {
        uint8_t response[254];
        int n = ds4_feature_transfer(ds4, get_feature, set_feature, data, len, response, sizeof(response));
        if (n < 0) {
            fprintf(stderr, "DS4 control relay failed: request=%02x/%02x report=%02x size=%zu\n", len > 0 ? data[0] : 0, len > 1 ? data[1] : 0, len > 2 ? data[2] : 0, len);
            if (len >= 1 && (data[0] & 0x80)) (void)packet(fd, 0x44, NULL, 0);
            return -1;
        }
        fprintf(stderr, "DS4 feature %s 0x%02x: %d bytes\n", data[0] & 0x80 ? "GET" : "SET", data[2], n);
        if (data[0] & 0x80) return packet(fd, 0x44, response, (size_t)n);
        return 0;
    }
    if (type != 0xee && type != 0x99) fprintf(stderr, "Adapter packet %02x (%zu bytes)\n", type, len);
    return 0;
}
int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s /dev/cu.usbserial-...\n", argv[0]); return 2; }
    signal(SIGINT, on_signal); signal(SIGTERM, on_signal);
    int rc = 1, fd = -1, have_term = 0, started = 0;
    struct termios old, cfg;
    PSMove *move = NULL;
    hid_device *ds4 = NULL;
    s_report_packet report[MAX_REPORTS]; int axes[AXIS_MAX] = {0};
    memset(report, 0, sizeof(report));
    if (!psmove_init(PSMOVE_CURRENT_VERSION) || !(move = psmove_connect()) || psmove_connection_type(move) != Conn_Bluetooth) {
        fputs("Bluetooth PS Move unavailable.\n", stderr); goto done;
    }
    ds4 = hid_open(0x054c, 0x09cc, NULL);
    if (!ds4) ds4 = hid_open(0x054c, 0x05c4, NULL);
    if (!ds4) { fputs("USB DS4 unavailable.\n", stderr); goto done; }
    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0 || tcgetattr(fd, &old) < 0) { perror("serial open"); goto done; }
    have_term = 1; cfg = old; cfmakeraw(&cfg);
    cfg.c_cflag = (cfg.c_cflag & ~(CSIZE | PARENB | CSTOPB)) | CS8 | CLOCAL | CREAD;
    cfsetispeed(&cfg, B9600); cfsetospeed(&cfg, B9600);
    speed_t speed = 500000;
    if (tcsetattr(fd, TCSANOW, &cfg) < 0 || ioctl(fd, IOSSIOSPEED, &speed) < 0) { perror("serial configuration"); goto done; }
    tcflush(fd, TCIOFLUSH);
    controller_init_report(C_TYPE_DS4, &report[0].value);
    if (packet(fd, 0x33, NULL, 0) < 0) { fputs("Cannot start adapter.\n", stderr); goto done; }
    started = 1;
    fputs("Bridge active: Move PS, Cross, Circle, Square and Triangle map to DS4 buttons. Ctrl-C stops.\n", stderr);
    uint8_t input[1024]; size_t used = 0; uint64_t last_input = millis(), next_report = 0;
    unsigned int last_buttons = 0;
    while (!stop) {
        if (psmove_poll(move)) { last_buttons = psmove_get_buttons(move); last_input = millis(); }
        if (millis() - last_input > 300) last_buttons = 0;
        axes[ds4a_cross] = (last_buttons & Btn_CROSS) ? 255 : 0;
        axes[ds4a_circle] = (last_buttons & Btn_CIRCLE) ? 255 : 0;
        axes[ds4a_triangle] = (last_buttons & Btn_TRIANGLE) ? 255 : 0;
        axes[ds4a_square] = (last_buttons & Btn_SQUARE) ? 255 : 0;
        axes[ds4a_ps] = (last_buttons & Btn_PS) ? 255 : 0;
        axes[ds4a_share] = (last_buttons & Btn_SELECT) ? 255 : 0;
        axes[ds4a_options] = (last_buttons & Btn_START) ? 255 : 0;
        if (millis() >= next_report) {
            unsigned int index = controller_build_report(C_TYPE_DS4, axes, report);
            report[index].value.ds4.report_id = DS4_USB_HID_IN_REPORT_ID;
            if (packet(fd, 0xff, &report[index].value.ds4, DS4_USB_INTERRUPT_PACKET_SIZE) < 0) { fputs("Serial write failed.\n", stderr); goto done; }
            next_report = millis() + 10;
        }
        struct pollfd p = { .fd = fd, .events = POLLIN };
        int ready = poll(&p, 1, 2);
        if (ready < 0 && errno != EINTR) { perror("serial poll"); goto done; }
        if (p.revents & (POLLERR | POLLHUP | POLLNVAL)) { fputs("Serial disconnected.\n", stderr); goto done; }
        if (ready > 0 && (p.revents & POLLIN)) {
            ssize_t n = read(fd, input + used, sizeof(input) - used);
            if (n > 0) used += (size_t)n;
            else if (n < 0 && errno != EAGAIN && errno != EINTR) { perror("serial read"); goto done; }
            while (used >= 2 && used >= (size_t)input[1] + 2) {
                size_t size = (size_t)input[1] + 2;
                if (handle_packet(fd, ds4, input[0], input + 2, input[1]) < 0) goto done;
                memmove(input, input + size, used - size); used -= size;
            }
            if (used == sizeof(input)) { fputs("Invalid serial packet.\n", stderr); goto done; }
        }
    }
    rc = 0;
done:
    if (fd >= 0 && started) {
        memset(axes, 0, sizeof(axes)); controller_build_report(C_TYPE_DS4, axes, report);
        report[0].value.ds4.report_id = DS4_USB_HID_IN_REPORT_ID;
        stop = 0; (void)packet(fd, 0xff, &report[0].value.ds4, DS4_USB_INTERRUPT_PACKET_SIZE);
        (void)packet(fd, 0x55, NULL, 0);
    }
    if (fd >= 0 && have_term) tcsetattr(fd, TCSANOW, &old);
    if (fd >= 0) close(fd);
    if (ds4) hid_close(ds4);
    if (move) psmove_disconnect(move);
    return rc;
}
