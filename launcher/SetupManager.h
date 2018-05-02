/*
 Copyright (c) 2018 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef SETUPMANAGER_H_
#define SETUPMANAGER_H_

#include <string>
#include <wx/progdlg.h>
#include <gimxupdater/Updater.h>

class SetupManager {
public:
    SetupManager();
    virtual ~SetupManager();
    void run();
    int onUpdateProgress(Updater::UpdaterStatus status, double progress, double total);
private:
    void initDownload(wxProgressDialog * dlg);
    void cleanDownload();

    wxProgressDialog * progressDialog;
};

#endif /* SETUPMANAGER_H_ */
