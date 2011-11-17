/***************************************************************
 * Name:      sixemuguiApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-01-12
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "sixemuguiApp.h"

//(*AppHeaders
#include "sixemuguiMain.h"
#include <wx/image.h>
//*)

#include <glib.h>

IMPLEMENT_APP(sixemuguiApp);

bool sixemuguiApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	sixemuguiFrame* Frame = new sixemuguiFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

int sixemuguiApp::OnExit()
{
    g_spawn_command_line_sync ("killall emu", NULL, NULL, NULL, NULL);
    return 0;
}
