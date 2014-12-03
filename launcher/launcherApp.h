/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef LAUNCHERAPP_H
#define LAUNCHERAPP_H

#include <wx/app.h>

class launcherApp : public wxApp
{
    public:
        virtual bool OnInit();
        virtual int OnExit();
};

#endif // SIXEMUGUIAPP_H
