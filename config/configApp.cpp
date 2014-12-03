/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "configApp.h"

//(*AppHeaders
#include "gimx-config.h"
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

IMPLEMENT_APP(configApp);

bool configApp::OnInit()
{
    wxLocale* locale = new wxLocale(wxLANGUAGE_DEFAULT);
#ifdef WIN32
    locale->AddCatalogLookupPathPrefix(wxT("share/locale"));
#endif
    locale->AddCatalog(wxT("gimx"));
    
    // call default behaviour (mandatory)
    if (!wxApp::OnInit())
        return false;

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	configFrame* Frame = new configFrame(file, 0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

int configApp::OnExit()
{
  return 0;
}

void configApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc (g_cmdLineDesc);
    // must refuse '/' as parameter starter or cannot use "/path" style paths
    parser.SetSwitchChars (wxT("-"));
}

bool configApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    parser.Found(wxT("f"), &file);

    return true;
}
