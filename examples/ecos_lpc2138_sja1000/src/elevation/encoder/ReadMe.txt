               ____     ____     ______    __  ___    ______   ____
              / __ \   / __ \   / ____/   /  |/  /   / ____/  / __ \
             / / / /  / /_/ /  / __/     / /|_/ /   / __/    / / / /
            / /_/ /  / _, _/  / /___    / /  / /   / /___   / /_/ /
            \____/  /_/ |_|  /_____/   /_/  /_/   /_____/   \___\_\

					http://www.oremeq.qc.ca/

This is an absolute rotary encoder gateway to CANopen.
We used for this demo an absolute rotary encoder from Siemens Automation
model: 6FX2001-5SS12  ,  see the 6FX2001-5SS12.jpg for an idea of the look. 

This Encoder work with the SSI31 protocol. The 31 mean 31 bits. And SSI
is a special version of the SPI protocol.

The pinout for this encoder is:
	 1=Clock -
	 2=Clock +
	 3=Data +
	 4=Data -
	 5=n.c.
	 6=n.c.
	 7=n.c.
	 8= ????
	 9=n.c.
	10=n.c.
	11=+10 to 30 VDC @ 250mA
	12=GND

Our LPC2138 implement the SSI31 protocol to pool the encoder
and fill the DS-406 specification for CANopen.

You can find this specification at: http://www.can-cia.org/
Ask for the document: CiA DS 406 V3.1: CANopen device profile for encoder

To build this project, see $CANFESTIVAL_HOME/driver/ecos-lpc2138/ReadMe.txt
to prepare your environemnt.

See the schematic.jpg in this directory.

		Enjoye!
