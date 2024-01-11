# XELab

The **XELab package** is an open-source Linux toolbox made of several intercommunicating programs that provide the following **two tools to carry out basic experiments in electronics**.
* An **oscilloscope** to acquire signals from the microphone input of a computer's sound card.
* A **waveform generator** to generate signals and deliver them to the headphones/speaker output of a computer's sound card.

**XELab** stands for _linu**X** **E**lectronics **Lab**oratory_.

To access the sound card, the programs rely on the **ALSA** Library API ([Advanced Linux Sound Architecture](https://alsa-project.org/wiki/Main_Page)). ALSA is part of the Linux kernel (since version 2.5, 2002) and is therefore available on any recent Linux system. The oscilloscope uses [**gnuplot**](http://www.gnuplot.info/), an open-source plotting program, as data display engine. Graphical user interfaces (GUI) for the two tools are built on the [**wxWidgets** library](https://www.wxwidgets.org/), an open-source cross-platform GUI library.

## License

This package is free software. It is distributed under the terms of the **GNU General Public License (GPL) version 3.0** - see the `LICENSE.txt` file for details. Please notice that, as the license clearly states, the software comes with NO WARRANTY. In no event will any copyright holder, or any other party who modified and/or conveys the program, be liable to you for damages. **Use this software at your own risk**.

## Authors

- Leonardo Ricci (1,2), leonardo.ricci@unitn.it
- Alessio Perinelli (1,3), alessio.perinelli@unitn.it
- Marco Prevedelli (4), marco.prevedelli@unibo.it

(1) Department of Physics, University of Trento, Trento, Italy  
Nonlinear Systems & Electronics Lab - [nse.physics.unitn.it](nse.physics.unitn.it)  

(2) CIMeC, Center for Mind/Brain Sciences, University of Trento, Rovereto, Italy

(3) INFN-TIFPA, University of Trento, Rovereto, Italy

(4) Department of Physics and Astronomy, University of Bologna, Bologna, Italy

## Setup

To setup the package, required libraries have to be installed first, as described in the user manual (`/doc/manual.pdf`). Then, within the package directory, check that requirements are met, compile and install:
```bash
./check
make
make install
```
