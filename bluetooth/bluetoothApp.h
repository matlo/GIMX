/***************************************************************
 * Name:      bluetoothApp.h
 * Purpose:   Defines Application Class
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-01-12
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

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
