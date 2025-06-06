#pragma once

#define BP_VERSION "V1.1.1"

#define RESET_INTERVAL_MS (12UL * 60UL * 60UL * 1000UL)



/*
* LoRa Default Settings
*/
#define RF_FREQUENCY 865000000 // Hz
#define TX_OUTPUT_POWER 20     // dBm
#define LORA_BANDWIDTH 0       // 125 kHz
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 16
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define LORA_LBT_RSSI -90 // Listen before talk threshold
#define LORA_LBT_TIME 20  // Listen before talk time in ms
#define LORA_LBT_RETRY 5  // Listen before talk retry count
#define LORA_BUFFER 255 //< DO NOT CHANGE
#define LORA_TX_TIMEOUT 1000
/*
* Modbus/serial default settings
*/
#define MODBUS_BD 9600
#define MODBUS_READ_DELAY 100