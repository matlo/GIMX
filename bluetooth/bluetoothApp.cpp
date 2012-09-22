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

IMPLEMENT_APP(bluetoothApp);

bool bluetoothApp::OnInit()
{
    wxLocale* locale = new wxLocale(wxLANGUAGE_DEFAULT);
#ifdef WIN32
    locale->AddCatalogLookupPathPrefix(wxT("share/locale"));
#endif
    locale->AddCatalog(wxT("gimx"));

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
