#include <stdlib.h>
#include <string.h>

#include "hid.h"
#include "pcprog.h"

#define GPPKG_INPUT_REPORT      0x01
#define GPPKG_OUTPUT_REPORT     0x04
#define GPPKG_ENTER_CAPTURE     0x07
#define GPPKG_LEAVE_CAPTURE     0x08

int8 gpppcprog_send(uint8 type, uint8 *data, uint16 lenght);

int8 gppcprog_connected_flag = 0;

int8 gppcprog_connect() {
    uint16 r;
    
    gppcprog_disconnect();
    
    // Connect to GPP
    r = (uint8) rawhid_open(1, 0x2508, 0x0001, 0xFFAB, 0x0200);
    if(r <= 0) return(r);
    gppcprog_connected_flag = 1;
    
    // Enter Capture Mode
    r = gpppcprog_send(GPPKG_ENTER_CAPTURE, NULL, 0);
    if(r <= 0) { gppcprog_disconnect(); return(r); }
    
    return(1);
}

int8 gppcprog_connected() {
    return(gppcprog_connected_flag);
}

void gppcprog_disconnect() {
    if(gppcprog_connected_flag) {
        // Leave Capture Mode
        gpppcprog_send(GPPKG_LEAVE_CAPTURE, NULL, 0);
        
        // Disconnect to GPP
        rawhid_close(0);
        gppcprog_connected_flag = 0;
    }
    return;
} 

int8 gpppcprog_input(struct gppReport *report, int timeout) {
    int bytesReceived;
    uint8 rcvBuf[64], i;

    if(!gppcprog_connected_flag || report == NULL) return(-1);
    bytesReceived = rawhid_recv(0, rcvBuf, 64, timeout);
    if(bytesReceived < 0) { 
        gppcprog_disconnect();
        return(-1);
    } else if(bytesReceived && rcvBuf[0] == GPPKG_INPUT_REPORT) {
        int8 *rep01 = (int8 *) (rcvBuf + 7);
        report->controller = *rep01++;
		report->console = *rep01++;
		report->led[0] = *rep01++;
		report->led[1] = *rep01++;
		report->led[2] = *rep01++;
		report->led[3] = *rep01++;
		report->rumble[0] = *rep01++;
		report->rumble[1] = *rep01++;
		report->battery_level = *rep01++;
		for(i=0; i<REP_IO_COUNT; i++) {
			report->input[i] = *rep01++;
		}
        return(1);
    }
    return(0);
}

int8 gpppcprog_output(int8 *output) {
    uint8 outputReport[REP_IO_COUNT+6];
    memset(outputReport, 0x00, REP_IO_COUNT+6);
    memcpy(outputReport, (uint8 *) output, REP_IO_COUNT);
    return(gpppcprog_send(GPPKG_OUTPUT_REPORT, outputReport, REP_IO_COUNT+6));
}

int8 gpppcprog_send(uint8 type, uint8 *data, uint16 lenght) {
    uint8 sndBuf[64];
    uint16 sndLen;
    uint16 i = 0;

    *(sndBuf+0) = type;						    // Report type
    *((uint16 *)(sndBuf+1)) = lenght;           // Total length
    *(sndBuf+3) = 1;							// First Packet
    do {										// Data
        if(lenght) {
            sndLen = (((i + 60) < lenght) ? 60 : (lenght - i));
            memcpy(sndBuf+4, data+i, sndLen);
            i += sndLen;
        }
        if(rawhid_send(0, sndBuf, 64, 1000) == -1) return(0);
        if(*(sndBuf+3) == 1) *(sndBuf+3) = 0;
    } while(i < lenght);
    return(1);
}

