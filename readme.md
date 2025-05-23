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

| **AT Command**             | **Description**                              | **Accepted Values**                | **Effect**                                      |
|---------------------------|----------------------------------------------|------------------------------------|-------------------------------------------------|
| `AT+`                     | Basic ping command                           | –                                  | Prints `OK`                                     |
| `AT+VERSION`              | Print firmware version                       | –                                  | Prints `BP_VERSION`                             |
| `AT+SETDEBUG=<0\|1>`       | Enable or disable debug output               | 0 (disable), 1 (enable)            | Toggles debug printing                          |
| `AT+SETRF=<freq>`         | Set LoRa RF frequency                        | 863000000 to 870000000 Hz          | Sets RF frequency and re-applies radio config   |
| `AT+SETTXPWR=<val>`       | Set TX output power                          | 2 to 22 dBm                        | Sets TX power and re-applies radio config       |
| `AT+SETSF=<val>`          | Set LoRa spreading factor                    | 6 to 12                            | Sets SF and re-applies radio config             |
| `AT+SETBW=<val>`          | Set LoRa bandwidth                           | 0 (125kHz), 1 (250kHz), 2 (500kHz) | Sets BW and re-applies radio config             |
| `AT+SETCR=<val>`          | Set LoRa coding rate                         | 1 to 4                             | Sets CR                                         |
| `AT+SETPREAMBLE=<val>`    | Set LoRa preamble length                     | 4 to 64                            | Sets preamble length                            |
| `AT+SETSYMTIMEOUT=<val>`  | Set LoRa symbol timeout                      | 0 to 10000                         | Sets symbol timeout                             |
| `AT+SETFIXLEN=<0\|1>`      | Set fixed payload length mode                | 0 (off), 1 (on)                    | Sets fixed length payload flag                  |
| `AT+SETIQINV=<0\|1>`       | Set IQ inversion                             | 0 (off), 1 (on)                    | Sets IQ inversion                               |
| `AT+SETLBT_RSSI=<val>`    | Set Listen-Before-Talk RSSI threshold        | -120 to 0 dBm                      | Sets LBT RSSI threshold                         |
| `AT+SETLBT_TIME=<val>`    | Set LBT wait time                            | 10 to 5000 ms                      | Sets LBT time                                   |
| `AT+SETLBT_RETRY=<val>`   | Set number of LBT retries                    | 0 to 10                            | Sets LBT retry count                            |
| `AT+SETMODBUSBD=<val>`    | Set Modbus baud rate                         | 1200 to 115200                     | Sets Modbus baudrate and updates Serial         |
| `AT+SETMODBUSDELAY=<val>` | Set Modbus read delay                        | 1 to 1000 ms                       | Sets Modbus read delay                          |
| `AT+BEACON=<0\|1>`         | Enable or disable beacon mode                | 0 (off), 1 (on)                    | Toggles beacon mode                             |
| `AT+STATUS`               | Print current configuration status           | –                                  | Dumps config to serial                          |




## TODOS:
- [] Cleanup wiring diagram
- [] Cleanup/rework PCB:
- [] Add power reset button for mcu
- [] Some sort of LED signalization ?
- [] Maybe use GPIO expander so we have cleaner design
- [] 5VDC power supply?
- [] Implement Modbus slave with read/write register coresponding to I/O pins
- [] Cleanup AT+ command module
