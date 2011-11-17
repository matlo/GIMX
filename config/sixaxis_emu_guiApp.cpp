/***************************************************************
 * Name:      sixaxis_emu_guiApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2010-11-09
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "sixaxis_emu_guiApp.h"
#include <libxml/parser.h>

//(*AppHeaders
#include "sixaxis_emu_guiMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(sixaxis_emu_guiApp);

bool sixaxis_emu_guiApp::OnInit()
{
    // call default behaviour (mandatory)
    if (!wxApp::OnInit())
        return false;

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	sixaxis_emu_guiFrame* Frame = new sixaxis_emu_guiFrame(file, 0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

int sixaxis_emu_guiApp::OnExit()
{
  xmlCleanupParser();

  return 0;
}

void sixaxis_emu_guiApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc (g_cmdLineDesc);
    // must refuse '/' as parameter starter or cannot use "/path" style paths
    parser.SetSwitchChars (wxT("-"));
}

bool sixaxis_emu_guiApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    parser.Found(_("f"), &file);

    return true;
}
