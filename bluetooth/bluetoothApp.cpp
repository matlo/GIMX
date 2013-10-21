/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "bluetoothApp.h"

//(*AppHeaders
#include "bluetoothMain.h"
#include <wx/image.h>
//*)

#include <glib.h>

#ifndef WIN32
void gtk_init_hack(void) __attribute__((constructor));
void gtk_init_hack(void) { setregid(getegid(), -1); }  // This will always run before main()
#endif

IMPLEMENT_APP(bluetoothApp);

bool bluetoothApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	bluetoothFrame* Frame = new bluetoothFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

int bluetoothApp::OnExit()
{
    return 0;
}
