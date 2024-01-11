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

#include "xoscilloscope-engine_gnuplot.h"
#include "xoscilloscope-engine_main.h"

int main (int argc, char *argv[])
{
	int err, readbytes;
	int16_t * buf = (int16_t *) malloc(sizeof(int16_t) * BUF_SIZE * CHN_SIZE);
	snd_pcm_t *device_handle;
	snd_pcm_hw_params_t *device_parameters;
	unsigned int sample_rate = SAMPLING_RATE;

	requested_termination = false;
	signal(SIGINT, signalHandler);

	std::cerr << "Setting up acquisition device...";
	if (snd_pcm_open (&device_handle, "default", SND_PCM_STREAM_CAPTURE, 0) < 0) {
		std::cerr <<  "Could not open audio device <default>\n",
		exit(1);
	}
	oXs_hardware_setup_capture(device_handle, device_parameters, &sample_rate);
	snd_pcm_hw_params_free(device_parameters);
	std::cerr << " done.\n";

	std::cerr << "Setting up connection with console...";
	int sockfd, servlen,n;
	struct sockaddr_un serv_addr;
	char socket_buffer[SOCKET_BUFFER_SIZE];
	char connection_name[32];
	sprintf(connection_name, "xoscilloscope.socket");
	bzero((char *)&serv_addr,sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, connection_name);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
	if ((sockfd = socket(AF_UNIX, SOCK_STREAM,0)) < 0) {
		std::cerr << "Error in creating socket... exiting.\n";
		exit(1);
	}
	if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
		std::cerr << "Error in connecting to the console... exiting.\n";
		exit(1);
	}
	std::cerr << " done.\n";

	int pid;
	std::cerr << "Setting up oscilloscope display...";
	std::vector<double>			txy(3, 0.0);
	std::vector< std::vector<double> >	gnuplot_data;
	std::vector<double>			xy(2, 0.0);
	std::deque< std::vector<double> >	trigger_data;
	std::deque< std::vector<double> >	sr;
	std::deque< std::vector<double> >	accumulator_ch1;
	std::deque< std::vector<double> >	accumulator_ch2;
	std::vector<double>			aux_double_vec;
	std::string				string_voltmeter_1, string_voltmeter_2;
	ScopeParameters*			scope_parameters = (ScopeParameters *) malloc(sizeof(ScopeParameters));
	oXs_default_scope_parameters(scope_parameters);
	FILE*	gnuplot_pipe;
	char*	gnuplot_fifo = (char *) malloc(sizeof(char) * 64);
	char*	clean_fifo = (char *) malloc(sizeof(char) * 64);
	sprintf(gnuplot_fifo, "scope.fifo");
	sprintf(clean_fifo, "rm -f scope.fifo");
	system(clean_fifo);
	gnuplot_pipe = popen2(pid);
	setvbuf(gnuplot_pipe, NULL, _IONBF, 0);
	mkfifo(gnuplot_fifo, S_IRUSR | S_IWUSR);
	oXs_setup_oscilloscope_screen(gnuplot_pipe, gnuplot_fifo);
	oXs_setup_gnuplot_analog_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
	std::cerr << " done.\n";

	std::cerr << "Oscilloscope running.\n";
	double dt = 1.0 / (double) sample_rate;
	double t = 0.0;
	int niter = 0, ntrig = 0;
	bool triggered = false;
	bool pause_command = false;
	osc_mode operation_mode = MODE_ANALOG;
	while(!requested_termination) {
		int trace_size = ceil(scope_parameters->tdiv * HORIZ_DIVS * sample_rate);
		int nr_of_averages = scope_parameters->navg;
		triggered = false;
		trigger_data.clear();

		if (!pause_command) {
			if (operation_mode == MODE_ANALOG) {
				while (trigger_data.size() < trace_size / 2) {
					snd_pcm_readi(device_handle, buf, BUF_SIZE);
					for (int j = 0; (j < BUF_SIZE * CHN_SIZE); j = j + CHN_SIZE) {
						xy[0] = buf[j];
						xy[1] = buf[j+1];
						trigger_data.push_back(xy);
						if (trigger_data.size() > trace_size / 2)
							trigger_data.pop_front();
					}
				}

				ntrig = 0;
				while (!triggered) {
					snd_pcm_readi(device_handle, buf, BUF_SIZE);
					for (int j = 0; ((j < BUF_SIZE * CHN_SIZE) && (trigger_data.size() < trace_size)); j = j + CHN_SIZE) {
						xy[0] = buf[j];
						xy[1] = buf[j+1];
						if (!triggered && !oXs_trigger_crossing(trigger_data, xy, scope_parameters)) {
							trigger_data.pop_front();
						} else {
							triggered = true;
						}
						trigger_data.push_back(xy);
						ntrig++;
					}
					if (ntrig > 1.0 * trace_size) {
						std::cerr << "Trigger? (graphing anyway...)\n";
						break;
					}
				}

				while (trigger_data.size() < trace_size) {
					snd_pcm_readi(device_handle, buf, BUF_SIZE);
					for (int j = 0; ((j < BUF_SIZE * CHN_SIZE) && (trigger_data.size() < trace_size)); j = j + CHN_SIZE) {
						xy[0] = buf[j];
						xy[1] = buf[j+1];
						trigger_data.push_back(xy);
					}
				}

				if (nr_of_averages > 1) {
					aux_double_vec.clear();
					for (int j = 0; j < trigger_data.size(); j++)
						aux_double_vec.push_back(trigger_data[j][0] * scope_parameters->y1_vps);
					accumulator_ch1.push_back(aux_double_vec);
					if (accumulator_ch1.size() > nr_of_averages)
						accumulator_ch1.pop_front();

					aux_double_vec.clear();
					for (int j = 0; j < trigger_data.size(); j++)
						aux_double_vec.push_back(trigger_data[j][1] * scope_parameters->y2_vps);
					accumulator_ch2.push_back(aux_double_vec);
					if (accumulator_ch2.size() > nr_of_averages)
						accumulator_ch2.pop_front();

					gnuplot_data.clear();
					t = -0.5*trigger_data.size()*dt;
					for (int j = 0; j < trigger_data.size(); j++) {
						txy[0] = t;
						txy[1] = 0.0;
						for (int i = 0; i < accumulator_ch1.size(); i++)
							txy[1] += accumulator_ch1[i][j] / (double) accumulator_ch1.size();
						txy[2] = 0.0;
						for (int i = 0; i < accumulator_ch2.size(); i++)
							txy[2] += accumulator_ch2[i][j] / (double) accumulator_ch2.size();
						gnuplot_data.push_back(txy);
						t += dt;
					}
				} else {
					gnuplot_data.clear();
					t = -0.5*trigger_data.size()*dt;
					for (int j = 0; j < trigger_data.size(); j++) {
						txy[0] = t;
						txy[1] = trigger_data[j][0] * scope_parameters->y1_vps;
						txy[2] = trigger_data[j][1] * scope_parameters->y2_vps;
						gnuplot_data.push_back(txy);
						t += dt;
					}
				}

			} else if (operation_mode == MODE_XY) {
				while (trigger_data.size() < trace_size) {
					snd_pcm_readi(device_handle, buf, BUF_SIZE);
					for (int j = 0; (j < BUF_SIZE * CHN_SIZE); j = j + CHN_SIZE) {
						xy[0] = buf[j];
						xy[1] = buf[j+1];
						trigger_data.push_back(xy);
						if (trigger_data.size() > trace_size)
							trigger_data.pop_front();
					}
				}
				gnuplot_data.clear();
				t = -0.5*trigger_data.size()*dt;
				for (int j = 0; j < trigger_data.size(); j++) {
					txy[0] = t;
					txy[1] = trigger_data[j][0] * scope_parameters->y1_vps;
					txy[2] = trigger_data[j][1] * scope_parameters->y2_vps;
					gnuplot_data.push_back(txy);
					t += dt;
				}
			} else if (operation_mode == MODE_DIGITAL) {
				while (trigger_data.size() < trace_size / 2) {
					snd_pcm_readi(device_handle, buf, BUF_SIZE);
					for (int j = 0; (j < BUF_SIZE * CHN_SIZE); j = j + CHN_SIZE) {
						oXs_digital_acquisition(xy, sr, buf, j);
						trigger_data.push_back(xy);
						if (trigger_data.size() > trace_size / 2)
							trigger_data.pop_front();
					}
				}

				ntrig = 0;
				while (!triggered) {
					snd_pcm_readi(device_handle, buf, BUF_SIZE);
					for (int j = 0; ((j < BUF_SIZE * CHN_SIZE) && (trigger_data.size() < trace_size)); j = j + CHN_SIZE) {
						oXs_digital_acquisition(xy, sr, buf, j);
						if (!triggered && !oXs_trigger_digital(trigger_data, xy, scope_parameters)) {
							trigger_data.pop_front();
						} else {
							triggered = true;
						}
						trigger_data.push_back(xy);
						ntrig++;
					}
					if (ntrig > 1.0 * trace_size) {
						std::cerr << "Trigger? [graphing anyway...]\n";
						break;
					}
				}

				while (trigger_data.size() < trace_size) {
					snd_pcm_readi(device_handle, buf, BUF_SIZE);
					for (int j = 0; ((j < BUF_SIZE * CHN_SIZE) && (trigger_data.size() < trace_size)); j = j + CHN_SIZE) {
						oXs_digital_acquisition(xy, sr, buf, j);
						trigger_data.push_back(xy);
					}
				}

				gnuplot_data.clear();
				t = -0.5*trigger_data.size()*dt;
				for (int j = 0; j < trigger_data.size(); j++) {
					txy[0] = t;
					txy[1] = trigger_data[j][0];
					txy[2] = trigger_data[j][1];
					gnuplot_data.push_back(txy);
					t += dt;
				}
			} else if (operation_mode == MODE_VOLTMETER) {
				while (trigger_data.size() < trace_size) {
					snd_pcm_readi(device_handle, buf, BUF_SIZE);
					for (int j = 0; (j < BUF_SIZE * CHN_SIZE); j = j + CHN_SIZE) {
						xy[0] = buf[j];
						xy[1] = buf[j+1];
						trigger_data.push_back(xy);
						if (trigger_data.size() > trace_size)
							trigger_data.pop_front();
					}
				}
				gnuplot_data.clear();
				t = -0.5*trigger_data.size()*dt;
				for (int j = 0; j < trigger_data.size(); j++) {
					txy[0] = t;
					txy[1] = 0.0;
					txy[2] = 0.0;
					gnuplot_data.push_back(txy);
					t += dt;
				}
				oXs_voltmeter_acquisition(string_voltmeter_1, string_voltmeter_2, trigger_data, scope_parameters);
			}
		} else {
			while (trigger_data.size() < ((trace_size > 4410)? 4410 : trace_size)) {
				snd_pcm_readi(device_handle, buf, BUF_SIZE);
				for (int j = 0; (j < BUF_SIZE * CHN_SIZE); j = j + CHN_SIZE) {
					xy[0] = buf[j];
					xy[1] = buf[j+1];
					trigger_data.push_back(xy);
				}
			}
		}

		if (!pause_command) {
			if (niter % REFRESH_GP == 0) {
				if (operation_mode == MODE_ANALOG) {
					GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "plot", "u 1:2 axis x1y1 w l lw 3 lc rgb 'yellow', \"\" u 1:3 axis x1y2 w l lw 3 lc rgb 'cyan'", gnuplot_data);
				} else if (operation_mode == MODE_XY) {
					GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "plot", "u 2:3 w l lw 2 lc rgb 'magenta'", gnuplot_data);
				} else if (operation_mode == MODE_DIGITAL) {
					GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "plot", "u 1:($2+1.2) axis x1y1 w l lw 3 lc rgb 'yellow', \"\" u 1:3 axis x1y2 w l lw 3 lc rgb 'cyan'", gnuplot_data);
				} else if (operation_mode == MODE_VOLTMETER) {
					GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", string_voltmeter_1.c_str(), gnuplot_data);
					GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", string_voltmeter_2.c_str(), gnuplot_data);
					GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "plot", "u 1:2 axis x1y1 w l lw 3 lc rgb 'yellow', \"\" u 1:3 axis x1y2 w l lw 3 lc rgb 'cyan'", gnuplot_data);
				}
				usleep(10000);
				niter = 0;
			} else if (niter % REFRESH_GP == (REFRESH_GP - 1)){
				kill(-pid, 9);
				pclose2(gnuplot_pipe, pid);
				usleep(10000);
				gnuplot_pipe = popen2(pid);
				oXs_setup_oscilloscope_screen(gnuplot_pipe, gnuplot_fifo);
				if (operation_mode == MODE_ANALOG) {
					oXs_setup_gnuplot_analog_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				} else if (operation_mode == MODE_XY) {
					oXs_setup_gnuplot_xy_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				} else if (operation_mode == MODE_DIGITAL) {
					oXs_setup_gnuplot_digital_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				} else if (operation_mode == MODE_VOLTMETER) {
					oXs_setup_gnuplot_voltmeter_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				}
				usleep(10000);
			} else {
				if (operation_mode == MODE_VOLTMETER) {
					GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", string_voltmeter_1.c_str(), gnuplot_data);
					GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", string_voltmeter_2.c_str(), gnuplot_data);
					if (niter < (REFRESH_GP - 250))
						niter = REFRESH_GP - 250;
				}
				GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "refresh", "", gnuplot_data);
				usleep(10000);
			}
		} else {
			usleep(10000);
		}

		if (requested_termination)
			break;
		bzero(socket_buffer, SOCKET_BUFFER_SIZE);
		socket_buffer[0] = 'g';
		socket_buffer[1] = '\0';
		write(sockfd, socket_buffer, strlen(socket_buffer));
		readbytes = read(sockfd, socket_buffer, SOCKET_BUFFER_SIZE-2);
		if (readbytes < 1) {
			kill(-pid, 9);
			pclose2(gnuplot_pipe, pid);
			exit(1);
		}
		if (socket_buffer[0] == 'y') {
			std::string socket_buffer_msg(socket_buffer);
			socket_buffer_msg.erase(socket_buffer_msg.begin());
			std::string msg_tdiv = socket_buffer_msg.substr(0, 4);
			std::string msg_y1div = socket_buffer_msg.substr(4, 4);
			std::string msg_y2div = socket_buffer_msg.substr(8, 4);
			std::string msg_tredge = socket_buffer_msg.substr(12, 1);
			std::string msg_trchan = socket_buffer_msg.substr(13, 1);
			std::string msg_trlevel = socket_buffer_msg.substr(14, 9);
			std::string msg_y1vps = socket_buffer_msg.substr(23, 9);
			std::string msg_y2vps = socket_buffer_msg.substr(32, 9);
			std::string msg_navg = socket_buffer_msg.substr(41, 3);

			scope_parameters->tdiv = atof(msg_tdiv.c_str());
			scope_parameters->y1div = atof(msg_y1div.c_str());
			scope_parameters->y2div = atof(msg_y2div.c_str());
			scope_parameters->trig_rising_edge = (msg_tredge == "r")? true : false;
			scope_parameters->trig_chan = atoi(msg_trchan.c_str());
			scope_parameters->trig_level = atof(msg_trlevel.c_str());
			scope_parameters->y1_vps = atof(msg_y1vps.c_str());
			scope_parameters->y2_vps = atof(msg_y2vps.c_str());
			scope_parameters->navg = atoi(msg_navg.c_str());

			accumulator_ch1.clear();
			accumulator_ch2.clear();
			kill(-pid, 9);
			pclose2(gnuplot_pipe, pid);
			usleep(10000);
			gnuplot_pipe = popen2(pid);
			oXs_setup_oscilloscope_screen(gnuplot_pipe, gnuplot_fifo);
			if (operation_mode == MODE_ANALOG) {
				oXs_setup_gnuplot_analog_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "plot", "u 1:2 axis x1y1 w l lw 3 lc rgb 'yellow', \"\" u 1:3 axis x1y2 w l lw 3 lc rgb 'cyan'", gnuplot_data);
			} else if (operation_mode == MODE_XY) {
				oXs_setup_gnuplot_xy_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "plot", "u 2:3 w l lw 2 lc rgb 'magenta'", gnuplot_data);
			} else if (operation_mode == MODE_DIGITAL) {
				oXs_setup_gnuplot_digital_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "plot", "u 1:($2+1.2) axis x1y1 w l lw 3 lc rgb 'yellow', \"\" u 1:3 axis x1y2 w l lw 3 lc rgb 'cyan'", gnuplot_data);
			} else if (operation_mode == MODE_VOLTMETER) {
				oXs_setup_gnuplot_voltmeter_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", string_voltmeter_1.c_str(), gnuplot_data);
				GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", string_voltmeter_2.c_str(), gnuplot_data);
				GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "plot", "u 1:($2+1.2) axis x1y1 w l lw 3 lc rgb 'yellow', \"\" u 1:3 axis x1y2 w l lw 3 lc rgb 'cyan'", gnuplot_data);
			}
			usleep(10000);
			pause_command = false;
		} else if (socket_buffer[0] == 'p') {
			pause_command = true;
		} else if (socket_buffer[0] == 's') {
			std::string socket_buffer_msg(socket_buffer);
			socket_buffer_msg.erase(socket_buffer_msg.begin());
			oXs_save_output_file(socket_buffer_msg, gnuplot_data);
			pause_command = false;
		} else if (socket_buffer[0] == 'm') {
			kill(-pid, 9);
			pclose2(gnuplot_pipe, pid);
			usleep(10000);
			gnuplot_pipe = popen2(pid);
			oXs_setup_oscilloscope_screen(gnuplot_pipe, gnuplot_fifo);
			if (socket_buffer[1] == 'a') {
				oXs_setup_gnuplot_analog_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				operation_mode = MODE_ANALOG;
				niter = -1;
			} else if (socket_buffer[1] == 'x') {
				oXs_setup_gnuplot_xy_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				operation_mode = MODE_XY;
				niter = -1;
			} else if (socket_buffer[1] == 'd') {
				oXs_setup_gnuplot_digital_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				operation_mode = MODE_DIGITAL;
				niter = -1;
			} else if (socket_buffer[1] == 'v') {
				oXs_setup_gnuplot_voltmeter_parameters(gnuplot_pipe, gnuplot_fifo, scope_parameters);
				operation_mode = MODE_VOLTMETER;
				niter = -1;
			}
		} else if (socket_buffer[0] == 'n') {
			pause_command = false;
		} else {
			std::cerr << "Communication error! Anomalous message found on socket... Try restarting the program.\n";
			usleep(1000000);
			pause_command = false;
		}
		niter++;
	}

	free(buf);
	snd_pcm_close(device_handle);
	close(sockfd);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "execute", "q", gnuplot_data);
	usleep(100000);
	free(gnuplot_fifo);
	kill(-pid, 9);
	pclose2(gnuplot_pipe, pid);
	std::cerr << " everything stopped correctly.\n";
	exit(0);
}

bool oXs_trigger_crossing(std::deque<std::vector<double> > & data, std::vector<double> & xy_new, ScopeParameters* scope_parameters)
{
	bool crossed = false;

	std::vector <double> xy_last = data.back();
	double y_new = xy_new[scope_parameters->trig_chan - 1];
	double y_last = xy_last[scope_parameters->trig_chan - 1];

	if (scope_parameters->trig_rising_edge) {
		if ((y_last < scope_parameters->trig_level) && (y_new >= scope_parameters->trig_level))
			crossed = true;
	} else {
		if ((y_last > scope_parameters->trig_level) && (y_new <= scope_parameters->trig_level))
			crossed = true;
	}

	return crossed;
}

void oXs_digital_acquisition(std::vector<double> & xy, std::deque< std::vector<double> > & sr, const int16_t* buf, int j)
{
	double m0 = 0.0, m1 = 0.0, s0 = 0.0, s1 = 0.0;
	std::vector<double> x(2, 0.0);
	x[0] = buf[j];
	x[1] = buf[j+1];
	sr.push_back(x);
	if (sr.size() > DIG_SR_SIZE)
		sr.pop_front();
	for (int i = 0; i < sr.size(); i++) {
		m0 += sr[i][0];
		m1 += sr[i][1];
		s0 += sr[i][0]*sr[i][0];
		s1 += sr[i][1]*sr[i][1];
	}
	m0 /= (double) sr.size();
	m1 /= (double) sr.size();
	s0 /= (double) sr.size();
	s1 /= (double) sr.size();
	s0 -= m0*m0;
	s1 -= m1*m1;

	xy[0] = (sqrt(s0) < DIG_SIG_THR)? 0 : 1;
	xy[1] = (sqrt(s1) < DIG_SIG_THR)? 0 : 1;

	return;
}

bool oXs_trigger_digital(std::deque<std::vector<double> > & data, std::vector<double> & xy_new, ScopeParameters* scope_parameters)
{
	bool crossed = false;

	std::vector <double> xy_last = data.back();
	double y_new = xy_new[scope_parameters->trig_chan - 1];
	double y_last = xy_last[scope_parameters->trig_chan - 1];

	if (scope_parameters->trig_rising_edge) {
		if ((y_last == 0) && (y_new == 1))
			crossed = true;
	} else {
		if ((y_last == 1) && (y_new == 0))
			crossed = true;
	}

	return crossed;
}

void oXs_voltmeter_acquisition(std::string & string_voltmeter_1, std::string & string_voltmeter_2, const std::deque< std::vector<double> > & collected_data, const ScopeParameters * scope_parameters)
{
	double V1 = 0.0, V2 = 0.0;

	for (int i = 0; i < collected_data.size(); i++) {
		V1 += fabs(collected_data[i][0]);
		V2 += fabs(collected_data[i][1]);
	}
	V1 *= 2.0 * scope_parameters->y1_vps / (double) collected_data.size();
	V2 *= 2.0 * scope_parameters->y2_vps / (double) collected_data.size();

	string_voltmeter_1.clear();
	string_voltmeter_2.clear();
	char *str_V1 = (char *) malloc(sizeof(char) * 256);
	char *str_V2 = (char *) malloc(sizeof(char) * 256);
	if (scope_parameters->y1_vps != 1.0) {
		sprintf(str_V1, "label 1 \"Ch1 = %.4f V\" at 0,0.5 center textcolor rgb '#d0d0d0' font \"mbfont:Courier,24\"", V1);
	} else {
		sprintf(str_V1, "label 1 \"Ch1 = %.f (a.u.)\" at 0,0.5 center textcolor rgb '#d0d0d0' font \"mbfont:Courier,24\"", V1);
	}
	if (scope_parameters->y2_vps != 1.0) {
		sprintf(str_V2, "label 2 \"Ch2 = %.4f V\" at 0,-0.5 center textcolor rgb '#d0d0d0' font \"mbfont:Courier,24\"", V2);
	} else {
		sprintf(str_V2, "label 2 \"Ch2 = %.f (a.u.)\" at 0,-0.5 center textcolor rgb '#d0d0d0' font \"mbfont:Courier,24\"", V2);
	}

	string_voltmeter_1 = str_V1;
	string_voltmeter_2 = str_V2;

	free(str_V1);
	free(str_V2);

	return;
}

void signalHandler(int signum)
{
	std::cerr << "\nTerminating...";
	requested_termination = true;
	return;
}

void oXs_save_output_file(std::string file_name_and_path, std::vector< std::vector<double> > & data_txy)
{
	std::ofstream	output_file;
	output_file.open(file_name_and_path.c_str(), std::ofstream::out);
	for (int i = 0; i < data_txy.size(); i++) {
		for (int j = 0; j < data_txy[j].size(); j++) {
			if (j > 0) output_file << "\t";
			output_file << data_txy[i][j];
		}
		output_file << "\n";
	}
	output_file.close();

	std::cerr << "Data saved at '" << file_name_and_path << "'\n";

	return;
}

void oXs_default_scope_parameters(ScopeParameters* scope_parameters)
{
	scope_parameters->tdiv = 1e-4;
	scope_parameters->y1div = 1e4;
	scope_parameters->y2div = 1e4;
	scope_parameters->trig_level = 0.0;
	scope_parameters->trig_chan = 1;
	scope_parameters->trig_rising_edge = true;
	scope_parameters->y1_vps = 1.0;
	scope_parameters->y2_vps = 1.0;
	scope_parameters->navg = 1;

	return;
}

void oXs_setup_gnuplot_analog_parameters(FILE* gnuplot_pipe, char* gnuplot_fifo, ScopeParameters* scope_parameters)
{
	double tlim = scope_parameters->tdiv * HORIZ_DIVS / 2.0;
	double y1lim = scope_parameters->y1div * VERTC_DIVS / 2.0;
	double y2lim = scope_parameters->y2div * VERTC_DIVS / 2.0;

	char* xrange = (char *) malloc(sizeof(char) * 128);
	char* y1range = (char *) malloc(sizeof(char) * 128);
	char* y2range = (char *) malloc(sizeof(char) * 128);
	char* xtics = (char *) malloc(sizeof(char) * 128);
	char* y1tics = (char *) malloc(sizeof(char) * 128);
	char* y2tics = (char *) malloc(sizeof(char) * 128);

	sprintf(xrange, "xrange [%f:%f]", -tlim, tlim);
	sprintf(y1range, "yrange [%f:%f]", -y1lim, y1lim);
	sprintf(y2range, "y2range [%f:%f]", -y2lim, y2lim);
	sprintf(xtics, "xtics %f, %f, %f format \"\"", -tlim, scope_parameters->tdiv, tlim);
	sprintf(y1tics, "ytics %f, %f, %f", -y1lim, scope_parameters->y1div, y1lim);
	sprintf(y2tics, "y2tics %f, %f, %f", -y2lim, scope_parameters->y2div, y2lim);

	std::vector< std::vector<double> > dummy;
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", xrange, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y1range, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y2range, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", xtics, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y1tics, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y2tics, dummy);
	if (scope_parameters->y1_vps != 1.0)
		GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "ylabel \"Channel 1 (V)\" textcolor rgb '#d0d0d0' offset 0,0", dummy);
	else
		GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "ylabel \"Channel 1 (a.u.)\" textcolor rgb '#d0d0d0' offset 0,0", dummy);

	if (scope_parameters->y2_vps != 1.0)
		GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "y2label \"Channel 2 (V)\" textcolor rgb '#d0d0d0' offset 0,0", dummy);
	else
		GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "y2label \"Channel 2 (a.u.)\" textcolor rgb '#d0d0d0' offset 0,0", dummy);

	free(xrange);
	free(y1range);
	free(y2range);
	free(xtics);
	free(y1tics);
	free(y2tics);

	return;
}

void oXs_setup_gnuplot_xy_parameters(FILE* gnuplot_pipe, char* gnuplot_fifo, ScopeParameters* scope_parameters)
{
	double y1lim = scope_parameters->y1div * XY_DIVS / 2.0;
	double y2lim = scope_parameters->y2div * XY_DIVS / 2.0;

	char* xrange = (char *) malloc(sizeof(char) * 128);
	char* y1range = (char *) malloc(sizeof(char) * 128);
	char* xtics = (char *) malloc(sizeof(char) * 128);
	char* y1tics = (char *) malloc(sizeof(char) * 128);
	char* y2tics = (char *) malloc(sizeof(char) * 128);

	sprintf(xrange, "xrange [%f:%f]", -y1lim, y1lim);
	sprintf(y1range, "yrange [%f:%f]", -y2lim, y2lim);
	sprintf(xtics, "xtics %f, %f, %f format \"%%.3f\"", -y1lim, scope_parameters->y1div, y1lim);
	sprintf(y1tics, "ytics %f, %f, %f format \"%%.3f\"", -y2lim, scope_parameters->y2div, y2lim);
	sprintf(y2tics, "y2tics format \"\"");

	std::vector< std::vector<double> > dummy;
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "size ratio 1", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", xrange, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y1range, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", xtics, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y1tics, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y2tics, dummy);
	if (scope_parameters->y1_vps != 1.0)
		GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "xlabel \"Channel 1 (V)\" textcolor rgb '#d0d0d0' offset 0,0", dummy);
	else
		GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "xlabel \"Channel 1 (a.u.)\" textcolor rgb '#d0d0d0' offset 0,0", dummy);
	if (scope_parameters->y2_vps != 1.0)
		GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "y1label \"Channel 2 (V)\" textcolor rgb '#d0d0d0' offset 0,0", dummy);
	else
		GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "y1label \"Channel 2 (a.u.)\" textcolor rgb '#d0d0d0' offset 0,0", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "y2label \"\"", dummy);

	free(xrange);
	free(y1range);
	free(xtics);
	free(y1tics);
	free(y2tics);

	return;
}

void oXs_setup_gnuplot_digital_parameters(FILE* gnuplot_pipe, char* gnuplot_fifo, ScopeParameters* scope_parameters)
{
	double tlim = scope_parameters->tdiv * HORIZ_DIVS / 2.0;

	char* xrange = (char *) malloc(sizeof(char) * 128);
	char* y1range = (char *) malloc(sizeof(char) * 128);
	char* y2range = (char *) malloc(sizeof(char) * 128);
	char* xtics = (char *) malloc(sizeof(char) * 128);
	char* y1tics = (char *) malloc(sizeof(char) * 128);
	char* y2tics = (char *) malloc(sizeof(char) * 128);

	sprintf(xrange, "xrange [%f:%f]", -tlim, tlim);
	sprintf(y1range, "yrange [-0.2:2.4]");
	sprintf(y2range, "y2range [-0.2:2.4]");
	sprintf(xtics, "xtics %f, %f, %f format \"\"", -tlim, scope_parameters->tdiv, tlim);
	sprintf(y1tics, "ytics (\"0\" 1.2, \"1\" 2.2)");
	sprintf(y2tics, "y2tics 0, 1, 1");

	std::vector< std::vector<double> > dummy;
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", xrange, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y1range, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y2range, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", xtics, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y1tics, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y2tics, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "ylabel \"Channel 1\" textcolor rgb '#d0d0d0' offset 0,7", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "y2label \"Channel 2\" textcolor rgb '#d0d0d0' offset 0,-6", dummy);

	free(xrange);
	free(y1range);
	free(y2range);
	free(xtics);
	free(y1tics);
	free(y2tics);

	return;
}

void oXs_setup_gnuplot_voltmeter_parameters(FILE* gnuplot_pipe, char* gnuplot_fifo, ScopeParameters* scope_parameters)
{
	double tlim = scope_parameters->tdiv * HORIZ_DIVS / 2.0;

	char* xrange = (char *) malloc(sizeof(char) * 128);
	char* y1range = (char *) malloc(sizeof(char) * 128);
	char* y2range = (char *) malloc(sizeof(char) * 128);
	char* xtics = (char *) malloc(sizeof(char) * 128);
	char* y1tics = (char *) malloc(sizeof(char) * 128);
	char* y2tics = (char *) malloc(sizeof(char) * 128);

	sprintf(xrange, "xrange [%f:%f]", -tlim, tlim);
	sprintf(y1range, "yrange [-1:1]");
	sprintf(y2range, "y2range [-1:1]");
	sprintf(xtics, "xtics");
	sprintf(y1tics, "ytics");
	sprintf(y2tics, "y2tics");

	std::vector< std::vector<double> > dummy;
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", xrange, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y1range, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", y2range, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "unset", xtics, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "unset", y1tics, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "unset", y2tics, dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "unset", "ylabel", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "unset", "y2label", dummy);

	free(xrange);
	free(y1range);
	free(y2range);
	free(xtics);
	free(y1tics);
	free(y2tics);

	return;
}

void oXs_setup_oscilloscope_screen(FILE* gnuplot_pipe, char* gnuplot_fifo)
{
	std::vector< std::vector<double> >	dummy;
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "term x11 background rgb '#151515' size 1000,500 position 50,550 font \"mbfont:Courier,18\"", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "unset", "key", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "style line 12 lc rgb '#c0c0c0' dt 3 lw 0.2", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", " grid ls 12", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "xtics textcolor rgb '#d0d0d0'", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "ytics textcolor rgb '#d0d0d0'", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "y2tics textcolor rgb '#d0d0d0'", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "xlabel \"Time (s)\" textcolor rgb '#d0d0d0'", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "ylabel \"Channel 1 (V)\" textcolor rgb '#d0d0d0' offset 0,0", dummy);
	GnuplotInterface(gnuplot_pipe, gnuplot_fifo, "set", "y2label \"Channel 2 (V)\" textcolor rgb '#d0d0d0' offset 0,0", dummy);

	return;
}

int oXs_hardware_setup_capture(snd_pcm_t* device_handle, snd_pcm_hw_params_t* device_parameters, unsigned int* sample_rate)
{
	int err;
	if (snd_pcm_hw_params_malloc(&device_parameters) < 0) {
		std::cerr <<  "Could not allocate hardware parameter structure\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params_any(device_handle, device_parameters)) < 0) {
		std::cerr <<  "cannot initialize hardware parameter structure\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params_set_access(device_handle, device_parameters, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		std::cerr <<  "cannot set access type\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params_set_format(device_handle, device_parameters, SND_PCM_FORMAT_S16_LE)) < 0) {
		std::cerr <<  "cannot set sample format\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params_set_rate_near(device_handle, device_parameters, sample_rate, 0)) < 0) {
		std::cerr <<  "cannot set sample rate\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params_set_channels(device_handle, device_parameters, 2)) < 0) {
		std::cerr <<  "cannot set channel count\n";
		exit(1);
	}
	if ((err = snd_pcm_hw_params(device_handle, device_parameters)) < 0) {
		std::cerr <<  "cannot set parameters\n";
		exit(1);
	}
	snd_pcm_nonblock(device_handle, 0);
	if ((err = snd_pcm_prepare(device_handle)) < 0) {
		std::cerr <<  "cannot prepare audio interface for use\n";
		exit(1);
	}

	return 0;
}
