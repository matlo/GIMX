/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "loaderApp.h"

//(*AppHeaders
#include "gimx-loader.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(loaderApp);

bool loaderApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	loaderFrame* Frame = new loaderFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

int loaderApp::OnExit()
{
    return 0;
}
