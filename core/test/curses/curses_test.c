#include <unistd.h>
#include <time.h>
#include <gimxcommon/include/gperf.h>
#include "calibration.h"
#include "gimx.h"
#include <controller.h>
#include <stats.h>
#include <display.h>

inline s_mouse_cal* cal_get_mouse(int mouse __attribute__((unused)), int conf __attribute__((unused)))
{
  return NULL;
}

static s_adapter ladapter = { .ctype = C_TYPE_DS4 };

inline s_adapter * adapter_get(unsigned char adapter __attribute__((unused)))
{
  return &ladapter;
}

int current_mouse = 0;
int current_conf = 0;
e_current_cal current_cal = NONE;

GE_MK_Mode ginput_get_mk_mode()
{
  return GE_MK_MODE_SINGLE_INPUT;
}

const char * ginput_mouse_name(int id __attribute__((unused)))
{
  return NULL;
}

int ginput_mouse_virtual_id(int id __attribute__((unused)))
{
  return 0;
}

s_gimx_params gimx_params = { .debug.stats = 0 };

#define NBSAMPLES 1

#define SAMPLETYPE \
    struct { \
            gtime now; \
            gtime delta; \
        }

GPERF_INST(display, SAMPLETYPE, NBSAMPLES);

int makerandom(int min, int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

static void dump(unsigned char * packet, unsigned char length)
{
  int i;
  for (i = 0; i < length; ++i)
  {
    if(i && !(i%8))
    {
      printf("\n");
    }
    printf("0x%02x ", packet[i]);
  }
  printf("\n");
}

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused)))
{
  unsigned int i;
  display_init();
  int axes[AXIS_MAX] = {16,16,0,0};

  srand(time(NULL));

  struct stats * s = stats_init(E_STATS_TYPE_CONTROLLER);

  for(i = 0; i < NBSAMPLES; ++i)
  {
    unsigned j, k;
    for (j = rel_axis_0; j <= rel_axis_max; ++j)
    {
      axes[j] = 0;
    }
    for (k = 0; k < 8; ++k)
    {
      axes[makerandom(rel_axis_0, rel_axis_max)] = makerandom(-256, 256);
    }
    for (j = abs_axis_0; j <= abs_axis_0; ++j)
    {
      axes[j] = 0;
    }
    for (k = 0; k < 4; ++k)
    {
      axes[makerandom(abs_axis_0, abs_axis_max)] = makerandom(0, 255);
    }
    GPERF_START(display);
    display_run(adapter_get(0)->ctype, axes, s);
    GPERF_END(display);
    stats_update(s);
    getchar();
  }

  stats_clean(s);

  display_end();

  GPERF_LOG(display);

  return 0;
}
