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

int GnuplotInterface(FILE* gnuplotPipe, const char* fifo_name, const char* command, const char* content, std::vector< std::vector<double> >& pointSet) {

	if (!(strcmp(command, "wait"))) {
		usleep((int) (1.e6*atof(content)));
	} else if (!(strcmp(command, "execute"))) {
		fprintf(gnuplotPipe, "%s\n", content);
		fflush(gnuplotPipe);
	} else if (strstr(command, "set")) {
		fprintf(gnuplotPipe, "%s %s\n", command, content);
		fflush(gnuplotPipe);
	} else if (strstr(command, "plot")) {
		fprintf(gnuplotPipe, "%s %s%s%s %s\n", command, "\"", fifo_name, "\"", (strlen(content))? content : "");
		fflush(gnuplotPipe);
		std::ofstream data(fifo_name);
		for (int n = 0; n < pointSet.size(); n++) {
			if (pointSet[n].size() == 0)
				continue;
			data << pointSet[n][0];
			for(int m = 1; m < pointSet[n].size(); m++)
				data << "\t" << pointSet[n][m];
			data << "\n";
		}
		data.flush();
		data.close();
	} else if (strstr(command, "refresh")) {
		fprintf(gnuplotPipe, "rep\n");
		fflush(gnuplotPipe);
		std::ofstream data(fifo_name);
		for (int n = 0; n < pointSet.size(); n++) {
			if (pointSet[n].size() == 0)
				data << "\n\n";
			data << pointSet[n][0];
			for(int m = 1; m < pointSet[n].size(); m++)
				data << "\t" << pointSet[n][m];
			data << "\n";
		}
		data.flush();
		data.close();
	}

	return 0;
}

FILE * popen2(int & pid)
{
	pid_t child_pid;
	int fd[2];
	pipe(fd);

	if((child_pid = fork()) == -1)
		exit(1);

	if (child_pid == 0) {
		close(fd[1]);
		dup2(fd[0], 0);
		setpgid(child_pid, child_pid);
		execl("/bin/sh", "/bin/sh", "-c", "gnuplot 2> /dev/null", NULL);
		exit(0);
	} else {
		close(fd[0]);
	}
	pid = child_pid;
	return fdopen(fd[1], "w");
}

int pclose2(FILE * fp, pid_t pid)
{
	int stat;
	fclose(fp);
	while (waitpid(pid, &stat, 0) == -1) {
		if (errno != EINTR) {
		    stat = -1;
		    break;
		}
	}
	return stat;
}
