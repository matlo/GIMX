/***************************************************************
 * Name:      sixemuguiApp.h
 * Purpose:   Defines Application Class
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-01-12
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#ifndef SIXEMUGUIAPP_H
#define SIXEMUGUIAPP_H

#include <wx/app.h>
#include "sixemuguiMain.h"

class sixemuguiApp : public wxApp
{
    public:
        virtual bool OnInit();
        virtual int OnExit();
};

#endif // SIXEMUGUIAPP_H
