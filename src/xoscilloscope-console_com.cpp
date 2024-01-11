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
#include <cstdio>
#include <cstring>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef INCLUDED_MAINAPP
	#include "xoscilloscope-console_main.hpp"
	#define INCLUDED_MAINAPP
#endif

void GuiFrame::onWorkerStart (wxCommandEvent& WXUNUSED(event))
{
	WorkerThread *thread = new WorkerThread(this);
	if (thread->Create() != wxTHREAD_NO_ERROR) {
		wxMessageBox("Runtime error:\ncannot start console communication thread.", "Error", wxOK | wxICON_ERROR, NULL, wxDefaultCoord, wxDefaultCoord);
		thread->Delete();
		return;
	}
	thread->Run();
}

WorkerThread::WorkerThread (GuiFrame *frame)
: wxThread()
{
	parent_frame = frame;
	data_container = parent_frame->scope_parameters;
}

void WorkerThread::OnExit () {}

wxThread::ExitCode WorkerThread::Entry ()
{
	int sockfd, newsockfd, servlen, n;
	socklen_t clilen;
	struct sockaddr_un  cli_addr, serv_addr;
	char buf[SOCKET_BUFFER_SIZE];
	char paramsg[SOCKET_BUFFER_SIZE];
	char connection_name[32];
	sprintf(connection_name, "xoscilloscope.socket");

	if ((sockfd = socket(AF_UNIX,SOCK_STREAM,0)) < 0) {
		std::cerr << "Error while creating socket... exiting.\n";
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, connection_name);
	servlen=strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
	if(bind(sockfd,(struct sockaddr *)&serv_addr,servlen)<0) {
		std::cerr << "Error while binding socket... exiting.\n";
		exit(1);
	}

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) {
		std::cerr << "Error while accepting connection... exiting.\n";
		exit(1);
	}

	while (true) {
		bzero(paramsg, (SOCKET_BUFFER_SIZE - 2) * sizeof(char));
		n = read(newsockfd, buf, (SOCKET_BUFFER_SIZE - 2) * sizeof(char));
		if (buf[0] == 'g') {
			if (this->data_container->send_changes) {
				char temp_string[32];
				std::string tdiv_msg = this->data_container->list_tdiv[this->data_container->tdiv_idx];
				std::string y1dv_msg;
				if (this->data_container->y1_vps != 1.0)
					y1dv_msg = this->data_container->list_ydiv_volts[this->data_container->y1div_idx];
				else
					y1dv_msg = this->data_container->list_ydiv_samples[this->data_container->y1div_idx];
				std::string y2dv_msg;
				if (this->data_container->y2_vps != 1.0)
					y2dv_msg = this->data_container->list_ydiv_volts[this->data_container->y2div_idx];
				else
					y2dv_msg = this->data_container->list_ydiv_samples[this->data_container->y2div_idx];
				char edge = (this->data_container->trig_edge == 0)? 'r' : 'f';
				int ch = this->data_container->trig_channel + 1;
				sprintf(temp_string, "%+.2e", this->data_container->trig_level);
				std::string trig_level_msg = temp_string;
				bzero(temp_string, 32 * sizeof(char));
				sprintf(temp_string, "%+.2e", this->data_container->y1_vps);
				std::string y1factor_msg = temp_string;
				bzero(temp_string, 32 * sizeof(char));
				sprintf(temp_string, "%+.2e", this->data_container->y2_vps);
				std::string y2factor_msg = temp_string;
				bzero(temp_string, 32 * sizeof(char));
				sprintf(temp_string, "%.3d", this->data_container->navg);
				std::string navg_msg = temp_string;
				bzero(temp_string, 32 * sizeof(char));

				sprintf(paramsg, "y%s%s%s%c%d%s%s%s%s", tdiv_msg.c_str(), y1dv_msg.c_str(), y2dv_msg.c_str(), edge, ch, trig_level_msg.c_str(), y1factor_msg.c_str(), y2factor_msg.c_str(), navg_msg.c_str());
				this->data_container->send_changes = false;
			} else if (this->data_container->pause_command) {
				sprintf(paramsg, "p");
			} else if (this->data_container->save_command) {
				sprintf(paramsg, "s%s", this->data_container->output_file.c_str());
				this->data_container->save_command = false;
			} else if (this->data_container->change_mode) {
				sprintf(paramsg, "m%c", this->data_container->mode);
				this->data_container->change_mode = false;
			} else {
				sprintf(paramsg, "n");
			}
			write(newsockfd, paramsg, (SOCKET_BUFFER_SIZE - 2) * sizeof(char));
		} else {
			std::cerr << "Communication error: invalid request delivered to the console...\n";
		}
	}

	close(newsockfd);
	close(sockfd);

}
