/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "launcherApp.h"

//(*AppHeaders
#include "gimx-launcher.h"
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

IMPLEMENT_APP(launcherApp);

bool launcherApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	launcherFrame* Frame = new launcherFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

int launcherApp::OnExit()
{
    return 0;
}
