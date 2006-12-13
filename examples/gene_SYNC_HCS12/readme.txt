Generator of Canopen SYNC message (cobid = 0x80)
---------------------------------------------
francis.dupin@inrets.fr
27 Janv 2005. 
Revised 13 Dec 2006


Status of the code :
====================
Tested with 
- CanFestival 3 rc2 (cvs version)
- gcc port for HC12 release 3.1
- Microcontroler MC9S12DP256 
on board  T-board (http://www.elektronikladen.de)



To build the example
--------------------
a) Compile CanFestival for hcs12 :
  Go to the root of CanFestival and enter
  ./configure --target=hcs12
  make clean all

b) Build the example
--------------------
 cd examples/gene_SYNC_HCS12

 make clean all


What does the node ?
====================
Just reset it, it should send the SYNC (cobId 0x80) every 10 ms
Informations availables if you connect the serial port 0 to a terminal configured at 38400 8N1
				   




The default values :
nodeId = 0x03
CAN rate = 250 kbps
Please read appli.c, these values can be modified by switch.


If you put the node in operational state, the CAN messages received are filtered : Only the NMT and Nodeguard can be received.
The parameters of the filter are mapped in the object dictionary, so that the filter can be configured by SDO before entering in operational state. See the object dictionary index 2015 to 2023. To have the values applied, always download at 0x2023 index 0 the value 1 before entering in operational.

Read the file objdict.c to see the capabilities of the node. 









