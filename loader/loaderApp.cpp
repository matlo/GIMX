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
    // call default behaviour (mandatory)
    if (!wxApp::OnInit())
        return false;

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	loaderFrame* Frame = new loaderFrame(firmware, 0);
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

void loaderApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc (g_cmdLineDesc);
    // must refuse '/' as parameter starter or cannot use "/path" style paths
    parser.SetSwitchChars (wxT("-"));
}

bool loaderApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    parser.Found(wxT("f"), &firmware);

    return true;
}
