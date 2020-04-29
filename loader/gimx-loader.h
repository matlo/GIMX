/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef LAUNCHERMAIN_H
#define LAUNCHERMAIN_H

//(*Headers(loaderFrame)
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/choice.h>
//*)

#include <wx/process.h>

class loaderFrame: public wxFrame
{
    public:

        loaderFrame(wxString firmware,wxWindow* parent,wxWindowID id = -1);
        virtual ~loaderFrame();

        void OnProcessTerminated(wxProcess *process, int status);

    private:

        //(*Handlers(loaderFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnButtonLoadClick(wxCommandEvent& event);
        //*)

        //(*Identifiers(loaderFrame)
        static const long ID_CHOICE1;
        static const long ID_BUTTON1;
        static const long ID_PANEL1;
        static const long ID_MENUITEM1;
        //*)

        //(*Declarations(loaderFrame)
        wxMenuItem* MenuItem1;
        wxFlexGridSizer* FlexGridSizer1;
        wxPanel* Panel1;
        wxChoice* ChoiceFirmware;
        wxMenu* Menu1;
        wxButton* ButtonLoad;
        wxMenuBar* MenuBar1;
        //*)

        wxLocale* locale;

        wxString selected;

        DECLARE_EVENT_TABLE()
};

#endif // LAUNCHERMAIN_H
