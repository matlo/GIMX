/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef FPSCONFIGAPP_H
#define FPSCONFIGAPP_H

#include <wx/app.h>
#include <wx/cmdline.h>

class fpsconfigApp : public wxApp
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
     { wxCMD_LINE_OPTION, wxT_2("f"), _("file"), _("specifies a file to open"),
         wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },

     { wxCMD_LINE_NONE, NULL, NULL, NULL, wxCMD_LINE_VAL_NONE, 0 }
};

#endif // FPSCONFIGAPP_H
