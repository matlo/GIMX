/***************************************************************
 * Name:      configApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2010-11-09
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "configApp.h"
#include <libxml/parser.h>

//(*AppHeaders
#include "configMain.h"
#include <wx/image.h>
//*)

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
  xmlCleanupParser();

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
