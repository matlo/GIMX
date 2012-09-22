/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SIXEMUGUIAPP_H
#define SIXEMUGUIAPP_H

#include <wx/app.h>
#include "serialMain.h"

class serialApp : public wxApp
{
    public:
        virtual bool OnInit();
        virtual int OnExit();
};

#endif // SIXEMUGUIAPP_H
