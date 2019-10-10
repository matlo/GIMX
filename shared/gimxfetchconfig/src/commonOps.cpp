#include "../include/commonOps.h"


int process_cb(GE_Event* event __attribute__((unused)))
{
    return 0;
}


void autoConfigDownload(std::function<void(std::string)> func)
{
    std::list<std::string> joysticks;

    GPOLL_INTERFACE poll_interace =
    {
            .fp_register = REGISTER_FUNCTION,
            .fp_remove = REMOVE_FUNCTION,
    };
    if(ginput_init(&poll_interace, GE_MKB_SOURCE_NONE, process_cb) < 0)
    {
        ginput_quit();
        return;
    }

    for (int i = 0; ginput_joystick_name(i) != NULL; ++i)
    {
        joysticks.push_back(ginput_joystick_name(i));
    }

    ginput_quit();

    // TODO MLA: have an online index with device -> config, and be able to merge multiple configs

    struct
    {
        std::string name;
        std::string config;
    } configs [] =
#ifndef WIN32
    {
            { "Logitech Inc. WingMan Formula", "LogitechWingManFormula_G29.xml" },
            { "Logitech Inc. WingMan Formula GP", "LogitechWingManFormulaGP_G29.xml" },
            { "Logitech Inc. WingMan Formula Force", "LogitechWingManFormulaForce_G29.xml" },
            { "Logitech Inc. WingMan Formula Force GP", "LogitechWingManFormulaForceGP_G29.xml" },
            { "Logitech Logitech Driving Force", "LogitechDrivingForce_G29.xml" },
            { "Logitech Logitech Driving Force EX", "LogitechDrivingForceEx_G29.xml" },
            { "Logitech Logitech Driving Force Rx", "LogitechDrivingForceRx_G29.xml" },
            { "Logitech Logitech Formula Force EX", "LogitechFormulaForceEx_G29.xml" },
            { "PS3/USB Cordless Wheel", "LogitechDrivingForceWireless_DS4.xml" },
            // TODO MLA { "Logitech MOMO Force USB", "LogitechMomoForce_G29.xml" },
            { "Logitech Logitech Driving Force Pro", "LogitechDrivingForcePro_G29.xml" },
            { "G25 Racing Wheel", "LogitechG25_G29.xml" },
            { "Driving Force GT", "LogitechDrivingForceGT_G29.xml" },
            { "G27 Racing Wheel", "LogitechG27_G29.xml" },
            { "Logitech  Logitech MOMO Racing ", "LogitechMomoRacing_G29.xml" },
            { "Logitech G920 Driving Force Racing Wheel", "LogitechG920_G29.xml" },
    };
#else
    {
            { "Logitech WingMan Formula (Yellow) (USB)", "LogitechWingManFormula_G29.xml" },
            { "Logitech WingMan Formula GP", "LogitechWingManFormulaGP_G29.xml" },
            { "Logitech WingMan Formula Force USB", "LogitechWingManFormulaForce_G29.xml" },
            { "Logitech WingMan Formula Force GP USB", "LogitechWingManFormulaForceGP_G29.xml" },
            { "Logitech Driving Force USB", "LogitechDrivingForce_G29.xml" },
            { "Logitech MOMO Force USB", "LogitechMomoForce_G29.xml" },
            { "Logitech Driving Force Pro USB", "LogitechDrivingForcePro_G29.xml" },
            { "Logitech G25 Racing Wheel USB", "LogitechG25_G29.xml" },
            { "Logitech Driving Force GT USB", "LogitechDrivingForceGT_G29.xml" },
            { "Logitech G27 Racing Wheel USB", "LogitechG27_G29.xml" },
            { "Logitech MOMO Racing USB", "LogitechMomoRacing_G29.xml" },
            { "Logitech G920 Driving Force Racing Wheel USB", "LogitechG920_G29.xml" },
    };
#endif

    for (std::list<std::string>::iterator it = joysticks.begin(); it != joysticks.end(); ++it)
    {
        for (unsigned int i = 0; i < sizeof(configs) / sizeof(*configs); ++i)
        {
            if (*it == configs[i].name)
            {
                func(configs[i].config);
            }
        }
    }
}
