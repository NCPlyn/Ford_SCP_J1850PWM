# Ford_SCP_J1850PWM
I have made this to control OEM Ford Focus MK1 instrument cluster with Arduino & ELM327

You need ELM327 which actually does J1850PWM and has desoldarable BT module.

Connections:
- Desolder BT module
- Looking at the pcb and pins, where the BT module antenna was pointed up. 6th pin on right from top is ground, connect to Arduino ground. 1th pin on left from top is ELM UART RX, connect to Arduino TX (for this code pin 11). 2th pin on left from top is ELM UART TX, connect to Arduino RX (for this code pin 10).
- Connect GND (pin 4&5, middle two of the bigger row) and 12v (pin 16) to the OBD port. (Lookup picture of OBD pinout, hard to explain due orientation)
- Connect OBD pin 2 (J1850+) to pin 13 on cluster (looking from back, bottom row, most right pin). Connect OBD pin 10 (J1850-) to pin 26 on cluster (top row, most right).
- Connect GND to pin 2 on cluster (bottom row, second pin from left) and 12V to pins 14&16 (top row, first and third pin from left)

File 'ign-start-drive-dump.txt' has a dump of couple second SCP communication from my 2001 Ford Focus 1.6 FYDA, first 2 bytes are priority, next 2 address of reciever, next 2 address of sender, next bytes are the message and last 2 are checksum.

The .ino file has arduino PoC code where you can set values of temperature, speed and rpm and once ran, the cluster should react.

## Focus SCP message guide
```
priority to from  data(most times first two do not change)
A1 7B 10   02 00             (last 2 by how much, kph/10 at message interval of 50ms)
81 49 10   10 00             (last 2 = temp+40, 16-low|96-middle|a0-max, every sec)
A1 29 10   02 00 00          (last 4 = kph*128, needle is off by little, every 50ms)
81 1B 10   25 00 00 00 00 00 (next 6 after first 2 = rpm*1024, next 2 rate of change??, last 2 = throttle 00-b1, every 50ms)
81 88 10   30 01             (last 2 = CEL, 01 off, 02 on, 03 blink, every sec)
41 73 10   14 20             (last 2 = alternator, 21 on, 20 off, once)
41 88 10   02                (turns off gear light, once, idk how to turn on)

no idea what these do:
81 3B 10   04 40
41 53 10   3B 10 24 12 24 
41 88 10   14 
41 3B 10   3B 81 03 82 
41 73 10   14 21 
81 3B 10   04 40
```
