/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef BLUETOOTHAPP_H
#define BLUETOOTHAPP_H

#include <wx/app.h>
#include "bluetoothMain.h"

class bluetoothApp : public wxApp
{
    public:
        virtual bool OnInit();
        virtual int OnExit();
};

#endif // BLUETOOTHAPP_H
