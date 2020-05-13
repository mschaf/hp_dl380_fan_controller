## HP DL380 Fan Controller

A fan controller for my DL380e gen8, should work with similar servers as well. It takes the fan signal from the mainboard and divides it by 2, expect it is over 70% then it just passed it as it is. 

It gets its power and commands from the internal USB port of the server. It acts like a serial device and can be configured via the serial console. 

Firmware for the PIC microcontroller is written with MPLAB X in c.
The PCB layout is made in KiCAD and can be ordered from services like JLCPCB with the files from the gerbers directory.

Its not perfect, there are still some features it should have, like saving the config to survive a power cylce.

The cables needed can either be hacked together with some spare fan cables from HP or made yourself. The connectors are somewhat custom, or at least I could not find them online. You can use 2mm pitch standart pin sockets and grind them down a litte ti fit in the header on the mainbaord.

![alt text](https://i.imgur.com/F6pru9H.jpg "Fan Controller Installed")
