/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "controller.h"
#include "ds4.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    int axes[AXIS_MAX] = {0};
    s_report_packet reports[MAX_REPORTS];
    memset(reports, 0, sizeof(reports));
    controller_init_report(C_TYPE_DS4, &reports[0].value);
    controller_build_report(C_TYPE_DS4, axes, reports);
    assert(reports[0].value.ds4.X == 128);
    assert(!(reports[0].value.ds4.HatAndButtons & DS4_CROSS_MASK));
    axes[ds4a_cross] = 255;
    controller_build_report(C_TYPE_DS4, axes, reports);
    assert(reports[0].value.ds4.HatAndButtons & DS4_CROSS_MASK);
    axes[ds4a_cross] = 0;
    controller_build_report(C_TYPE_DS4, axes, reports);
    assert(!(reports[0].value.ds4.HatAndButtons & DS4_CROSS_MASK));
    puts("Upstream DS4 report press/release test passed");
    return 0;
}
