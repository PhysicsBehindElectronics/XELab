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

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <csignal>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <string>
#include <deque>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <alsa/asoundlib.h>

#define SOCKET_BUFFER_SIZE 128
#define BUF_SIZE 441
#define CHN_SIZE 2
#define SAMPLING_RATE 44100
#define REFRESH_GP 5000
#define HORIZ_DIVS 14
#define VERTC_DIVS 8
#define XY_DIVS 6
#define DIG_SR_SIZE 24
#define DIG_SIG_THR 8192

struct ScopeParameters {
	bool trig_rising_edge;
	unsigned int trig_chan;
	double tdiv;
	double y1div;
	double y2div;
	double trig_level;
	double y1_vps;
	double y2_vps;
	unsigned int navg;
};

enum osc_mode : unsigned int {
	MODE_ANALOG,
	MODE_XY,
	MODE_DIGITAL,
	MODE_VOLTMETER
};

bool requested_termination;
void signalHandler(int);

int oXs_hardware_setup_capture(snd_pcm_t*, snd_pcm_hw_params_t*, unsigned int*);
void oXs_default_scope_parameters(ScopeParameters*);
void oXs_setup_oscilloscope_screen(FILE*, char*);
void oXs_setup_gnuplot_analog_parameters(FILE*, char*, ScopeParameters*);
void oXs_setup_gnuplot_xy_parameters(FILE*, char*, ScopeParameters*);
void oXs_setup_gnuplot_digital_parameters(FILE*, char*, ScopeParameters*);
void oXs_setup_gnuplot_voltmeter_parameters(FILE*, char*, ScopeParameters*);
bool oXs_trigger_crossing(std::deque<std::vector<double> > &, std::vector<double> &, ScopeParameters*);
void oXs_digital_acquisition(std::vector<double> &, std::deque<std::vector<double> > &, const short*, int);
void oXs_voltmeter_acquisition(std::string &, std::string &, const std::deque< std::vector<double> > &, const ScopeParameters *);
bool oXs_trigger_digital(std::deque<std::vector<double> > &, std::vector<double> &, ScopeParameters*);
void oXs_save_output_file(std::string, std::vector< std::vector<double> > &);
