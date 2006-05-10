Generator of Canopen SYNC message (cobid = 0x80)
---------------------------------------------
francis.dupin@inrets.fr
27 Janv 2005

Node for Microcontroler MC9S12DP256 
on board  T-board (http://www.elektronikladen.de

=============
Node N° 0x03
=============

This node generate only the SYNC signal.
(cob-id : 0x80)

To test this node
-----------------
Reset it :
It is sending :
cobid : 0x703
data : 00

 1 - Wants to generate SYNC every 10 milliseconds

1a - put the value (4 bytes) : 0x00002710 in its dictionary 
index 0x1006, subindex 0x00 :

CAN message (SDO) : 
cobid : 0x603
data : 23 06 10 00 10 27 00 00
(put 23 to transmit a data of 4 bytes
     27                       3 bytes
     2B                       2 bytes
     2F                       1 byte
)

The node is responding :
cobid : 0x583
data : 60 06 10 00 00 00 00 00

1b - put the value (4 bytes) : 0x40000080 at index 0x1005, subindex 0x00
to start the SYNC : 
cobid : 0x603
data : 23 05 10 00 80 00 00 40

The node is responding :
cobid : 0x583
data : 60 05 10 00 00 00 00 00

 2 - Put the node in operational mode
CAN message (NMT) :
cobid : 0x00
data : 01 03

The node is sending the SYNC every 10 ms


Nota
-----
To stop the SYNC : 2 methods

1 - put 0x00000000 at index 1006 subindex 0
2 - put 0x00000080 at index 1005 subindex 0 





