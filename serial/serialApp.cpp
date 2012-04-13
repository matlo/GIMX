/***************************************************************
 * Name:      serialApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2011-01-12
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "serialApp.h"

//(*AppHeaders
#include "serialMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(serialApp);

bool serialApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	serialFrame* Frame = new serialFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

int serialApp::OnExit()
{
    return 0;
}
