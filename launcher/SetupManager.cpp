/*
 Copyright (c) 2018 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <gimxhid/include/ghid.h>
#include <set>
#include <string>
#include <string>

#include "SetupManager.h"
#include "wx_pch.h"

SetupManager::SetupManager() : progressDialog(NULL) {

}

SetupManager::~SetupManager() {

}

int SetupManager::onUpdateProgress(Updater::UpdaterStatus status, double progress, double total) {
    wxString message;
    switch (status) {
    case Updater::UpdaterStatusConnectionPending:
        message = _("Connecting");
        break;
    case Updater::UpdaterStatusDownloadInProgress:
        message = _("Progress: ");
        message.Append(wxString(Updater::getProgress(progress, total).c_str(), wxConvUTF8));
        break;
    case Updater::UpdaterStatusInstallPending:
        message = _("Waiting for installation to complete");
        break;
    default:
        break;
    }

    if (status >= 0) {
        if (progressDialog->Update(progress, message) == false) {
            return 1;
        }
    } else {
        return 1;
    }

    return 0;
}

void SetupManager::initDownload(wxProgressDialog * dlg) {
    progressDialog = dlg;
}

void SetupManager::cleanDownload() {
    progressDialog = NULL;
}

#ifdef WIN32
static int progress_callback(void *clientp, Updater::UpdaterStatus status, double progress, double total) {
    return ((SetupManager *) clientp)->onUpdateProgress(status, progress, total);
}

#define USB_VENDOR_ID_LOGITECH                   0x046d

#define USB_PRODUCT_ID_LOGITECH_FORMULA_YELLOW   0xc202 // no force feedback
#define USB_PRODUCT_ID_LOGITECH_FORMULA_GP       0xc20e // no force feedback
#define USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE    0xc291 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP 0xc293 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE    0xc294 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL       0xc295 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_DFP_WHEEL        0xc298 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_G25_WHEEL        0xc299 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL       0xc29a // classic protocol
#define USB_PRODUCT_ID_LOGITECH_G27_WHEEL        0xc29b // classic protocol
#define USB_PRODUCT_ID_LOGITECH_WII_WHEEL        0xc29c // rumble only
#define USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2      0xca03 // classic protocol
#define USB_PRODUCT_ID_LOGITECH_VIBRATION_WHEEL  0xca04 // rumble only
#define USB_PRODUCT_ID_LOGITECH_G920_WHEEL       0xc262 // hid++ protocol only
#define USB_PRODUCT_ID_LOGITECH_G29_PC_WHEEL     0xc24f // classic protocol
#define USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL    0xc260 // classic protocol with 1 byte offset

const char * lgsName = "Logitech Gaming Software";

const char * lgsDownload32 = "https://gimx.fr/download/LGS32";
const char * lgsDownload64 = "https://gimx.fr/download/LGS64";

const char * lgsManager = "Logitech WingMan Event Monitor";

const char * lgs2Download32 = "https://gimx.fr/download/LGS32_2";
const char * lgs2Download64 = "https://gimx.fr/download/LGS64_2";

const char * lgs2Manager = "Logitech Gaming Framework";

#define MAKE_IDS1(USB_VENDOR_ID, USB_PRODUCT_ID, MANAGER_NAME, MANAGER_WINDOW, MANAGER_DOWNLOAD_32, MANAGER_DOWNLOAD_64) \
    { \
        .vendor_id = USB_VENDOR_ID, \
        .product_id = USB_PRODUCT_ID, \
        .manager = \
        { \
            .name = MANAGER_NAME, \
            .window = MANAGER_WINDOW, \
            .download32 = MANAGER_DOWNLOAD_32, \
            .download64 = MANAGER_DOWNLOAD_64 \
        } \
    }

typedef struct {
    unsigned short vendor_id;
    unsigned short product_id;
    struct {
        const char * name;
        const char * window;
        const char * download32;
        const char * download64;
    } manager;
} s_device_manager_ids;

static s_device_manager_ids ids[] =
{
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_FORMULA_YELLOW, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_FORMULA_GP, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_FORMULA_FORCE_GP, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_DRIVING_FORCE, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_DFP_WHEEL, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_G25_WHEEL, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_DFGT_WHEEL, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_G27_WHEEL, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_MOMO_WHEEL2, lgsName, lgsManager, lgsDownload32, lgsDownload64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_VIBRATION_WHEEL, lgsName, lgsManager, lgsDownload32, lgsDownload64),

    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_G29_PC_WHEEL, lgsName, lgs2Manager, lgs2Download32, lgs2Download64),
    MAKE_IDS1(USB_VENDOR_ID_LOGITECH, USB_PRODUCT_ID_LOGITECH_G29_PS4_WHEEL, lgsName, lgs2Manager, lgs2Download32, lgs2Download64),

    { },
};

void SetupManager::run() {

    std::set<std::pair<const char *, const char *> > managers;

    struct ghid_device_info *devs, *cur_dev;

    devs = ghid_enumerate(0x0000, 0x0000);
    for (cur_dev = devs; cur_dev != NULL; cur_dev = cur_dev->next) {
        for (unsigned int i = 0; i < sizeof(ids) / sizeof(*ids); ++i) {
            if (cur_dev->vendor_id == ids[i].vendor_id && cur_dev->product_id == ids[i].product_id) {
                if (FindWindowA(NULL, ids[i].manager.window) == NULL) {
                    SYSTEM_INFO info;
                    GetNativeSystemInfo(&info);
                    switch (info.wProcessorArchitecture) {
                    case PROCESSOR_ARCHITECTURE_AMD64:
                    case PROCESSOR_ARCHITECTURE_IA64:
                        managers.insert(std::make_pair(ids[i].manager.name, ids[i].manager.download64));
                        break;
                    case PROCESSOR_ARCHITECTURE_INTEL:
                        managers.insert(std::make_pair(ids[i].manager.name, ids[i].manager.download32));
                        break;
                    }
                }
            }
        }
    }
    ghid_free_enumeration(devs);

    std::set<std::pair<const char *, const char *> >::iterator it;
    for (it = managers.begin(); it != managers.end(); ++it) {
        int answer = wxMessageBox(_("A device requires the following component: \n")
                + wxString(it->first, wxConvUTF8) + _("\nDownload and install?"), _("Confirm"), wxYES_NO);
        if (answer == wxNO) {
            return;
        }

        wxProgressDialog dlg(_("Downloading ")  + wxString(it->first, wxConvUTF8), _("Connecting"), 101, NULL,
                wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_REMAINING_TIME | wxPD_SMOOTH);
        initDownload(&dlg);
        Updater::UpdaterStatus status = Updater().update(it->second, progress_callback, this, true);
        cleanDownload();
        if (status < Updater::UpdaterStatusOk && status != Updater::UpdaterStatusCancelled) {
            wxMessageBox(_("Can't retrieve Logitech Gaming Software!"), _("Error"), wxICON_ERROR);
        }
    }

}
#else
void SetupManager::run() {

}
#endif
