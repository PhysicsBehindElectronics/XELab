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

wxIMPLEMENT_APP(MainApp);

MainApp::MainApp(){}

bool MainApp::OnInit()
{
	if (!wxApp::OnInit()) {
		return false;
	}

	wxImage::AddHandler(new wxPNGHandler);

	new GuiFrame("Oscilloscope console");

	return true;
}
