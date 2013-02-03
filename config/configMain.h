/*
 Copyright (c) 2010 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONFIGMAIN_H
#define CONFIGMAIN_H

//(*Headers(configFrame)
#include <wx/grid.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/panel.h>
#include <wx/statusbr.h>
#include <wx/statline.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//*)

#include <event_catcher.h>
#include <ConfigurationFile.h>

class configFrame: public wxFrame
{
    public:

        configFrame(wxString file,wxWindow* parent,wxWindowID id = -1);
        virtual ~configFrame();

        void reset_buttons();
        void save_current();
        void load_current();
        void refresh_gui();

    private:

        //(*Handlers(configFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnMenuItemNew(wxCommandEvent& event);
        void OnButtonTabModifyClick(wxCommandEvent& event);
        void OnButtonAddPanelButton(wxCommandEvent& event);
        void OnEventTypeSelectPanelAxis(wxCommandEvent& event);
        void OnAxisTabBufferSizeText(wxCommandEvent& event);
        void OnAxisTabAxisIdSelect(wxCommandEvent& event);
        void OnButtonAddPanelAxis(wxCommandEvent& event);
        void OnButtonRemovePanelButton(wxCommandEvent& event);
        void OnButtonRemovePanelAxis(wxCommandEvent& event);
        void OnButtonTabEventTypeSelect(wxCommandEvent& event);
        void OnChoice6Select(wxCommandEvent& event);
        void OnChoice6Select1(wxCommandEvent& event);
        void OnButtonAutoDetectClick(wxCommandEvent& event);
        void OnAxisTabShape5Select(wxCommandEvent& event);
        void OnAxisTabShape7Select(wxCommandEvent& event);
        void OnAxisTabShape8Select(wxCommandEvent& event);
        void OnCheckBox2Click(wxCommandEvent& event);
        void OnCheckBoxSwitchBackClick(wxCommandEvent& event);
        void OnButtonTabAutoDetectClick(wxCommandEvent& event);
        void OnAxisTabAutoDetectClick(wxCommandEvent& event);
        void OnButtonDeleteClick(wxCommandEvent& event);
        void OnMenuOpen(wxCommandEvent& event);
        void OnMenuItemExpert(wxCommandEvent& event);
        void OnMenuSave(wxCommandEvent& event);
        void OnMenuSaveAs(wxCommandEvent& event);
        void OnButtonModifyButton(wxCommandEvent& event);
        void OnButtonModifyAxis(wxCommandEvent& event);
        void OnAxisTabShapeSelect(wxCommandEvent& event);
        void OnAxisTabAxisIdSelect1(wxCommandEvent& event);
        void OnMenuItemCopyConfiguration(wxCommandEvent& event);
        void OnMenuItemPasteConfiguration(wxCommandEvent& event);
        void OnMenuItemCopyController(wxCommandEvent& event);
        void OnMenuItemPasteController(wxCommandEvent& event);
        void OnButtonDeleteTrigger(wxCommandEvent& event);
        void OnMenuReplaceMouse(wxCommandEvent& event);
        void OnMenuReplaceKeyboard(wxCommandEvent& event);
        void OnMenuItemReplaceMouseDPI(wxCommandEvent& event);
        void OnMenuReplaceMouseDPI(wxCommandEvent& event);
        void OnMenuSetMouseDPI(wxCommandEvent& event);
        void OnMenuMultipleMK(wxCommandEvent& event);
        void OnTextCtrl(wxCommandEvent& event);
        void OnMenuUpdate(wxCommandEvent& event);
        void OnMenuAutoBindControls(wxCommandEvent& event);
        void OnMenuItemController(wxCommandEvent& event);
        void OnMenuItemConfiguration(wxCommandEvent& event);
        void OnChoice1Select(wxCommandEvent& event);
        void OnIntensityAutoDetectClick(wxCommandEvent& event);
        void OnIntensityDeadZoneChange(wxSpinEvent& event);
        void OnIntensityModifyClick(wxCommandEvent& event);
        void OnIntensityAddClick(wxCommandEvent& event);
        void OnIntensityRemoveClick(wxCommandEvent& event);
        void OnIntensityAxisSelect(wxCommandEvent& event);
        void OnMouseOptionsAutoDetectClick(wxCommandEvent& event);
        void OnMouseOptionsTriggerAutoDetectClick(wxCommandEvent& event);
        void OnMouseOptionsAddClick(wxCommandEvent& event);
        void OnMouseOptionsRemoveClick(wxCommandEvent& event);
        void OnMouseOptionsModifyClick(wxCommandEvent& event);
        //*)
        void DeleteSelectedRows(wxGrid* grid);
        void DeleteLinkedRows(wxGrid* grid, int row);
        void replaceDevice(wxString wx_device_type);
        void auto_detect(wxStaticText* device_type, string* name, wxStaticText* device_name, wxStaticText* device_id, wxString event_type, wxStaticText* event_id);
        void updateButtonConfigurations();
        void updateAxisConfigurations();
        void updateIntensityConfigurations(Intensity* oldI, Intensity* newI);
        void updateMouseOptionsConfigurations(MouseOptions* oldM, MouseOptions* newM);
        wxString isAlreadyUsed(wxString device_type, wxString device_name, wxString device_id, wxString event_type, wxString event_id, int gridIndex1, int gridIndex2, int gridIndex3);
        bool isMouseOptionsDefined(wxString device_name, wxString device_id, int gridIndex);
        void readLabels();

        //(*Identifiers(configFrame)
        static const long ID_STATICTEXT35;
        static const long ID_STATICTEXT27;
        static const long ID_STATICTEXT36;
        static const long ID_STATICTEXT37;
        static const long ID_BUTTON1;
        static const long ID_STATICLINE5;
        static const long ID_CHECKBOX1;
        static const long ID_STATICTEXT28;
        static const long ID_SPINCTRL5;
        static const long ID_STATICLINE6;
        static const long ID_BUTTON10;
        static const long ID_PANEL4;
        static const long ID_STATICTEXT3;
        static const long ID_STATICTEXT4;
        static const long ID_STATICTEXT5;
        static const long ID_STATICTEXT14;
        static const long ID_STATICTEXT9;
        static const long ID_BUTTON11;
        static const long ID_STATICLINE1;
        static const long ID_STATICTEXT10;
        static const long ID_CHOICE3;
        static const long ID_STATICLINE2;
        static const long ID_STATICTEXT26;
        static const long ID_TEXTCTRL7;
        static const long ID_TEXTCTRL11;
        static const long ID_GRID4;
        static const long ID_BUTTON12;
        static const long ID_BUTTON13;
        static const long ID_BUTTON14;
        static const long ID_PANEL5;
        static const long ID_STATICTEXT55;
        static const long ID_CHOICE6;
        static const long ID_STATICLINE4;
        static const long ID_STATICTEXT45;
        static const long ID_STATICTEXT46;
        static const long ID_STATICTEXT47;
        static const long ID_STATICTEXT52;
        static const long ID_STATICTEXT73;
        static const long ID_BUTTON19;
        static const long ID_STATICLINE3;
        static const long ID_STATICTEXT74;
        static const long ID_STATICTEXT53;
        static const long ID_STATICTEXT1;
        static const long ID_STATICTEXT54;
        static const long ID_CHOICE9;
        static const long ID_SPINCTRL6;
        static const long ID_CHOICE2;
        static const long ID_SPINCTRL7;
        static const long ID_GRID3;
        static const long ID_BUTTON21;
        static const long ID_BUTTON22;
        static const long ID_BUTTON23;
        static const long ID_PANEL6;
        static const long ID_NOTEBOOK2;
        static const long ID_PANEL1;
        static const long ID_STATICTEXT38;
        static const long ID_STATICTEXT30;
        static const long ID_STATICTEXT39;
        static const long ID_CHOICE4;
        static const long ID_STATICTEXT40;
        static const long ID_STATICTEXT2;
        static const long ID_BUTTON8;
        static const long ID_STATICLINE8;
        static const long ID_STATICTEXT7;
        static const long ID_CHOICE5;
        static const long ID_STATICLINE9;
        static const long ID_STATICTEXT8;
        static const long ID_TEXTCTRL3;
        static const long ID_STATICLINE10;
        static const long ID_STATICTEXT34;
        static const long ID_COMBOBOX1;
        static const long ID_GRID1;
        static const long ID_BUTTON4;
        static const long ID_BUTTON6;
        static const long ID_BUTTON2;
        static const long ID_PANEL2;
        static const long ID_STATICTEXT41;
        static const long ID_STATICTEXT32;
        static const long ID_STATICTEXT42;
        static const long ID_CHOICE7;
        static const long ID_STATICTEXT43;
        static const long ID_STATICTEXT21;
        static const long ID_BUTTON9;
        static const long ID_STATICLINE11;
        static const long ID_STATICTEXT16;
        static const long ID_CHOICE8;
        static const long ID_STATICLINE7;
        static const long ID_STATICTEXT15;
        static const long ID_STATICTEXT17;
        static const long ID_STATICTEXT18;
        static const long ID_STATICTEXT22;
        static const long ID_STATICTEXT33;
        static const long ID_TEXTCTRL8;
        static const long ID_TEXTCTRL9;
        static const long ID_TEXTCTRL10;
        static const long ID_CHOICE1;
        static const long ID_TEXTCTRL1;
        static const long ID_TEXTCTRL2;
        static const long ID_STATICLINE12;
        static const long ID_STATICTEXT44;
        static const long ID_COMBOBOX2;
        static const long ID_GRID2;
        static const long ID_BUTTON3;
        static const long ID_BUTTON7;
        static const long ID_BUTTON5;
        static const long ID_PANEL3;
        static const long ID_NOTEBOOK1;
        static const long idMenuNew;
        static const long idMenuOpen;
        static const long idMenuSave;
        static const long idMenuSaveAs;
        static const long idMenuQuit;
        static const long ID_MENUITEM12;
        static const long ID_MENUITEM18;
        static const long ID_MENUITEM17;
        static const long ID_MENUITEM19;
        static const long ID_MENUITEM23;
        static const long ID_MENUITEM20;
        static const long ID_MENUITEM22;
        static const long ID_MENUITEM21;
        static const long ID_MENUITEM1;
        static const long ID_MENUITEM2;
        static const long ID_MENUITEM3;
        static const long ID_MENUITEM4;
        static const long ID_MENUITEM5;
        static const long ID_MENUITEM6;
        static const long ID_MENUITEM7;
        static const long ID_MENUITEM8;
        static const long ID_MENUITEM9;
        static const long ID_MENUITEM10;
        static const long ID_MENUITEM11;
        static const long ID_MENUITEM13;
        static const long ID_MENUITEM14;
        static const long ID_MENUITEM15;
        static const long ID_MENUITEM16;
        static const long ID_MENUITEM24;
        static const long ID_MENUITEM25;
        static const long ID_MENUITEM27;
        static const long ID_MENUITEM26;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(configFrame)
        wxMenuItem* MenuConfiguration3;
        wxStaticText* StaticText24;
        wxStaticText* StaticTextSmoothingPanelAxis;
        wxStaticText* StaticTextDZPanelAxis;
        wxStaticLine* StaticLine10;
        wxStaticText* ButtonTabDeviceType;
        wxStaticLine* StaticLine6;
        wxMenuItem* MenuItemSaveAs;
        wxStaticText* MouseOptionsId;
        wxButton* ButtonTabRemove;
        wxButton* IntensityAutoDetect;
        wxButton* MouseOptionsRemove;
        wxChoice* MouseOptionsInitMode;
        wxSpinCtrl* ProfileTriggerDelay;
        wxStaticText* IntensityDeviceType;
        wxStaticText* StaticTextLabel;
        wxPanel* PanelMouseOptions;
        wxButton* IntensityRemove;
        wxPanel* PanelAxis;
        wxButton* ButtonTabAdd;
        wxChoice* ButtonTabButtonId;
        wxStaticText* ButtonTabDeviceId;
        wxButton* IntensityModify;
        wxStaticLine* StaticLine2;
        wxGrid* GridPanelAxis;
        wxStatusBar* StatusBar1;
        wxMenuItem* MenuController6;
        wxStaticLine* StaticLine11;
        wxComboBox* ButtonTabLabel;
        wxStaticText* IntensityDeviceId;
        wxStaticText* IntensityDeviceName;
        wxMenuItem* MenuAutoBindControls;
        wxGrid* GridIntensity;
        wxTextCtrl* MouseOptionsFilter;
        wxStaticLine* StaticLine12;
        wxChoice* AxisTabEventType;
        wxStaticText* StaticText17;
        wxMenu* MenuController;
        wxStaticText* StaticTextEmptyPanelAxis;
        wxMenu* MenuFile;
        wxStaticText* AxisTabDeviceId;
        wxStaticLine* StaticLine7;
        wxStaticText* MouseOptionsButton;
        wxMenuItem* MenuItemPasteProfile;
        wxMenuItem* MenuController7;
        wxChoice* IntensityDirection;
        wxStaticText* ProfileTriggerDeviceName;
        wxGrid* GridMouseOption;
        wxMenuItem* MenuItemSave;
        wxPanel* PanelOverall;
        wxMenuItem* MenuConfiguration2;
        wxMenuItem* MenuItemCopyController;
        wxMenuItem* MenuConfiguration8;
        wxPanel* PanelIntensity;
        wxStaticText* ButtonTabDeviceName;
        wxStaticLine* StaticLine5;
        wxMenuItem* MenuController2;
        wxButton* Button3;
        wxPanel* PanelButton;
        wxMenuItem* MenuConfiguration5;
        wxMenuItem* MenuUpdate;
        wxStaticText* IntensityButtonId;
        wxStaticText* MouseOptionsType;
        wxButton* ButtonDelete;
        wxStaticText* StaticText18;
        wxStaticText* AxisTabEventId;
        wxStaticText* StaticText1;
        wxChoice* AxisTabAxisId;
        wxTextCtrl* AxisTabAcceleration;
        wxCheckBox* CheckBoxSwitchBack;
        wxButton* MouseOptionsAdd;
        wxMenuItem* MenuConfiguration7;
        wxStaticText* StaticText3;
        wxMenuItem* MenuItemPasteController;
        wxMenuItem* MenuItemLinkControls;
        wxStaticText* StaticTextSensPanelAxis;
        wxComboBox* AxisTabLabel;
        wxChoice* IntensityShape;
        wxStaticLine* StaticLine1;
        wxStaticText* StaticTextShapePanelAxis;
        wxSpinCtrl* IntensitySteps;
        wxButton* MouseOptionsAutoDetect;
        wxButton* IntensityAdd;
        wxButton* Button5;
        wxTextCtrl* AxisTabFilter;
        wxMenu* MenuAdvanced;
        wxMenuItem* MenuItemNew;
        wxStaticText* StaticText8;
        wxStaticText* StaticText12;
        wxMenuItem* MenuConfiguration4;
        wxMenuItem* MenuController4;
        wxStaticText* ButtonTabEventId;
        wxTextCtrl* AxisTabBufferSize;
        wxFileDialog* FileDialog1;
        wxStaticText* AxisTabDeviceType;
        wxStaticLine* StaticLine3;
        wxChoice* AxisTabShape;
        wxStaticText* StaticText7;
        wxMenuItem* MenuController3;
        wxMenuItem* MenuItemSetMouseDPI;
        wxButton* ButtonAutoDetect;
        wxNotebook* Notebook2;
        wxStaticText* ProfileTriggerDeviceId;
        wxMenuItem* MenuItemMultipleMiceAndKeyboards;
        wxButton* AxisTabAutoDetect;
        wxStaticText* StaticTextAccelPanelAxis;
        wxMenuItem* MenuItemReplaceMouse;
        wxMenuItem* MenuConfiguration6;
        wxTextCtrl* AxisTabDeadZone;
        wxMenu* MenuConfiguration;
        wxStaticText* StaticText2;
        wxNotebook* Notebook1;
        wxGrid* GridPanelButton;
        wxStaticText* MouseOptionsName;
        wxMenuItem* MenuItemReplaceKeyboard;
        wxStaticLine* StaticLine8;
        wxStaticText* StaticText6;
        wxSpinCtrl* IntensityDeadZone;
        wxMenuItem* MenuController5;
        wxButton* ButtonTabModify;
        wxStaticText* StaticTextButtonPanelButton;
        wxStaticLine* StaticLine4;
        wxStaticText* AxisTabDeviceName;
        wxMenu* MenuEdit;
        wxMenuItem* MenuItemReplaceMouseDPI;
        wxChoice* IntensityAxis;
        wxMenuItem* MenuItemCopyProfile;
        wxButton* Button7;
        wxTextCtrl* MouseOptionsBuffer;
        wxStaticText* ProfileTriggerDeviceType;
        wxPanel* PanelTrigger;
        wxMenuItem* MenuController1;
        wxStaticText* StaticTextThresholdPanelButton;
        wxTextCtrl* AxisTabSensitivity;
        wxChoice* ButtonTabEventType;
        wxButton* ButtonTabAutoDetect;
        wxTextCtrl* ButtonTabThreshold;
        wxButton* MouseOptionsModify;
        wxStaticText* StaticTextAxisPanelAxis;
        wxMenuItem* MenuConfiguration1;
        wxStaticLine* StaticLine9;
        wxStaticText* StaticText11;
        wxStaticText* ProfileTriggerButtonId;
        wxStaticText* StaticTextDelayPanelOverall;
        //*)

        event_catcher evcatch;
        ConfigurationFile configFile;
        unsigned int currentController;
        unsigned int currentConfiguration;

        Configuration tempConfiguration;
        Controller tempController;

        unsigned int grid1mod;
        unsigned int grid2mod;
        unsigned int grid3mod;
        unsigned int grid4mod;
        
        wxString default_directory;

        string buttonTabDeviceName;
        string axisTabDeviceName;
        string triggerTabDeviceName;
        string mouseTabDeviceName;
        string intensityTabDeviceName;

        DECLARE_EVENT_TABLE()
};

#endif // CONFIGMAIN_H
