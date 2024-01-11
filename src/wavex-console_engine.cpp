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
#include <cstring>
#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <alsa/asoundlib.h>

#define BUF_SIZE 441
#define CHN_SIZE 2
#define SAMPLING_RATE 44100

#ifndef INCLUDED_MAINAPP
	#include "wavex-console_main.hpp"
	#define INCLUDED_MAINAPP
#endif

int wXs_hardware_setup_playback(snd_pcm_t*, snd_pcm_hw_params_t*, unsigned int*);

void GuiFrame::onWorkerStart()
{
	wxThread	*thread = new WorkerThread(this);

	thread_shall_be_cancelled = false;
	if (thread->Create() != wxTHREAD_NO_ERROR) {
		wxMessageBox("Runtime error:\ncannot start engine thread.", "Error", wxOK | wxICON_ERROR, NULL, wxDefaultCoord, wxDefaultCoord);
		thread->Delete();
		return;
	}
	thread->Run();
	thread_is_running = true;
}

WorkerThread::WorkerThread (GuiFrame *frame)
: wxThread()
{
	parent_frame = frame;
	wave_parameters = parent_frame->wave_parameters;
}

void WorkerThread::OnExit () {}

wxThread::ExitCode WorkerThread::Entry ()
{
	snd_pcm_t *device_handle;
	snd_pcm_hw_params_t *device_parameters;
	unsigned int sample_rate = SAMPLING_RATE;

	if (snd_pcm_open(&device_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		std::cerr <<  "Could not open audio device <default>\n",
		exit(1);
	}
	wXs_hardware_setup_playback(device_handle, device_parameters, &sample_rate);

	int16_t* buf = (int16_t *) malloc(sizeof(int16_t) * BUF_SIZE * CHN_SIZE);
	double t = 0.0, dt = 1.0 / sample_rate;
	double f, A, d, y, triang_sign;
	int n = 0;
	long int nr_written, nr_available, nr_rewinded;
	clock_t old_clk, new_clk;
	double timer_ms = 0.0;
	old_clk = clock();
	while (true) {
		bzero(buf, BUF_SIZE * CHN_SIZE * sizeof(int16_t));
		for (int i = 0; i < BUF_SIZE * CHN_SIZE; i = i + 2) {
			if (this->wave_parameters->output_1) {
				f = this->wave_parameters->f1;
				A = this->wave_parameters->A1 / this->wave_parameters->y1_vps;
				d = this->wave_parameters->delay1;
				y = sin(8.0*atan(1.0)*f*(t-d));
				switch(this->wave_parameters->waveshape_1) {
					case 0 :
						buf[i] = (int16_t) floor(A * y);
						break;
					case 1 :
						triang_sign = (((int) floor(2.0*(t-d)*f + 0.5)) % 2 == 0)? 1.0 : -1.0;
						buf[i] = (int16_t) floor(A * 4.0 * f * ((t-d) - 1.0/(2.0*f) * floor(2.0*(t-d)*f + 0.5)) * triang_sign);
						break;
					case 2 :
						buf[i] = (int16_t) (y > 0)? floor(A) : ((y < 0)? floor(-A) : 0.0);
						break;
					default :
						buf[i] = (int16_t) 0;
						break;
				}
			} else {
				buf[i] = (int16_t) 0;
			}
			if (this->wave_parameters->output_2) {
				f = this->wave_parameters->f2;
				A = this->wave_parameters->A2 / this->wave_parameters->y2_vps;
				d = this->wave_parameters->delay2;
				y = sin(8.0*atan(1.0)*f*(t-d));
				switch(this->wave_parameters->waveshape_2) {
					case 0 :
						buf[i+1] = (int16_t) floor(A * y);
						break;
					case 1 :
						triang_sign = (((int) floor(2.0*(t-d)*f + 0.5)) % 2 == 0)? 1.0 : -1.0;
						buf[i+1] = (int16_t) floor(A * 4.0 * f * ((t-d) - 1.0/(2.0*f) * floor(2.0*(t-d)*f + 0.5)) * triang_sign);
						break;
					case 2 :
						buf[i+1] = (int16_t) (y > 0)? floor(A) : ((y < 0)? floor(-A) : 0.0);
						break;
					default :
						buf[i+1] = (int16_t) 0;
						break;
				}
			} else {
				buf[i+1] = (int16_t) 0;
			}
			t += dt;
		}
		if (t > 30.0)
			t = 0.0;

		nr_written = snd_pcm_writei(device_handle, buf, BUF_SIZE);
		if (nr_written == -EPIPE) {
			free(buf);
			snd_pcm_close(device_handle);
			snd_pcm_hw_params_free(device_parameters);
			usleep(10000);
			parent_frame->thread_is_running = false;
			parent_frame->onWorkerStart();
			return (wxThread::ExitCode) 1;
		}

		if (this->wave_parameters->changed) {
			std::cerr.flush();
			usleep(10000);
			snd_pcm_drop(device_handle);
			snd_pcm_close(device_handle);
			usleep(10000);
			if (snd_pcm_open(&device_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
				std::cerr <<  "Could not open audio device <default>\n",
				exit(1);
			}
			wXs_hardware_setup_playback(device_handle, device_parameters, &sample_rate);
			usleep(10000);
			this->wave_parameters->changed = false;
		}

		if (parent_frame->thread_shall_be_cancelled || TestDestroy()) {
			free(buf);
			snd_pcm_close(device_handle);
			snd_pcm_hw_params_free(device_parameters);
			parent_frame->thread_is_running = false;
			parent_frame->thread_shall_be_cancelled = false;
			return (wxThread::ExitCode) 0;
		}

		n++;
	}

	free(buf);
	snd_pcm_close(device_handle);
	snd_pcm_hw_params_free(device_parameters);
	parent_frame->thread_is_running = false;
	parent_frame->thread_shall_be_cancelled = false;
	return (wxThread::ExitCode) 0;
}


int wXs_hardware_setup_playback(snd_pcm_t* device_handle, snd_pcm_hw_params_t* device_parameters, unsigned int* sample_rate)
{
	int err;

	if (snd_pcm_hw_params_malloc(&device_parameters) < 0) {
		std::cerr <<  "Could not allocate hardware parameter structure\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params_any(device_handle, device_parameters)) < 0) {
		std::cerr <<  "Cannot initialize hardware parameter structure\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params_set_access(device_handle, device_parameters, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		std::cerr <<  "Cannot set access type\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params_set_format(device_handle, device_parameters, SND_PCM_FORMAT_S16_LE)) < 0) {
		std::cerr <<  "Cannot set sample format\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params_set_rate_near(device_handle, device_parameters, sample_rate, 0)) < 0) {
		std::cerr <<  "Cannot set sample rate\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params_set_channels(device_handle, device_parameters, 2)) < 0) {
		std::cerr <<  "Cannot set channel count\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params(device_handle, device_parameters)) < 0) {
		std::cerr <<  "Cannot set parameters\n";
		exit(1);
	}
	if ((err = snd_pcm_prepare(device_handle)) < 0) {
		std::cerr <<  "Cannot prepare audio interface for use\n";
		exit(1);
	}

	return 0;
}
