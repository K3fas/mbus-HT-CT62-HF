#pragma once
#include "Arduino.h"
#include "LoRaWan_APP.h"

#define BP_VERSION "V1.0.0"

#define RF_FREQ_MIN 863000000
#define RF_FREQ_MAX 870000000

#define TX_PWR_MIN 2
#define TX_PWR_MAX 22

#define SF_MIN 6
#define SF_MAX 12

#define BW_MIN 0
#define BW_MAX 2

#define CR_MIN 1
#define CR_MAX 4

#define PREAMBLE_MIN 6
#define PREAMBLE_MAX 65535

#define TIMEOUT_MIN 10
#define TIMEOUT_MAX 60000

#define MODBUS_BAUD_MIN 1200
#define MODBUS_BAUD_MAX 115200

#define MODBUS_DELAY_MIN 1
#define MODBUS_DELAY_MAX 1000

typedef struct
{
    uint32_t rf_frequency;
    int8_t tx_output_power;
    uint8_t lora_bandwidth;
    uint8_t lora_spreading_factor;
    uint8_t lora_codingrate;
    uint16_t lora_preamble_length;
    uint16_t lora_symbol_timeout;
    bool lora_fix_length_payload_on;
    bool lora_iq_inversion_on;
    int8_t lbt_rssi_threshold;
    uint16_t lbt_time;
    uint8_t lbt_retry;
    uint32_t rx_timeout_value;

    // Modbus
    uint32_t modbus_baudrate;
    uint16_t modbus_read_delay;
    uint16_t buffer_size;

    // Debug
    bool print_debug;
    bool beaconEnabled;
    unsigned long beaconIntervalMs; // seconds
    unsigned long lastBeaconMillis;
} device_config_t;

device_config_t config = {
    .rf_frequency = 865000000,
    .tx_output_power = 20,
    .lora_bandwidth = 0,
    .lora_spreading_factor = 7,
    .lora_codingrate = 1,
    .lora_preamble_length = 8,
    .lora_symbol_timeout = 0,
    .lora_fix_length_payload_on = false,
    .lora_iq_inversion_on = false,
    .lbt_rssi_threshold = -90,
    .lbt_time = 20,
    .lbt_retry = 5,
    .rx_timeout_value = 1000,

    .modbus_baudrate = 9600,
    .modbus_read_delay = 5,
    .buffer_size = 512,

    .print_debug = false,
    .beaconEnabled = false,
    .beaconIntervalMs = 5000, // 5 seconds
    .lastBeaconMillis = 0,
};

void applyConfigToRadio()
{
    Radio.SetTxConfig(MODEM_LORA,
                      config.tx_output_power,
                      0, // frequency deviation (not used in LoRa)
                      config.lora_bandwidth,
                      config.lora_spreading_factor,
                      config.lora_codingrate,
                      config.lora_preamble_length,
                      config.lora_fix_length_payload_on,
                      true, // CRC on
                      0,
                      config.lora_iq_inversion_on,
                      config.rx_timeout_value,
                      false);
    Radio.SetRxConfig(MODEM_LORA,
                      config.lora_bandwidth,
                      config.lora_spreading_factor,
                      config.lora_codingrate,
                      0,
                      config.lora_preamble_length,
                      config.lora_symbol_timeout,
                      config.lora_fix_length_payload_on,
                      0,
                      true, // CRC on
                      0,
                      0,
                      config.lora_iq_inversion_on,
                      true);

    Radio.SetChannel(config.rf_frequency);
}

void handleATCommand(const String &cmd)
{
    if (cmd == "AT+")
    {
        Serial.println("OK");
    }
    else if (cmd == "AT+VERSION")
    {
        Serial.println("VERSION:1.0.0");
        Serial.println("OK");
    }
    else if (cmd.startsWith("AT+SETDEBUG="))
    {
        config.print_debug = cmd.endsWith("1");
        Serial.println("OK");
    }
    else if (cmd.startsWith("AT+SETRF="))
    {
        uint32_t val = cmd.substring(9).toInt();
        if (val >= RF_FREQ_MIN && val <= RF_FREQ_MAX)
        {
            config.rf_frequency = val;
            applyConfigToRadio();
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERROR: Invalid RF frequency");
        }
    }
    else if (cmd.startsWith("AT+SETTXPWR="))
    {
        int val = cmd.substring(12).toInt();
        if (val >= TX_PWR_MIN && val <= TX_PWR_MAX)
        {
            config.tx_output_power = val;
            applyConfigToRadio();
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERROR: Invalid TX power");
        }
    }
    else if (cmd.startsWith("AT+SETSF="))
    {
        int val = cmd.substring(9).toInt();
        if (val >= SF_MIN && val <= SF_MAX)
        {
            config.lora_spreading_factor = val;
            applyConfigToRadio();
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERROR: Invalid SF");
        }
    }
    else if (cmd.startsWith("AT+SETBW="))
    {
        int val = cmd.substring(9).toInt();
        if (val >= BW_MIN && val <= BW_MAX)
        {
            config.lora_bandwidth = val;
            applyConfigToRadio();
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERROR: Invalid BW");
        }
    }

    else if (cmd.startsWith("AT+SETMODBUSBD="))
    {
        int val = cmd.substring(15).toInt();
        if (val >= MODBUS_BAUD_MIN && val <= MODBUS_BAUD_MAX)
        {
            config.modbus_baudrate = val;
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERROR: Invalid Modbus baud rate");
        }
    }
    else if (cmd.startsWith("AT+SETMODBUSDELAY="))
    {
        int val = cmd.substring(18).toInt();
        if (val >= MODBUS_DELAY_MIN && val <= MODBUS_DELAY_MAX)
        {
            config.modbus_read_delay = val;
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERROR: Invalid Modbus delay");
        }
    }
    else if (cmd.startsWith("AT+BEACON="))
    {
        int val = cmd.substring(10).toInt();
        if (val == 0)
        {
            config.beaconEnabled = false;
            Serial.println("Beacon disabled");
            Serial.println("OK");
        }
        else if (val == 1)
        {
            config.beaconEnabled = true;
            config.lastBeaconMillis = millis(); // reset timer
            Serial.println("Beacon enabled");
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERROR: Invalid beacon param");
        }
    }
    else
    {
        Serial.println("ERROR: Invalid command");
    }
}