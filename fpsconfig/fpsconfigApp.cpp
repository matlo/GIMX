/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "fpsconfigApp.h"

//(*AppHeaders
#include "gimx-fpsconfig.h"
#include <wx/image.h>
//*)

#ifndef WIN32
void gtk_init_hack(void) __attribute__((constructor));
void gtk_init_hack(void)  // This will always run before main()
{
  if(setregid(getegid(), -1) == -1)
  {
    fprintf(stderr, "setregid failed\n");
  }
}
#endif

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

int fpsconfigApp::OnExit()
{
  return 0;
}

void fpsconfigApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc (g_cmdLineDesc);
    // must refuse '/' as parameter starter or cannot use "/path" style paths
    parser.SetSwitchChars (wxT("-"));
}

bool fpsconfigApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    parser.Found(wxT("f"), &file);

    return true;
}
