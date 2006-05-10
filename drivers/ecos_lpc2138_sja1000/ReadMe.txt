/*
This file is part of CanFestival, a library implementing CanOpen Stack.
  ____    _    _   _
 / ___|  / \  | \ | | ___  _ __   ___ _ __
| |     / _ \ |  \| |/ _ \| '_ \ / _ \ '_ \
| |___ / ___ \| |\  | (_) | |_) |  __/ | | |
 \____/_/   \_\_| \_|\___/| .__/ \___|_| |_|
                          |_|
          ____                      _
         / ___|__ _ _ __   __ _  __| | __ _
        | |   / _` | '_ \ / _` |/ _` |/ _` |
        | |__| (_| | | | | (_| | (_| | (_| |
         \____\__,_|_| |_|\__,_|\__,_|\__,_|

                   canfestival@canopencanada.ca
/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MARKETING or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


CanFestival driver for the Philips ARM7 model LPC2138 , with a Philips
SJA1000 CAN controler, and running on the eCOS operating system.

Driver Features:
Full CANfestival-3 support including:
DS-150/205: CAN Power Management Layer specification (sleep/wake-up)
DS-201: CAN low level layer
DS-301: CANopen Application layer and Communication Profile (basic CANopen specification)
DS-303-1: LED diagnostic usage (led.c)
DS-305: Layer Setting Service (lss.c) to set the baudrateand node ID in a PnP fashion.
NVRAM: Non Volatil Ram (nvram.c) Full support for internal Philips IAT programming FLASH mode


To use this driver, you need to proceed to do all these steps.
NB: We choose to leave this driver in .o object code to keep the capability
to make on-the-fly redefinition of some parameters for all examples.

Step 1
-----------
Have a developpement workstation. To build everything, we chose to use
an old AMD K6/2-300Mhz running with FreeBSD-5.2-RELEASE freely available
at http://www.freebsd.org/.
To install freebsd, download the two floppy images from
ftp://ftp.freebsd.org/pub/FreeBSD/releases/i386/5.2-RELEASE/floppies
and copy them to a simple floppy disk by typeing:
	dd if=./image.dd of=/dev/fd0
	dd if=./image2.dd of=/dev/fd0
and proceed to boot on them.
You can use Linux or Windows also or any other O/S where you can use GCC.

on FreeBSD, you need to configure the serial port by editing the /etc/rc/serial
config file by adding this entry:

lpc2000 () {
        # Philips LPC2000 serial port configuration for 57600 bauds connection.

        ci=$1; shift
        co=$1; shift

        for i in $*
        do
                # Lock clocal on, hupcl off.
                # XON-XOFF printer
                stty < /dev/ttyi${ci}${i} clocal -hupcl ixon -imaxbel -ixany -ixoff -crtscts 57600
                stty < /dev/ttyl${ci}${i} clocal  hupcl
                stty < /dev/cuai${co}${i} clocal -hupcl ixon -imaxbel -ixany -ixoff -crtscts 57600
                stty < /dev/cual${co}${i} clocal  hupcl
        done
}

###############
lpc2000		d a 0 1  # to configure COM1 and COM2, mean, /dev/cuaa0 and /dev/cuaa1
###############

Step 2
-----------
Find an upload tool to be able to flash the LPC2138.
We found the lpc2isp at the address: 

	http://guest.engelschall.com/~martin/lpc21xx/isp/index.html

 You can use this upload script to call it with proper arguments.

#!/bin/sh
lpc21isp -term -control build/terminal.hex /dev/cuaa1 57600 14746


Step 3
-----------
Proceed to build a GCC cross compiler to be able to produce ARM7TDMI compatible
binary. Go to the port directory of your FreeBSD

	cd /usr/ports/devel/arm-elf-binutils
	gmake install

	cd /usr/ports/devel/arm-elf-gcc295
	gmake install

And add the installation binary PATH to your environment variables. 

	cd /etc
	vi profile

		export PATH=$PATH:/usr/local/armelf/bin


Step 4
-----------
Build an eCOS tree for the LPC2138.
Download eCOS from  http://www.ecoscentric.com  and proceed to compile
the configtool program.  

	cd /usr/ports/devel/ecos-tools/
	gmake install

	untar our special package for eCOS named  eCOS-OLIMEX-p2138.tgz in 
	the eCOS three on your files system.

Start now your configtool for eCOS:
	configtool 

Choose the right target 
For our purpose, we choose to use an OLIMEX P2138 target board.
This target is not in the list, we created it from a derivative of
the OLIMEX P2106. We simply selected an other processor in the pulldown menu
of the configtool. The LPC2138. Adjust some memory capacity, and that's it.

When all you need is properly selected in the configtool, you click on
Save_As and you suggest a name like My2138. 
After, you click on Generate Build Tree.
	cd My2138_build
	gmake depend
	gmake
	gmake install
 And now you have you own eCOS tree for your project in  My2138_install

Step 5
-----------
Prepare the hardware board for this project. We chose to use an
http://www.olimex.com/  board, the P2138.
Any other LPC2138 evaluation could made the job.
On the P2138 board, we have a little area that we can use to solder
the SJA1000 controler.
See can_controler.gif to know how to solder all wires.

Warning: If you want to use different GPIO (General Purpose Input Output)
pin to fit with your own project, 
you can choose a different one, but you need
to be sure to properly define then in the lpc2138_pinout.h

Step 6
-----------
Put the CANfestival-3.tar.gz stuff in My2138_install/src/CANfestival-3
	cd My2138_install/src/CANfestival-3
	./configure target=ecos-lpc2138-sja1000
	gmake
	cd My2138_install/lib
	ln -s My2138_install/src/CANfestival-3/src/libcanfestival.a
	cd My2138_install/src 
	ln -s My2138_install/src/CANfestival-3/driver/ecos-lpc2138-sja1000

Step 7
-----------
Now you are ready to build our demo.
  cd My2138_install/src/CANfestival-3/examples/DS-406Master_ecos
	gmake

  cd My2138_install/src/CANfestival-3/examples/TerminalSlave_ecos
	gmake
Step 8
-----------
To test, upload both .hex file to both targets with lpc21isp and
see the DS-406 absolut rotary encoder transmiting his absolute value to the
terminal.

	For more info about this project, see http://www.oremeq.qc.ca/
Step 9
-----------
For your own project, you can copy all the examples files in your own
directory and modify them.

What is important to understand is:

driver/ecos_lpc2138_sja1000:

	build_baudrate.c is a commande line tool to generate the proper
		timing file for your sja1000 regarding your sja1000 clk.
			see the Makefile for adjustment.

	canOpenDriver.c is the only link between the libcanfestival and
		the hardware.
			f_can_send
			f_can_receive
			interrupts
			nvram_save/load
			baudrate

	eCOS-OLIMEX-p2138.tgz  eCOS package for the OLIMEX p2138 evaluation board
		you have to untar that files in your eCOS three.
		
	sja1000.c containe only function for initialization or 
		configuration of the CAN controler. All this stuff is
		_not_ use by the libcanfestival. You have to call them
		from your main() to enable CAN with your needed configuration.

		hardware init 
			
	lpc2138.c All the basic stuff to run the LPC2138
		iat_flash  user programmable internal flash of the lpc2138

	lpc2138_pinout.h  Is your LPC2138 pinout definition. Modify this file
		if you want to redefine your pinout affectation.

	time_slicer.c  eCOS implementation of the CANfestival scheduler.
		settimer
		alarm

	applicfg.h is your configuration file for the libcanfestival

	lpc2138_pinout.h define all your GPIO to fit macros.


objdictedit:
	Objdictedit  will produce the dictionary.
	Generate your dictionnary. (Or use an already made YourFile.od with
	the tool: objdictgen.py
	The job is to implement all functions define in the YourFIle.c in
		your own project files.

Enjoye!!!!	
		Canopen Canada core team
		canfestival@canopencanada.ca
