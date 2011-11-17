/***************************************************************
 * Name:      fpsconfigApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-08-06
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "fpsconfigApp.h"

//(*AppHeaders
#include "fpsconfigMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(fpsconfigApp);

bool fpsconfigApp::OnInit()
{
    // call default behaviour (mandatory)
    if (!wxApp::OnInit())
        return false;

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
        fpsconfigFrame* Frame = new fpsconfigFrame(file, 0, -1);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

void fpsconfigApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc (g_cmdLineDesc);
    // must refuse '/' as parameter starter or cannot use "/path" style paths
    parser.SetSwitchChars (wxT("-"));
}

bool fpsconfigApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    parser.Found(_("f"), &file);

    return true;
}
