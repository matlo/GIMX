/***************************************************************
 * Name:      sixaxis_emu_guiApp.h
 * Purpose:   Defines Application Class
 * Author:    Matlo (mat.lau@laposte.net)
 * Created:   2010-11-09
 * Copyright: Matlo (http://diy-machine.blogspot.com/)
 * License:
 **************************************************************/

#ifndef SIXAXIS_EMU_GUIAPP_H
#define SIXAXIS_EMU_GUIAPP_H

#include <wx/app.h>
#include <wx/cmdline.h>

class sixaxis_emu_guiApp : public wxApp
{
    public:
        virtual bool OnInit();
        virtual int OnExit();
        virtual void OnInitCmdLine(wxCmdLineParser& parser);
        virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

    private:
        wxString file;
};

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
     { wxCMD_LINE_OPTION, wxT("f"), wxT("file"), wxT("specifies a file to open"),
         wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },

     { wxCMD_LINE_NONE }
};

#endif // SIXAXIS_EMU_GUIAPP_H
