CC ?= cc
CFLAGS ?= -O2
CFLAGS += -std=c11 -Wall -Wextra -Werror
PSMOVE := $(abspath ../../psmoveapi)
BUILD := build

.PHONY: test hardware-test clean

$(BUILD):
	mkdir -p $@

$(BUILD)/test_ds4_feature: ds4_feature.c ds4_feature.h test_ds4_feature.c | $(BUILD)
	$(CC) $(CFLAGS) ds4_feature.c test_ds4_feature.c -o $@

$(BUILD)/test_ds4_report: test_ds4_report.c ../shared/gimxcontroller/src/controller.c ../shared/gimxcontroller/src/ds4.c | $(BUILD)
	$(CC) $(CFLAGS) -I../shared/gimxcontroller/include $^ -o $@

$(BUILD)/test_ds4_hid: ds4_feature.c ds4_feature.h test_ds4_hid.c | $(BUILD)
	$(CC) $(CFLAGS) -I$(PSMOVE)/external/hidapi/hidapi ds4_feature.c test_ds4_hid.c \
		$(PSMOVE)/build/libpsmoveapi.dylib -Wl,-rpath,$(PSMOVE)/build -o $@

$(BUILD)/bridge: bridge.c ds4_feature.c ds4_feature.h ../shared/gimxcontroller/src/controller.c ../shared/gimxcontroller/src/ds4.c | $(BUILD)
	$(CC) $(CFLAGS) -I../shared/gimxcontroller/include -I$(PSMOVE)/include -I$(PSMOVE)/build -I$(PSMOVE)/external/hidapi/hidapi \
		bridge.c ds4_feature.c ../shared/gimxcontroller/src/controller.c ../shared/gimxcontroller/src/ds4.c \
		$(PSMOVE)/build/libpsmoveapi.dylib -Wl,-rpath,$(PSMOVE)/build -o $@

test: $(BUILD)/test_ds4_feature $(BUILD)/test_ds4_report
	./$(BUILD)/test_ds4_feature
	./$(BUILD)/test_ds4_report

hardware-test: $(BUILD)/test_ds4_hid
	./$(BUILD)/test_ds4_hid

clean:
	rm -rf $(BUILD)
