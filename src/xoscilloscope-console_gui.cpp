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
	#include "xoscilloscope-console_main.hpp"
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
	scope_parameters = new ContainerWorkspace;

	wxBitmap	temp_png = wxBITMAP_PNG_FROM_DATA(icon_64_xoscope);
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

	statictext_title_tdiv = new wxStaticText(this, wxID_ANY, wxT("Horizontal scale"), wxDefaultPosition, wxDefaultSize, 0);
	statictext_title_tdiv->SetFont(font_bold);
	staticline_title_tdiv = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1));
	statictext_value_tdiv = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
	button_tdiv_up = new wxButton(this, EVENT_BUTTON_TDUP, wxT("<"), wxDefaultPosition, wxSize(50,50));
	button_tdiv_dw = new wxButton(this, EVENT_BUTTON_TDDW, wxT(">"), wxDefaultPosition, wxSize(50,50));
	Connect(EVENT_BUTTON_TDUP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::knobTdivUp));
	Connect(EVENT_BUTTON_TDDW, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::knobTdivDw));

	statictext_title_y1dv = new wxStaticText(this, wxID_ANY, wxT("Vertical scale, channel 1"), wxDefaultPosition, wxDefaultSize, 0);
	statictext_title_y1dv->SetFont(font_bold);
	staticline_title_y1dv = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1));
	statictext_value_y1dv = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
	statictext_value_y1dv->SetFont(font_bold);
	button_y1dv_up = new wxButton(this, EVENT_BUTTON_Y1UP, wxT("+"), wxDefaultPosition, wxSize(50,50));
	button_y1dv_dw = new wxButton(this, EVENT_BUTTON_Y1DW, wxT("-"), wxDefaultPosition, wxSize(50,50));
	Connect(EVENT_BUTTON_Y1UP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::knobY1dvUp));
	Connect(EVENT_BUTTON_Y1DW, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::knobY1dvDw));
	staticline_calibr_ch1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1));
	statictext_title_calibration_ch1 = new wxStaticText(this, wxID_ANY, "Calibration:", wxDefaultPosition, wxDefaultSize);
	statictext_title_calibration_ch1->SetFont(font_bold);
	statictext_calibration_ch1 = new wxStaticText(this, wxID_ANY, "No calibration", wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
	button_calibrate_ch1 = new wxButton(this, EVENT_CALIBRATION_CH1, "Set", wxDefaultPosition, wxDefaultSize);
	Connect(EVENT_CALIBRATION_CH1, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::changedCalibrationCh1));

	statictext_title_y2dv = new wxStaticText(this, wxID_ANY, wxT("Vertical scale, channel 2"), wxDefaultPosition, wxDefaultSize, 0);
	statictext_title_y2dv->SetFont(font_bold);
	staticline_title_y2dv = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1));
	statictext_value_y2dv = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
	statictext_value_y2dv->SetFont(font_bold);
	button_y2dv_up = new wxButton(this, EVENT_BUTTON_Y2UP, wxT("+"), wxDefaultPosition, wxSize(50,50));
	button_y2dv_dw = new wxButton(this, EVENT_BUTTON_Y2DW, wxT("-"), wxDefaultPosition, wxSize(50,50));
	Connect(EVENT_BUTTON_Y2UP, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::knobY2dvUp));
	Connect(EVENT_BUTTON_Y2DW, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::knobY2dvDw));
	staticline_calibr_ch2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1));
	statictext_title_calibration_ch2 = new wxStaticText(this, wxID_ANY, "Calibration:", wxDefaultPosition, wxDefaultSize);
	statictext_title_calibration_ch2->SetFont(font_bold);
	statictext_calibration_ch2 = new wxStaticText(this, wxID_ANY, "No calibration", wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
	button_calibrate_ch2 = new wxButton(this, EVENT_CALIBRATION_CH2, "Set", wxDefaultPosition, wxDefaultSize);
	Connect(EVENT_CALIBRATION_CH2, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::changedCalibrationCh2));

	statictext_title_trig = new wxStaticText(this, wxID_ANY, wxT("Trigger settings"), wxDefaultPosition, wxDefaultSize, 0);
	statictext_title_trig->SetFont(font_bold);
	staticline_title_trig = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1));
	wxArrayString	m_list_channels;
	m_list_channels.Add(wxT("Ch. 1"));
	m_list_channels.Add(wxT("Ch. 2"));
	radiobox_trig_chan = new wxRadioBox(this, EVENT_CHOSEN_TRIG_CHAN, wxT("Trigger channel"), wxDefaultPosition, wxDefaultSize, m_list_channels, 0, wxRA_SPECIFY_ROWS);
	Connect(EVENT_CHOSEN_TRIG_CHAN, wxEVT_RADIOBOX, wxCommandEventHandler(GuiFrame::selectTrigChan));
	wxArrayString	m_list_trigedges;
	m_list_trigedges.Add(wxT("Rising"));
	m_list_trigedges.Add(wxT("Falling"));
	radiobox_trig_edge = new wxRadioBox(this, EVENT_CHOSEN_TRIG_EDGE, wxT("Trigger edge"), wxDefaultPosition, wxDefaultSize, m_list_trigedges, 0, wxRA_SPECIFY_ROWS);
	Connect(EVENT_CHOSEN_TRIG_EDGE, wxEVT_RADIOBOX, wxCommandEventHandler(GuiFrame::selectTrigEdge));
	statictext_label_trig = new wxStaticText(this, wxID_ANY, wxT("Trigger level:"), wxDefaultPosition, wxDefaultSize, 0);
	spinner_trig_level = new wxSpinCtrlDouble(this, EVENT_SPINNER_TRIG_LVL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL, -1.0, 1.0, 0.0);
	Connect(EVENT_SPINNER_TRIG_LVL, wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(GuiFrame::selectTrigLevel));

	statictext_title_misc = new wxStaticText(this, wxID_ANY, wxT("General controls"), wxDefaultPosition, wxDefaultSize, 0);
	statictext_title_misc->SetFont(font_bold);
	staticline_title_misc = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(-1,1));
	button_runpause = new wxButton(this, EVENT_BUTTON_RUNPAUSE, wxT("Pause"), wxDefaultPosition, wxDefaultSize);
	Connect(EVENT_BUTTON_RUNPAUSE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::togglePauseRun));
	button_togglemode = new wxButton(this, EVENT_BUTTON_TOGGLEMODE, wxT("MODE: Analog [Click to switch]"), wxDefaultPosition, wxDefaultSize, wxBU_LEFT);
	Connect(EVENT_BUTTON_TOGGLEMODE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::toggleMode));
	button_save = new wxButton(this, EVENT_BUTTON_SAVE, wxT("Save data"), wxDefaultPosition, wxDefaultSize);
	Connect(EVENT_BUTTON_SAVE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GuiFrame::selectFileToSave));
	wxArrayString	m_list_averages;
	m_list_averages.Add(wxT(CHOICES_AVERAGES_0));
	m_list_averages.Add(wxT(CHOICES_AVERAGES_1));
	m_list_averages.Add(wxT(CHOICES_AVERAGES_2));
	m_list_averages.Add(wxT(CHOICES_AVERAGES_3));
	m_list_averages.Add(wxT(CHOICES_AVERAGES_4));
	m_list_averages.Add(wxT(CHOICES_AVERAGES_5));
	choice_averages = new wxChoice(this, EVENT_CHOICE_AVERAGES, wxDefaultPosition, wxDefaultSize, m_list_averages);
	Connect(EVENT_CHOICE_AVERAGES, wxEVT_CHOICE, wxCommandEventHandler(GuiFrame::selectChoiceAverages));

	wxBoxSizer *vbox_all = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *hbox_tdtr_all = new wxBoxSizer(wxHORIZONTAL);
			wxBoxSizer *vbox_tdiv_all = new wxBoxSizer(wxVERTICAL);
				wxBoxSizer *hbox_tdiv_title = new wxBoxSizer(wxHORIZONTAL);
				hbox_tdiv_title->Add(statictext_title_tdiv, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 10);
				hbox_tdiv_title->Add(staticline_title_tdiv, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 10);
				wxBoxSizer *hbox_tdiv_btns = new wxBoxSizer(wxHORIZONTAL);
				hbox_tdiv_btns->Add(button_tdiv_up, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
				hbox_tdiv_btns->Add(button_tdiv_dw, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
				hbox_tdiv_btns->Add(statictext_value_tdiv, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
			vbox_tdiv_all->Add(hbox_tdiv_title, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			vbox_tdiv_all->Add(hbox_tdiv_btns, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			wxBoxSizer *vbox_trig_all = new wxBoxSizer(wxVERTICAL);
				wxBoxSizer *hbox_trig_title = new wxBoxSizer(wxHORIZONTAL);
				hbox_trig_title->Add(statictext_title_trig, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 10);
				hbox_trig_title->Add(staticline_title_trig, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 10);
				wxBoxSizer *hbox_trig_radios = new wxBoxSizer(wxHORIZONTAL);
				hbox_trig_radios->Add(radiobox_trig_chan, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
				hbox_trig_radios->Add(radiobox_trig_edge, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
				wxBoxSizer *hbox_trig_level = new wxBoxSizer(wxHORIZONTAL);
				hbox_trig_level->Add(statictext_label_trig, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
				hbox_trig_level->Add(spinner_trig_level, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
			vbox_trig_all->Add(hbox_trig_title, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			vbox_trig_all->Add(hbox_trig_radios, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			vbox_trig_all->Add(hbox_trig_level, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		hbox_tdtr_all->Add(vbox_tdiv_all, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		hbox_tdtr_all->Add(vbox_trig_all, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);

		wxBoxSizer *hbox_y1y2_all = new wxBoxSizer(wxHORIZONTAL);
			wxBoxSizer *vbox_y1div_all = new wxBoxSizer(wxVERTICAL);
				wxBoxSizer *hbox_y1div_title = new wxBoxSizer(wxHORIZONTAL);
				hbox_y1div_title->Add(statictext_title_y1dv, 3, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
				hbox_y1div_title->Add(staticline_title_y1dv, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
				wxBoxSizer *hbox_y1div_btns = new wxBoxSizer(wxHORIZONTAL);
					wxBoxSizer *vbox_y1div_left = new wxBoxSizer(wxVERTICAL);
						wxBoxSizer *hbox_y1calibr = new wxBoxSizer(wxHORIZONTAL);
						hbox_y1calibr->Add(statictext_calibration_ch1, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
						hbox_y1calibr->Add(button_calibrate_ch1, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					vbox_y1div_left->Add(statictext_value_y1dv, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					vbox_y1div_left->Add(staticline_calibr_ch1, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					vbox_y1div_left->Add(statictext_title_calibration_ch1, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					vbox_y1div_left->Add(hbox_y1calibr, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					wxBoxSizer *vbox_y1div_btns = new wxBoxSizer(wxVERTICAL);
					vbox_y1div_btns->Add(button_y1dv_up, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					vbox_y1div_btns->Add(button_y1dv_dw, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
				hbox_y1div_btns->Add(vbox_y1div_btns, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
				hbox_y1div_btns->Add(vbox_y1div_left, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
			vbox_y1div_all->Add(hbox_y1div_title, 0, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			vbox_y1div_all->Add(hbox_y1div_btns, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			wxBoxSizer *vbox_y2div_all = new wxBoxSizer(wxVERTICAL);
				wxBoxSizer *hbox_y2div_title = new wxBoxSizer(wxHORIZONTAL);
				hbox_y2div_title->Add(statictext_title_y2dv, 3, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
				hbox_y2div_title->Add(staticline_title_y2dv, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
				wxBoxSizer *hbox_y2div_btns = new wxBoxSizer(wxHORIZONTAL);
					wxBoxSizer *vbox_y2div_left = new wxBoxSizer(wxVERTICAL);
						wxBoxSizer *hbox_y2calibr = new wxBoxSizer(wxHORIZONTAL);
						hbox_y2calibr->Add(statictext_calibration_ch2, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
						hbox_y2calibr->Add(button_calibrate_ch2, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					vbox_y2div_left->Add(statictext_value_y2dv, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					vbox_y2div_left->Add(staticline_calibr_ch2, 1, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					vbox_y2div_left->Add(statictext_title_calibration_ch2, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					vbox_y2div_left->Add(hbox_y2calibr, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					wxBoxSizer *vbox_y2div_btns = new wxBoxSizer(wxVERTICAL);
					vbox_y2div_btns->Add(button_y2dv_up, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
					vbox_y2div_btns->Add(button_y2dv_dw, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
				hbox_y2div_btns->Add(vbox_y2div_btns, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
				hbox_y2div_btns->Add(vbox_y2div_left, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);
			vbox_y2div_all->Add(hbox_y2div_title, 0, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
			vbox_y2div_all->Add(hbox_y2div_btns, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		hbox_y1y2_all->Add(vbox_y1div_all, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		hbox_y1y2_all->Add(vbox_y2div_all, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);

		wxBoxSizer *vbox_misc_all = new wxBoxSizer(wxVERTICAL);
			wxBoxSizer *hbox_misc_title = new wxBoxSizer(wxHORIZONTAL);
			hbox_misc_title->Add(statictext_title_misc, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 10);
			hbox_misc_title->Add(staticline_title_misc, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 10);
			wxBoxSizer *hbox_misc_all = new wxBoxSizer(wxHORIZONTAL);
			hbox_misc_all->Add(button_runpause, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
			hbox_misc_all->Add(button_save, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
			hbox_misc_all->Add(button_togglemode, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
			hbox_misc_all->Add(choice_averages, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 8);
		vbox_misc_all->Add(hbox_misc_title, 0, wxALL | wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
		vbox_misc_all->Add(hbox_misc_all, 0, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	vbox_all->Add(hbox_tdtr_all, 1, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	vbox_all->Add(hbox_y1y2_all, 1, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);
	vbox_all->Add(vbox_misc_all, 1, wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 1);

	this->SetSizer(vbox_all);
	SetSize(1080,550,650,500);
	SetMinSize(wxSize(650,500));
	Show();
	GuiFrame::initializeConstants();

	wxCommandEvent eventStartup(wxEVT_THREAD, EVENT_WORKER_STARTUP);
	GuiFrame::onWorkerStart(eventStartup);
}

void GuiFrame::onFrameQuit (wxCommandEvent& WXUNUSED(event) )
{
	Close(true);
}

void GuiFrame::knobTdivUp (wxCommandEvent& WXUNUSED(event))
{
	int ti = this->scope_parameters->tdiv_idx;
	int ti_max = this->scope_parameters->list_tdiv.size();
	if (ti == ti_max - 1) {
		return;
	} else if (ti == 0) {
		this->button_tdiv_dw->Enable();
		ti++;
	} else if (ti == ti_max - 2){
		this->button_tdiv_up->Disable();
		ti++;
	} else {
		ti++;
	}
	this->scope_parameters->tdiv_idx = ti;
	this->scope_parameters->send_changes = true;
	this->updateTdiv();

	return;
}

void GuiFrame::knobTdivDw (wxCommandEvent& WXUNUSED(event))
{
	int ti = this->scope_parameters->tdiv_idx;
	int ti_max = this->scope_parameters->list_tdiv.size();
	if (ti == 0) {
		return;
	} else if (ti == ti_max - 1) {
		this->button_tdiv_up->Enable();
		ti--;
	} else if (ti == 1){
		this->button_tdiv_dw->Disable();
		ti--;
	} else {
		ti--;
	}
	this->scope_parameters->tdiv_idx = ti;
	this->scope_parameters->send_changes = true;
	this->updateTdiv();

	return;
}

void GuiFrame::updateTdiv()
{
	std::stringstream	displayed_value;
	displayed_value << "Time scale: \n" << this->scope_parameters->list_tdiv_txt[this->scope_parameters->tdiv_idx] << " / div";
	this->statictext_value_tdiv->SetLabel(displayed_value.str());
	return;
}

void GuiFrame::knobY1dvUp (wxCommandEvent& WXUNUSED(event))
{
	int yi = this->scope_parameters->y1div_idx;
	int yi_max = this->scope_parameters->ydiv_size;
	if (yi == yi_max - 1) {
		return;
	} else if (yi == 0) {
		this->button_y1dv_dw->Enable();
		yi++;
	} else if (yi == yi_max - 2){
		this->button_y1dv_up->Disable();
		yi++;
	} else {
		yi++;
	}
	this->scope_parameters->y1div_idx = yi;
	this->setSpinnerTrigLevelExtrema();
	this->scope_parameters->send_changes = true;
	this->updateY1div();

	return;
}

void GuiFrame::knobY1dvDw (wxCommandEvent& WXUNUSED(event))
{
	int yi = this->scope_parameters->y1div_idx;
	int yi_max = this->scope_parameters->ydiv_size;
	if (yi == 0) {
		return;
	} else if (yi == yi_max - 1) {
		this->button_y1dv_up->Enable();
		yi--;
	} else if (yi == 1){
		this->button_y1dv_dw->Disable();
		yi--;
	} else {
		yi--;
	}
	this->scope_parameters->y1div_idx = yi;
	this->setSpinnerTrigLevelExtrema();
	this->scope_parameters->send_changes = true;
	this->updateY1div();

	return;
}

void GuiFrame::updateY1div()
{
	std::stringstream	displayed_value;
	if (this->scope_parameters->y1_vps != 1.0) {
		displayed_value << "V scale: " << this->scope_parameters->list_ydiv_volts_txt[this->scope_parameters->y1div_idx] << " / div";
	} else {
		displayed_value << "V scale: " << this->scope_parameters->list_ydiv_samples_txt[this->scope_parameters->y1div_idx] << " / div";
	}
	this->statictext_value_y1dv->SetLabel(displayed_value.str());
	return;
}

void GuiFrame::knobY2dvUp (wxCommandEvent& WXUNUSED(event))
{
	int yi = this->scope_parameters->y2div_idx;
	int yi_max = this->scope_parameters->ydiv_size;
	if (yi == yi_max - 1) {
		return;
	} else if (yi == 0) {
		this->button_y2dv_dw->Enable();
		yi++;
	} else if (yi == yi_max - 2){
		this->button_y2dv_up->Disable();
		yi++;
	} else {
		yi++;
	}
	this->scope_parameters->y2div_idx = yi;
	this->setSpinnerTrigLevelExtrema();
	this->scope_parameters->send_changes = true;
	this->updateY2div();

	return;
}

void GuiFrame::knobY2dvDw (wxCommandEvent& WXUNUSED(event))
{
	int yi = this->scope_parameters->y2div_idx;
	int yi_max = this->scope_parameters->ydiv_size;
	if (yi == 0) {
		return;
	} else if (yi == yi_max - 1) {
		this->button_y2dv_up->Enable();
		yi--;
	} else if (yi == 1){
		this->button_y2dv_dw->Disable();
		yi--;
	} else {
		yi--;
	}
	this->scope_parameters->y2div_idx = yi;
	this->setSpinnerTrigLevelExtrema();
	this->scope_parameters->send_changes = true;
	this->updateY2div();

	return;
}

void GuiFrame::updateY2div()
{
	std::stringstream	displayed_value;
	if (this->scope_parameters->y2_vps != 1.0) {
		displayed_value << "V scale: " << this->scope_parameters->list_ydiv_volts_txt[this->scope_parameters->y2div_idx] << " / div";
	} else {
		displayed_value << "V scale: " << this->scope_parameters->list_ydiv_samples_txt[this->scope_parameters->y2div_idx] << " / div";
	}
	this->statictext_value_y2dv->SetLabel(displayed_value.str());
	return;
}

void GuiFrame::selectTrigChan(wxCommandEvent& WXUNUSED(event))
{
	this->scope_parameters->trig_channel = this->radiobox_trig_chan->GetSelection();
	this->setSpinnerTrigLevelExtrema();
	this->scope_parameters->send_changes = true;
	return;
}

void GuiFrame::selectTrigEdge(wxCommandEvent& WXUNUSED(event))
{
	this->scope_parameters->trig_edge = this->radiobox_trig_edge->GetSelection();
	this->scope_parameters->send_changes = true;
	return;
}

void GuiFrame::selectTrigLevel(wxCommandEvent& WXUNUSED(event))
{
	this->scope_parameters->trig_level = this->spinner_trig_level->GetValue();
	this->scope_parameters->send_changes = true;
	return;
}

void GuiFrame::setSpinnerTrigLevelExtrema()
{
	unsigned int chan = this->scope_parameters->trig_channel;
	double ymax = 4.0 * atof(this->scope_parameters->list_ydiv_samples[(chan == 0)? this->scope_parameters->y1div_idx : this->scope_parameters->y2div_idx].c_str());
	double previous_value = this->spinner_trig_level->GetValue();
	this->spinner_trig_level->SetRange(-ymax, ymax);
	this->spinner_trig_level->SetIncrement(ymax / 100);
	if (fabs(previous_value) > ymax) {
		wxCommandEvent ev(wxEVT_SPINCTRLDOUBLE, EVENT_SPINNER_TRIG_LVL);
		spinner_trig_level->GetEventHandler()->ProcessEvent(ev);
	}

	return;
}

void GuiFrame::selectChoiceAverages(wxCommandEvent& WXUNUSED(event))
{
	switch (this->choice_averages->GetSelection()) {
		case 0:
			this->scope_parameters->navg = CHOICES_AVERAGES_0_NR;
			break;
		case 1:
			this->scope_parameters->navg = CHOICES_AVERAGES_1_NR;
			break;
		case 2:
			this->scope_parameters->navg = CHOICES_AVERAGES_2_NR;
			break;
		case 3:
			this->scope_parameters->navg = CHOICES_AVERAGES_3_NR;
			break;
		case 4:
			this->scope_parameters->navg = CHOICES_AVERAGES_4_NR;
			break;
		case 5:
			this->scope_parameters->navg = CHOICES_AVERAGES_5_NR;
			break;
		default:
			this->scope_parameters->navg = 1;
			break;
	}
	this->scope_parameters->send_changes = true;

	return;
}

void GuiFrame::changedCalibrationCh1(wxCommandEvent& WXUNUSED(event))
{
	unsigned int user_value = (unsigned int) wxGetNumberFromUser("Insert calibration factor, i.e. an integer number corresponding to 1 Volt.", "[1,65535]", "Set calibration for Channel 1", 1, 1, 65535, this, wxDefaultPosition);

	char	*display_label = (char*) malloc(32 * sizeof(char));
	if (user_value == 1) {
		this->scope_parameters->y1_vps = 1.0;
		this->statictext_calibration_ch1->SetLabel("No calibration");
		this->statictext_calibration_ch1->Refresh();
	} else {
		this->scope_parameters->y1_vps = 1.0 / (double) user_value;
		sprintf(display_label, "%d units/V", user_value);
		this->statictext_calibration_ch1->SetLabel(display_label);
		this->statictext_calibration_ch1->Refresh();
	}
	this->updateY1div();
	this->scope_parameters->send_changes = true;

	free(display_label);
	return;
}

void GuiFrame::changedCalibrationCh2(wxCommandEvent& WXUNUSED(event))
{
	unsigned int user_value = (unsigned int) wxGetNumberFromUser("Insert calibration factor, i.e. an integer number corresponding to 1 Volt.", "[1,65535]", "Set calibration for Channel 2", 1, 1, 65535, this, wxDefaultPosition);

	char	*display_label = (char*) malloc(32 * sizeof(char));
	if (user_value == 1) {
		this->scope_parameters->y2_vps = 1.0;
		this->statictext_calibration_ch2->SetLabel("No calibration");
		this->statictext_calibration_ch2->Refresh();
	} else {
		this->scope_parameters->y2_vps = 1.0 / (double) user_value;
		sprintf(display_label, "%d units/V", user_value);
		this->statictext_calibration_ch2->SetLabel(display_label);
		this->statictext_calibration_ch2->Refresh();
	}
	this->updateY1div();
	this->scope_parameters->send_changes = true;

	free(display_label);
	return;
}

void GuiFrame::initializeConstants()
{
	this->scope_parameters->send_changes = true;
	this->scope_parameters->change_mode = false;
	this->scope_parameters->mode = 'a';
	this->scope_parameters->pause_command = false;
	this->button_runpause->SetLabel("Pause");
	this->scope_parameters->save_command = false;
	this->scope_parameters->output_file.clear();

	this->scope_parameters->list_tdiv.resize(16, "");
	this->scope_parameters->list_tdiv_txt.resize(16, "");
	this->scope_parameters->list_tdiv = {"5e-5", "1e-4", "2e-4", "5e-4", "1e-3", "2e-3", "5e-3", "1e-2", "2e-2", "5e-2", "1e-1", "2e-1", "5e-1", "1e+0", "2e+0", "5e+0"};
	this->scope_parameters->list_tdiv_txt = {"50 us", "0.1 ms", "0.2 ms", "0.5 ms", "1 ms", "2 ms", "5 ms", "10 ms", "20 ms", "50 ms", "0.1 s", "0.2 s", "0.5 s", "1 s", "2 s", "5 s"};
	this->scope_parameters->tdiv_idx = 4;

	this->scope_parameters->ydiv_size = 12;
	this->scope_parameters->list_ydiv_samples.resize(this->scope_parameters->ydiv_size, "");
	this->scope_parameters->list_ydiv_samples_txt.resize(this->scope_parameters->ydiv_size, "");
	this->scope_parameters->list_ydiv_volts.resize(this->scope_parameters->ydiv_size, "");
	this->scope_parameters->list_ydiv_volts_txt.resize(this->scope_parameters->ydiv_size, "");
	this->scope_parameters->list_ydiv_samples = {"4e+0", "8e+0", "2e+1", "4e+1", "8e+1", "2e+2", "4e+2", "8e+2", "2e+3", "4e+3", "8e+3", "2e+4"};
	this->scope_parameters->list_ydiv_samples_txt = {"4 units", "8 units", "20 units", "40 units", "80 units", "200 units", "400 units", "800 units", "2000 units", "4000 units", "8000 units", "20000 units"};
	this->scope_parameters->list_ydiv_volts = {"1e-3", "2e-3", "5e-3", "1e-2", "2e-2", "5e-2", "1e-1", "2e-1", "5e-1", "1e+0", "2e+0", "5e+1"};
	this->scope_parameters->list_ydiv_volts_txt = {"1 mV", "2 mV", "5 mV", "10 mV", "20 mV", "50 mV", "100 mV", "200 mV", "500 mV", "1 V", "2 V", "5 V"};
	this->scope_parameters->y1div_idx = this->scope_parameters->ydiv_size - 2;
	this->scope_parameters->y2div_idx = this->scope_parameters->ydiv_size - 2;

	this->scope_parameters->trig_edge = 0;
	this->scope_parameters->trig_channel = 0;
	this->scope_parameters->trig_level = 0.0;
	this->scope_parameters->y1_vps = 1.0;
	this->scope_parameters->y2_vps = 1.0;

	this->updateTdiv();
	this->updateY1div();
	this->updateY2div();

	if (this->scope_parameters->tdiv_idx == this->scope_parameters->list_tdiv.size() - 1)
		this->button_tdiv_up->Disable();
	else if (this->scope_parameters->tdiv_idx == 0)
		this->button_tdiv_dw->Disable();
	if (this->scope_parameters->y1div_idx == this->scope_parameters->ydiv_size - 1)
		this->button_y1dv_up->Disable();
	else if (this->scope_parameters->y1div_idx == 0)
		this->button_y1dv_dw->Disable();
	if (this->scope_parameters->y2div_idx == this->scope_parameters->ydiv_size - 1)
		this->button_y2dv_up->Disable();
	else if (this->scope_parameters->y2div_idx == 0)
		this->button_y2dv_dw->Disable();

	this->scope_parameters->navg = 1;
	this->choice_averages->SetSelection(0);

	this->radiobox_trig_chan->SetSelection(this->scope_parameters->trig_channel);
	this->radiobox_trig_edge->SetSelection(this->scope_parameters->trig_edge);
	this->setSpinnerTrigLevelExtrema();

	return;
}


void GuiFrame::selectFileToSave(wxCommandEvent& WXUNUSED(event))
{
	this->scope_parameters->pause_command = true;
	this->button_runpause->SetLabel("Run");

	wxString	selected_file_name;
	std::string	file_name;
	wxFileDialog saveFileDialog(this, _("Save data file"), "", "", "all files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveFileDialog.ShowModal() == wxID_OK) {
		selected_file_name = saveFileDialog.GetPath();
		file_name = selected_file_name.ToStdString();

		this->scope_parameters->output_file = file_name;
		this->scope_parameters->save_command = true;
	}
	this->scope_parameters->pause_command = false;
	this->button_runpause->SetLabel("Pause");

	return;
}

void GuiFrame::togglePauseRun(wxCommandEvent& WXUNUSED(event))
{
	if (this->scope_parameters->pause_command) {
		this->button_runpause->SetLabel("Pause");
		this->scope_parameters->pause_command = false;
	} else {
		this->button_runpause->SetLabel("Run");
		this->scope_parameters->pause_command = true;
	}
	return;
}

void GuiFrame::toggleMode(wxCommandEvent& WXUNUSED(event))
{
	if (this->scope_parameters->mode == 'a') {
		this->button_togglemode->SetLabel("MODE: X-Y");
		this->scope_parameters->mode = 'x';
		this->button_y1dv_up->Enable();
		this->button_y1dv_dw->Enable();
		this->button_y2dv_up->Enable();
		this->button_y2dv_dw->Enable();
		this->statictext_value_y1dv->Show();
		this->statictext_value_y2dv->Show();
		this->spinner_trig_level->Disable();
		this->statictext_label_trig->Disable();
		this->choice_averages->Disable();
	} else if (this->scope_parameters->mode == 'x') {
		this->button_togglemode->SetLabel("MODE: Digital");
		this->scope_parameters->mode = 'd';
		this->button_y1dv_up->Disable();
		this->button_y1dv_dw->Disable();
		this->button_y2dv_up->Disable();
		this->button_y2dv_dw->Disable();
		this->statictext_value_y1dv->Hide();
		this->statictext_value_y2dv->Hide();
		this->spinner_trig_level->Disable();
		this->statictext_label_trig->Disable();
		this->choice_averages->Disable();
	} else if (this->scope_parameters->mode == 'd') {
		this->button_togglemode->SetLabel("MODE: Voltmeter");
		this->scope_parameters->mode = 'v';
		this->button_y1dv_up->Disable();
		this->button_y1dv_dw->Disable();
		this->button_y2dv_up->Disable();
		this->button_y2dv_dw->Disable();
		this->statictext_value_y1dv->Hide();
		this->statictext_value_y2dv->Hide();
		this->spinner_trig_level->Disable();
		this->statictext_label_trig->Disable();
		this->choice_averages->Disable();
	} else if (this->scope_parameters->mode == 'v') {
		this->button_togglemode->SetLabel("MODE: Analog");
		this->scope_parameters->mode = 'a';
		this->button_y1dv_up->Enable();
		this->button_y1dv_dw->Enable();
		this->button_y2dv_up->Enable();
		this->button_y2dv_dw->Enable();
		this->statictext_value_y1dv->Show();
		this->statictext_value_y2dv->Show();
		this->spinner_trig_level->Enable();
		this->statictext_label_trig->Enable();
		this->choice_averages->Enable();
	}
	this->scope_parameters->change_mode = true;
	return;
}

void GuiFrame::showAboutDialog(wxCommandEvent& WXUNUSED(event))
{
	wxAboutDialogInfo info;
	info.SetName(wxT("xoscilloscope - digital oscilloscope"));
	info.SetVersion(wxT("1.0"));
	info.SetDescription(wxT("Graphical user interface to control the acquisition engine relying on the sound card."));
	info.SetCopyright(wxT("2020"));

	wxIcon		temp_icon;
	wxBitmap	temp_png = wxBITMAP_PNG_FROM_DATA(icon_64_xoscope);
	temp_icon.CopyFromBitmap(temp_png);
	info.SetIcon(temp_icon);

	wxAboutBox(info, this);

	return;
}
