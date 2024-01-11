# --------------------------------------------------------------------------
#
# This file is part of the RemoteLab software package.
#
# Version 1.0 - September 2020
#
#
# The RemoteLab package is free software; you can use it, redistribute it,
# and/or modify it under the terms of the GNU General Public License
# version 3 as published by the Free Software Foundation. The full text
# of the license can be found in the file LICENSE.txt at the top level of
# the package distribution.
#
# Authors:
# 		Alessio Perinelli and Leonardo Ricci
# 		Department of Physics, University of Trento
# 		I-38123 Trento, Italy
# 		alessio.perinelli@unitn.it
# 		leonardo.ricci@unitn.it
# 		https://github.com/LeonardoRicci/RemoteLab
#
# --------------------------------------------------------------------------

pwd = $(notdir $(PWD))
BIN_DIRECTORY = $(HOME)/bin
WORK_LIB = $(PWD)

CC = g++
CFLAGS = -std=c++11 -O3 -Wno-deprecated -Wno-unused-result
LDFLAGS = -lm
LDFLAGS_ALSA = -lasound

WXCFLAGS := `wx-config --cxxflags`
WXLIBFLAGS := `wx-config --libs`

XOSCILLOSCOPE-CONSOLE_SOURCES := xoscilloscope-console_main.cpp xoscilloscope-console_gui.cpp xoscilloscope-console_com.cpp
WAVEX-CONSOLE_SOURCES := wavex-console_main.cpp wavex-console_gui.cpp wavex-console_engine.cpp

all: build

.PHONY: build install uninstall clean
build:
	@echo -n "Creating build folder..."
	@mkdir -p build/
	@cp ./src/* build/
	@echo " done."
	@echo -n "Compiling gnuplot driver..."
	@cd build/; $(CC) $(CFLAGS) -c xoscilloscope-engine_gnuplot.cpp
	@echo " done."
	@echo -n "Compiling and linking oscilloscope engine..."
	@cd build/; $(CC) $(CFLAGS) xoscilloscope-engine_main.cpp xoscilloscope-engine_gnuplot.o -o xoscilloscope-engine $(LDFLAGS) $(LDFLAGS_ALSA)
	@echo " done."
	@echo -n "Compiling and linking oscilloscope console..."
	@cd build/; $(CC) $(CFLAGS) $(XOSCILLOSCOPE-CONSOLE_SOURCES) -o xoscilloscope-console $(CFLAGS) $(WXCFLAGS) $(WXLIBFLAGS)
	@echo " done."
	@echo -n "Compiling and linking waveform generator engine and console..."
	@cd build/; $(CC) $(CFLAGS) $(WAVEX-CONSOLE_SOURCES) -o wavex-generator $(CFLAGS) $(WXCFLAGS) $(WXLIBFLAGS) $(LDFLAGS) $(LDFLAGS_ALSA)
	@echo " done."
	@echo "\nAll executables built successfully."
	@echo "\nUse 'make install' to copy executables in ./installed/ and setup links to ~/bin/."

install:
	@echo -n "Creating install folder (installed/)..."
	@mkdir -p installed/
	@cp build/xoscilloscope-engine installed/;
	@cp build/xoscilloscope-console installed/;
	@cp build/wavex-generator installed/;
	@cp ./scripts/xoscilloscope-launcher installed/;
	@chmod +x ./installed/xoscilloscope-launcher;
	@echo " done."
	@echo -n "Linking binaries into '"$(BIN_DIRECTORY)"'..."
	@ln -sf $(PWD)/installed/xoscilloscope-engine $(BIN_DIRECTORY)/xoscilloscope-engine
	@ln -sf $(PWD)/installed/xoscilloscope-console $(BIN_DIRECTORY)/xoscilloscope-console
	@ln -sf $(PWD)/installed/wavex-generator $(BIN_DIRECTORY)/wavex-generator
	@ln -sf $(PWD)/installed/xoscilloscope-launcher $(BIN_DIRECTORY)/xoscilloscope-launcher
	@echo " done."

uninstall:
	@echo -n "Removing linked binaries from '"$(BIN_DIRECTORY)"'..."
	@rm -f $(BIN_DIRECTORY)/xoscilloscope-engine
	@rm -f $(BIN_DIRECTORY)/xoscilloscope-console
	@rm -f $(BIN_DIRECTORY)/wavex-generator
	@rm -f $(BIN_DIRECTORY)/xoscilloscope-launcher
	@echo " done."
	@echo -n "Removing binaries folder..."
	@rm -rf installed/
	@echo " done."

clean:
	@echo -n "Removing build folder..."
	@rm -rf build/
	@echo " done."
