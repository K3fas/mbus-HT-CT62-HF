## Main chip
-  HT-CT62-HF
- [Docs](https://docs.heltec.org/en/node/esp32/ht_ct62/index.html)
- SX1262 LoRa trancievers

## Power
- 24 VDC main external power suply
- 5 VDC LDO -> step down to 3.3 VDC LDO
- 3.3 VDC LDO main power rail

## PHY pinout
| Pin # | Type      | Description                  |
|-------|-----------|------------------------------|
| 1     | GND       | Ground |
| 2     | VCC       | +24V DC |
| 3     | DO        | Digital output 5V |
| 4     | GND       | Ground |
| 5     | DI        | Digital input 3.3V |
| 6     | DI        | Digital input 3.3V |
| 7     | DI        | Digital input 3.3V |
| 8     | DI        | Digital input 3.3V |
| 9     | DI        | Digital input 3.3V |
| 10    | RS485:A   | RS485 A |
| 11    | RS485:B   | RS485 B |
| 12    | RS485:GND | RS485 GND |

## Build
- [x] Arduino framework with Heltec library
- ESP IDF does not work with wireless module
- 
## Flash
- Download mode: hold rst button and power cycle board
- Flash with USB->RS485 converter 
- Baud rate 115200

## Settings
- Defines at the beggining of relay.ino file
- TODO: Implement dynamic settings throuh serial using +AT commands
- TODO: Implement send size larger than 255 bytes

## Limitations
- Maximum message size is 255 bytes.
- Sending data is best effort meaning no checks are performed if transmission happened.
- Nodes are as simple transcievers, no mesh setting is implemented.
- Simultanious transmissions are not avoidable cuz LSB is not atomic operation.

## AT Commands
# Available AT Commands

| Command                | Description                             | Parameters                 | Valid Range / Notes             |
|------------------------|---------------------------------------|----------------------------|--------------------------------|
| `AT`                   | Ping device                           | None                      | Responds with `OK`              |
| `AT+VERSION`           | Get firmware version                  | None                      | Example response: `VERSION:1.0.0` |
| `AT+SETDEBUG=<0/1>`    | Enable or disable debug output        | 0 = off, 1 = on            |                                |
| `AT+SETRF=<frequency>` | Set RF frequency (Hz)                 | Integer                   | 863000000 to 870000000          |
| `AT+SETTXPWR=<power>`  | Set transmit power (dBm)              | Integer                   | 2 to 22 dBm                    |
| `AT+SETSF=<sf>`         | Set LoRa spreading factor             | Integer                   | 6 to 12                       |
| `AT+SETBW=<bw>`         | Set LoRa bandwidth index              | Integer                   | 0 to 2 (e.g., 0=125kHz, 1=250kHz, 2=500kHz) |
| `AT+SETMODBUSBD=<baud>` | Set Modbus baud rate                  | Integer                   | 1200 to 115200                 |
| `AT+SETMODBUSDELAY=<ms>`| Set Modbus read delay (milliseconds) | Integer                   | 1 to 1000                     |
| `AT+SETMODBUSBUF=<size>`| Set Modbus buffer size                | Integer                   | 64 to 1024                    |
| `AT+BEACON=<0/1>`       | Disable/Enable periodic beacon mode  | 0 = disable, 1 = enable    |                                |
| `AT+BEACONINT=<ms>`     | Set beacon interval (milliseconds)   | Integer                   | 1000 to 60000 (1 sec to 60 sec) |


