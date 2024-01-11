// --------------------------------------------------------------------------
//
// This file is part of the XELab software package.
//
// Version 1.1 - January 2024
//
//
// The XELab package is free software; you can use it, redistribute it,
// and/or modify it under the terms of the GNU General Public License
// version 3 as published by the Free Software Foundation. The full text
// of the license can be found in the file LICENSE.txt at the top level of
// the package distribution.
//
// Authors:
//		Leonardo Ricci, Alessio Perinelli, Marco Prevedelli
//		leonardo.ricci@unitn.it
//		alessio.perinelli@unitn.it
//		marco.prevedelli@unibo.it
//		nse.physics.unitn.it
//		https://github.com/PhysicsBehindElectronics/XELab
//
// --------------------------------------------------------------------------

#ifndef INCLUDED_MAINAPP
	#include "wavex-console_main.hpp"
	#define INCLUDED_MAINAPP
#endif

wxBEGIN_EVENT_TABLE(GuiFrame, wxFrame)
	EVT_MENU(APP_ABOUT, GuiFrame::showAboutDialog)
	EVT_MENU(APP_QUIT, GuiFrame::onFrameQuit)
wxEND_EVENT_TABLE()

GuiFrame::GuiFrame (const wxString& title)
: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~(wxMAXIMIZE_BOX))
{
	wxFont font_bold(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,  wxFONTWEIGHT_BOLD);
	wave_parameters = new ParametersWorkspace;
	thread_is_running = false;
	thread_shall_be_cancelled = false;

	wxBitmap	temp_png = wxBITMAP_PNG_FROM_DATA(icon_64_wavex);
	wxIcon		temp_icon;
	temp_icon.CopyFromBitmap(temp_png);
	this->SetIcon(temp_icon);

	wxMenuBar *menuBar = new wxMenuBar;
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(APP_ABOUT, "A&bout\tCtrl-A");
	menuFile->AppendSeparator();
	menuFile->Append(APP_QUIT, "E&xit\tCtrl-Q");
	menuBar->Append(menuFile, "&File");
	SetMenuBar(menuBar);

	statictext_title_1 = new wxStaticText(this, wxID_ANY, wxT("Channel 1"), wxDefaultPosition, wxDefaultSize, 0);
	statictext_title_1->SetFont(font_bold);
	staticline_title_1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1), wxLI_HORIZONTAL);
	statictext_title_2 = new wxStaticText(this, wxID_ANY, wxT("Channel 2"), wxDefaultPosition, wxDefaultSize, 0);
	statictext_title_2->SetFont(font_bold);
	staticline_title_2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1), wxLI_HORIZONTAL);
	staticline_separate_channels = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);

	wxArrayString	m_list_waveforms;
	m_list_waveforms.Add(wxT("Sinusoidal"));
	m_list_waveforms.Add(wxT("Triangular"));
	m_list_waveforms.Add(wxT("Square"));
	radiobox_waveshape_1 = new wxRadioBox(this, EVENT_CHOSEN_WAVESHAPE_1, wxT("Waveform"), wxDefaultPosition, wxDefaultSize, m_list_waveforms, 0, wxRA_SPECIFY_ROWS);
	Connect(EVENT_CHOSEN_WAVESHAPE_1, wxEVT_RADIOBOX, wxCommandEventHandler(GuiFrame::changedParameters));
	radiobox_waveshape_2 = new wxRadioBox(this, EVENT_CHOSEN_WAVESHAPE_2, wxT("Waveform"), wxDefaultPosition, wxDefaultSize, m_list_waveforms, 0, wxRA_SPECIFY_ROWS);
	Connect(EVENT_CHOSEN_WAVESHAPE_2, wxEVT_RADIOBOX, wxCommandEventHandler(GuiFrame::changedParameters));

	spinner_f1 = new wxSpinCtrlDouble(this, EVENT_SPINNER_F_1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL, 0.1, 22000.0, 100.0);
	Connect(EVENT_SPINNER_F_1, wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(GuiFrame::changedParameters));
	statictext_title_f1 = new wxStaticText(this, wxID_ANY, wxT("Frequency (Hz)"), wxDefaultPosition, wxDefaultSize, 0);
	spinner_A1 = new wxSpinCtrlDouble(this, EVENT_SPINNER_A_1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL, 0.001, 2.4, 1.0);
	Connect(EVENT_SPINNER_A_1, wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(GuiFrame::changedParameters));
	statictext_title_A1 = new wxStaticText(this, wxID_ANY, wxT("Amplitude"), wxDefaultPosition, wxDefaultSize, 0);
	spinner_d1 = new wxSpinCtrlDouble(this, EVENT_SPINNER_D_1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL, -10.0, 10.0, 0.0);
	Connect(EVENT_SPINNER_D_1, wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(GuiFrame::changedParameters));
	statictext_title_d1 = new wxStaticText(this, wxID_ANY, wxT("Delay (s)"), wxDefaultPosition, wxDefaultSize, 0);
	checkbox_output_1 = new wxCheckBox(this, EVENT_TOGGLE_OUTPUT_1, wxT("Output on"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
	Connect(EVENT_TOGGLE_OUTPUT_1, wxEVT_CHECKBOX, wxCommandEventHandler(GuiFrame::changedParameters));
	staticline_calibr_ch1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1));
	statictext_title_calibration_ch1 = new wxStaticText(this, wxID_ANY, "Calibration:", wxDefaultPosition, wxDefaultSize);
	statictext_title_calibration_ch1->SetFont(font_bold);
	statictext_calibration_ch1 = new wxStaticText(this, wxID_ANY, "No calibration", wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
	button_calibrate_ch1 = new wxButton(this, EVENT_CALIBRATION_CH1, "Set", wxDefaultPosition, wxDefaultSize);
	Connect(EVENT_CALIBRATION_CH1, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::changedCalibrationCh1));

	spinner_f2 = new wxSpinCtrlDouble(this, EVENT_SPINNER_F_2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL, 0.1, 22000.0, 100.0);
	Connect(EVENT_SPINNER_F_2, wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(GuiFrame::changedParameters));
	statictext_title_f2 = new wxStaticText(this, wxID_ANY, wxT("Frequency (Hz)"), wxDefaultPosition, wxDefaultSize, 0);
	spinner_A2 = new wxSpinCtrlDouble(this, EVENT_SPINNER_A_2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL, 0.001, 2.4, 1.0);
	Connect(EVENT_SPINNER_A_2, wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(GuiFrame::changedParameters));
	statictext_title_A2 = new wxStaticText(this, wxID_ANY, wxT("Amplitude"), wxDefaultPosition, wxDefaultSize, 0);
	spinner_d2 = new wxSpinCtrlDouble(this, EVENT_SPINNER_D_2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL, -10.0, 10.0, 0.0);
	Connect(EVENT_SPINNER_D_2, wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(GuiFrame::changedParameters));
	statictext_title_d2 = new wxStaticText(this, wxID_ANY, wxT("Delay (s)"), wxDefaultPosition, wxDefaultSize, 0);
	checkbox_output_2 = new wxCheckBox(this, EVENT_TOGGLE_OUTPUT_2, wxT("Output on"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
	Connect(EVENT_TOGGLE_OUTPUT_2, wxEVT_CHECKBOX, wxCommandEventHandler(GuiFrame::changedParameters));
	staticline_calibr_ch2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1));
	statictext_title_calibration_ch2 = new wxStaticText(this, wxID_ANY, "Calibration:", wxDefaultPosition, wxDefaultSize);
	statictext_title_calibration_ch2->SetFont(font_bold);
	statictext_calibration_ch2 = new wxStaticText(this, wxID_ANY, "No calibration", wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
	button_calibrate_ch2 = new wxButton(this, EVENT_CALIBRATION_CH2, "Set", wxDefaultPosition, wxDefaultSize);
	Connect(EVENT_CALIBRATION_CH2, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::changedCalibrationCh2));

	spinner_f1->SetDigits(2);
	spinner_f1->SetIncrement(0.01);
	spinner_f2->SetDigits(2);
	spinner_f1->SetIncrement(0.01);
	spinner_d1->SetDigits(6);
	spinner_d1->SetIncrement(0.000001);
	spinner_d2->SetDigits(6);
	spinner_d2->SetIncrement(0.000001);

	statictext_title_misc = new wxStaticText(this, wxID_ANY, wxT("General controls"), wxDefaultPosition, wxDefaultSize, 0);
	statictext_title_misc->SetFont(font_bold);
	staticline_title_misc = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1));
	button_run = new wxButton(this, EVENT_BUTTON_RUN, wxT("Run"), wxDefaultPosition,  wxSize(100,50));
	Connect(EVENT_BUTTON_RUN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::runStopButton));

	wxBoxSizer *hbox_ch1_title = new wxBoxSizer(wxHORIZONTAL);
	hbox_ch1_title->Add(statictext_title_1, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
	hbox_ch1_title->Add(staticline_title_1, 2, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
			wxBoxSizer *hbox_f1 = new wxBoxSizer(wxHORIZONTAL);
			wxBoxSizer *hbox_A1 = new wxBoxSizer(wxHORIZONTAL);
			wxBoxSizer *hbox_d1 = new wxBoxSizer(wxHORIZONTAL);
			hbox_f1->Add(statictext_title_f1, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			hbox_f1->Add(spinner_f1, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			hbox_A1->Add(statictext_title_A1, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			hbox_A1->Add(spinner_A1, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			hbox_d1->Add(statictext_title_d1, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			hbox_d1->Add(spinner_d1, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		wxBoxSizer *vbox_ch1_controls_spinners = new wxBoxSizer(wxVERTICAL);
		vbox_ch1_controls_spinners->Add(hbox_f1, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
		vbox_ch1_controls_spinners->Add(hbox_A1, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
		vbox_ch1_controls_spinners->Add(hbox_d1, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
	wxBoxSizer *hbox_ch1_controls_all = new wxBoxSizer(wxHORIZONTAL);
	hbox_ch1_controls_all->Add(radiobox_waveshape_1, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	hbox_ch1_controls_all->Add(vbox_ch1_controls_spinners, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	wxBoxSizer *hbox_ch1_calibr = new wxBoxSizer(wxHORIZONTAL);
	hbox_ch1_calibr->Add(statictext_calibration_ch1, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	hbox_ch1_calibr->Add(button_calibrate_ch1, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);

	wxBoxSizer *hbox_ch2_title = new wxBoxSizer(wxHORIZONTAL);
	hbox_ch2_title->Add(statictext_title_2, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
	hbox_ch2_title->Add(staticline_title_2, 2, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
			wxBoxSizer *hbox_f2 = new wxBoxSizer(wxHORIZONTAL);
			wxBoxSizer *hbox_A2 = new wxBoxSizer(wxHORIZONTAL);
			wxBoxSizer *hbox_d2 = new wxBoxSizer(wxHORIZONTAL);
			hbox_f2->Add(statictext_title_f2, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			hbox_f2->Add(spinner_f2, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			hbox_A2->Add(statictext_title_A2, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			hbox_A2->Add(spinner_A2, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			hbox_d2->Add(statictext_title_d2, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			hbox_d2->Add(spinner_d2, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		wxBoxSizer *vbox_ch2_controls_spinners = new wxBoxSizer(wxVERTICAL);
		vbox_ch2_controls_spinners->Add(hbox_f2, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
		vbox_ch2_controls_spinners->Add(hbox_A2, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
		vbox_ch2_controls_spinners->Add(hbox_d2, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
	wxBoxSizer *hbox_ch2_controls_all = new wxBoxSizer(wxHORIZONTAL);
	hbox_ch2_controls_all->Add(radiobox_waveshape_2, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	hbox_ch2_controls_all->Add(vbox_ch2_controls_spinners, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	wxBoxSizer *hbox_ch2_calibr = new wxBoxSizer(wxHORIZONTAL);
	hbox_ch2_calibr->Add(statictext_calibration_ch2, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	hbox_ch2_calibr->Add(button_calibrate_ch2, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);

	wxBoxSizer *hbox_two_channels = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *vbox_ch1_all = new wxBoxSizer(wxVERTICAL);
		vbox_ch1_all->Add(hbox_ch1_title, 0, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		vbox_ch1_all->Add(hbox_ch1_controls_all, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		vbox_ch1_all->Add(checkbox_output_1, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		vbox_ch1_all->Add(staticline_calibr_ch1, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
		vbox_ch1_all->Add(statictext_title_calibration_ch1, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
		vbox_ch1_all->Add(hbox_ch1_calibr, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
		wxBoxSizer *vbox_ch2_all = new wxBoxSizer(wxVERTICAL);
		vbox_ch2_all->Add(hbox_ch2_title, 0, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		vbox_ch2_all->Add(hbox_ch2_controls_all, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		vbox_ch2_all->Add(checkbox_output_2, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		vbox_ch2_all->Add(staticline_calibr_ch2, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
		vbox_ch2_all->Add(statictext_title_calibration_ch2, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
		vbox_ch2_all->Add(hbox_ch2_calibr, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
	hbox_two_channels->Add(vbox_ch1_all, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	hbox_two_channels->Add(staticline_separate_channels, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	hbox_two_channels->Add(vbox_ch2_all, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);

	wxBoxSizer *vbox_misc_all = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *hbox_misc_title = new wxBoxSizer(wxHORIZONTAL);
		hbox_misc_title->Add(statictext_title_misc, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
		hbox_misc_title->Add(staticline_title_misc, 2, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
		wxBoxSizer *hbox_misc_buttons = new wxBoxSizer(wxHORIZONTAL);
		hbox_misc_buttons->Add(button_run, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
	vbox_misc_all->Add(hbox_misc_title, 0, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	vbox_misc_all->Add(hbox_misc_buttons, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);

	wxBoxSizer *vbox_all = new wxBoxSizer(wxVERTICAL);
	vbox_all->Add(hbox_two_channels, 1, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	vbox_all->Add(vbox_misc_all, 1, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);

	this->SetSizer(vbox_all);
	initializeConstants();
	SetSize(1000,160,900,420);
	SetMinSize(wxSize(900,420));
	Show();
}

void GuiFrame::onFrameQuit (wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void GuiFrame::changedParameters (wxCommandEvent& WXUNUSED(event))
{
	this->wave_parameters->output_1 = (this->checkbox_output_1->GetValue() == 1)? true : false;
	this->wave_parameters->output_2 = (this->checkbox_output_2->GetValue() == 1)? true : false;

	this->wave_parameters->waveshape_1 = this->radiobox_waveshape_1->GetSelection();
	this->wave_parameters->waveshape_2 = this->radiobox_waveshape_2->GetSelection();

	this->wave_parameters->f1 = this->spinner_f1->GetValue();
	this->wave_parameters->f2 = this->spinner_f2->GetValue();
	this->wave_parameters->A1 = this->spinner_A1->GetValue();
	this->wave_parameters->A2 = this->spinner_A2->GetValue();
	this->wave_parameters->delay1 = this->spinner_d1->GetValue();
	this->wave_parameters->delay2 = this->spinner_d2->GetValue();

	this->wave_parameters->changed = true;

	return;
}

void GuiFrame::runStopButton(wxCommandEvent& WXUNUSED(event))
{
	if (thread_is_running) {
		thread_shall_be_cancelled = true;
		this->button_run->SetLabel("Run");
	} else {
		onWorkerStart();
		this->button_run->SetLabel("Stop");
	}
	return;
}

void GuiFrame::changedCalibrationCh1(wxCommandEvent& WXUNUSED(event))
{
	int user_value = (int) wxGetNumberFromUser("Insert calibration factor, i.e. an integer number corresponding to 1 Volt.", "[1,65535]", "Set calibration for Channel 1", 1, 1, 65535, this, wxDefaultPosition);

	if (user_value < 0)
		return;

	char	*display_label = (char*) malloc(32 * sizeof(char));
	if (user_value == 1) {
		this->wave_parameters->y1_vps = 1.0;
		this->statictext_calibration_ch1->SetLabel("No calibration");
		this->statictext_calibration_ch1->Refresh();
	} else {
		this->wave_parameters->y1_vps = 1.0 / (double) user_value;
		sprintf(display_label, "%d units/V", user_value);
		this->statictext_calibration_ch1->SetLabel(display_label);
		this->statictext_calibration_ch1->Refresh();
	}
	this->updateA1range();

	free(display_label);
	return;
}

void GuiFrame::changedCalibrationCh2(wxCommandEvent& WXUNUSED(event))
{
	int user_value = (int) wxGetNumberFromUser("Insert calibration factor, i.e. an integer number corresponding to 1 Volt.", "[1,65535]", "Set calibration for Channel 2", 1, 1, 65535, this, wxDefaultPosition);

	if (user_value < 0)
		return;

	char	*display_label = (char*) malloc(32 * sizeof(char));
	if (user_value == 1) {
		this->wave_parameters->y2_vps = 1.0;
		this->statictext_calibration_ch2->SetLabel("No calibration");
		this->statictext_calibration_ch2->Refresh();
	} else {
		this->wave_parameters->y2_vps = 1.0 / (double) user_value;
		sprintf(display_label, "%d units/V", user_value);
		this->statictext_calibration_ch2->SetLabel(display_label);
		this->statictext_calibration_ch2->Refresh();
	}
	this->updateA2range();

	free(display_label);
	return;
}

void GuiFrame::updateA1range()
{
	if (this->wave_parameters->y1_vps == 1.0) {
		this->spinner_A1->SetRange(0, 32767);
		this->spinner_A1->SetDigits(0);
		this->spinner_A1->SetIncrement(1);
	} else {
		this->spinner_A1->SetRange(0, 3.0);
		this->spinner_A1->SetDigits(2);
		this->spinner_A1->SetIncrement(0.01);
	}

	return;
}

void GuiFrame::updateA2range()
{
	if (this->wave_parameters->y2_vps == 1.0) {
		this->spinner_A2->SetRange(0, 32767);
		this->spinner_A2->SetDigits(0);
		this->spinner_A2->SetIncrement(1);
	} else {
		this->spinner_A2->SetRange(0, 3.0);
		this->spinner_A2->SetDigits(2);
		this->spinner_A2->SetIncrement(0.01);
	}

	return;
}

void GuiFrame::initializeConstants()
{
	this->wave_parameters->output_1 = false;
	this->wave_parameters->output_2 = false;
	this->wave_parameters->f1 = 100.0;
	this->wave_parameters->f2 = 100.0;
	this->wave_parameters->A1 = 1;
	this->wave_parameters->A2 = 1;
	this->wave_parameters->delay1 = 0.0;
	this->wave_parameters->delay2 = 0.0;
	this->wave_parameters->waveshape_1 = 0;
	this->wave_parameters->waveshape_2 = 0;
	this->wave_parameters->y1_vps = 1.0;
	this->wave_parameters->y2_vps = 1.0;

	this->radiobox_waveshape_1->SetSelection(this->wave_parameters->waveshape_1);
	this->radiobox_waveshape_2->SetSelection(this->wave_parameters->waveshape_2);

	this->wave_parameters->changed = false;

	this->thread_is_running = false;
	this->thread_shall_be_cancelled = false;

	this->updateA1range();
	this->updateA2range();

	return;
}

void GuiFrame::showAboutDialog(wxCommandEvent& WXUNUSED(event))
{
	wxAboutDialogInfo info;
	info.SetName(wxT("wavex - waveform generator"));
	info.SetVersion(wxT("1.0"));
	info.SetDescription(wxT("Graphical user interface to control the sound card output."));
	info.SetCopyright(wxT("2020"));

	wxIcon		temp_icon;
	wxBitmap	temp_png = wxBITMAP_PNG_FROM_DATA(icon_64_wavex);
	temp_icon.CopyFromBitmap(temp_png);
	info.SetIcon(temp_icon);

	wxAboutBox(info, this);

	return;
}
