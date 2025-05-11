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
- ESP-IDF v5.4

## Flash
- Download mode: hold rst button and power cycle board
- Flash with USB->RS485 converter 
- Baud rate 115200


## RF nastaveni
