/***************************************************************
 * Name:      sixstatusApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-02-17
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "sixstatusApp.h"

//(*AppHeaders
#include "sixstatusMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(sixstatusApp);

bool sixstatusApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	sixstatusFrame* Frame = new sixstatusFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    //*)
    }
    return wxsOK;

}
