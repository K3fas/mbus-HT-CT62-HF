#pragma once
#include "Arduino.h"
#include "LoRaWan_APP.h"
#include "settings.h"
#include <Preferences.h>

/*
 * Input value bounds checking
 */
#define RF_FREQ_MIN 863000000
#define RF_FREQ_MAX 870000000

#define TX_PWR_MIN 2
#define TX_PWR_MAX 22

#define SF_MIN 6
#define SF_MAX 12

#define BW_MIN 0
#define BW_MAX 2

#define LORA_CODINGRATE_MIN 1
#define LORA_CODINGRATE_MAX 4

#define LORA_PREAMBLE_MIN 4
#define LORA_PREAMBLE_MAX 64

#define LORA_SYMTIMEOUT_MIN 0
#define LORA_SYMTIMEOUT_MAX 10000

#define LORA_BOOL_MIN 0
#define LORA_BOOL_MAX 1

#define PREAMBLE_MIN 6
#define PREAMBLE_MAX 64

#define TIMEOUT_MIN 10
#define TIMEOUT_MAX 60000

#define LBT_RSSI_MIN -120
#define LBT_RSSI_MAX 0

#define LBT_TIME_MIN 10   // ms
#define LBT_TIME_MAX 5000 // ms

#define LBT_RETRY_MIN 0
#define LBT_RETRY_MAX 10

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
    uint32_t tx_timeout;

    // Modbus
    uint32_t modbus_baudrate;
    uint16_t modbus_read_delay;
    uint16_t buffer_size;

    // Debug
    bool print_debug;
    bool beaconEnabled;
    unsigned long beaconIntervalMs;
    unsigned long lastBeaconMillis;
} device_config_t;

device_config_t config = {
    .rf_frequency = RF_FREQUENCY,
    .tx_output_power = TX_OUTPUT_POWER,
    .lora_bandwidth = LORA_BANDWIDTH,
    .lora_spreading_factor = LORA_SPREADING_FACTOR,
    .lora_codingrate = LORA_CODINGRATE,
    .lora_preamble_length = LORA_PREAMBLE_LENGTH,
    .lora_symbol_timeout = LORA_SYMBOL_TIMEOUT,
    .lora_fix_length_payload_on = LORA_FIX_LENGTH_PAYLOAD_ON,
    .lora_iq_inversion_on = LORA_IQ_INVERSION_ON,
    .lbt_rssi_threshold = LORA_LBT_RSSI,
    .lbt_time = LORA_LBT_TIME,
    .lbt_retry = LORA_LBT_RETRY,
    .tx_timeout = LORA_TX_TIMEOUT,

    .modbus_baudrate = MODBUS_BD,
    .modbus_read_delay = MODBUS_READ_DELAY,
    .buffer_size = 512,

    .print_debug = false,
    .beaconEnabled = false,
    .beaconIntervalMs = BEACON_INTERVAL_MS,
    .lastBeaconMillis = 0,
};

Preferences prefs;

void loadConfig()
{
    prefs.begin("device", false);

    config.rf_frequency = prefs.getULong("rf_freq", RF_FREQUENCY);
    config.tx_output_power = prefs.getChar("tx_pwr", TX_OUTPUT_POWER);
    config.lora_bandwidth = prefs.getUChar("l_bw", LORA_BANDWIDTH);
    config.lora_spreading_factor = prefs.getUChar("l_sf", LORA_SPREADING_FACTOR);
    config.lora_codingrate = prefs.getUChar("l_cr", LORA_CODINGRATE);
    config.lora_preamble_length = prefs.getUShort("l_pre", LORA_PREAMBLE_LENGTH);
    config.lora_symbol_timeout = prefs.getUShort("l_symto", LORA_SYMBOL_TIMEOUT);
    config.lora_fix_length_payload_on = prefs.getBool("l_fixlen", LORA_FIX_LENGTH_PAYLOAD_ON);
    config.lora_iq_inversion_on = prefs.getBool("l_iqinv", LORA_IQ_INVERSION_ON);
    config.lbt_rssi_threshold = prefs.getChar("l_lbt_rssi", LORA_LBT_RSSI);
    config.lbt_time = prefs.getUShort("l_lbt_time", LORA_LBT_TIME);
    config.lbt_retry = prefs.getUChar("l_lbt_rtry", LORA_LBT_RETRY);
    config.tx_timeout = prefs.getULong("tx_timeout", LORA_TX_TIMEOUT);

    config.modbus_baudrate = prefs.getULong("mb_bd", MODBUS_BD);
    config.modbus_read_delay = prefs.getUShort("mb_delay", MODBUS_READ_DELAY);
    config.buffer_size = prefs.getUShort("mb_buf", 512);

    config.print_debug = prefs.getBool("debug", false);
    config.beaconEnabled = prefs.getBool("beacon", false);
    config.beaconIntervalMs = prefs.getULong("beacon_int", BEACON_INTERVAL_MS);
    config.lastBeaconMillis = 0; // Always reset on boot

    prefs.end();
}

void saveConfig()
{
    prefs.begin("device", false); // Read/Write

    prefs.putULong("rf_freq", config.rf_frequency);
    prefs.putChar("tx_pwr", config.tx_output_power);
    prefs.putUChar("l_bw", config.lora_bandwidth);
    prefs.putUChar("l_sf", config.lora_spreading_factor);
    prefs.putUChar("l_cr", config.lora_codingrate);
    prefs.putUShort("l_pre", config.lora_preamble_length);
    prefs.putUShort("l_symto", config.lora_symbol_timeout);
    prefs.putBool("l_fixlen", config.lora_fix_length_payload_on);
    prefs.putBool("l_iqinv", config.lora_iq_inversion_on);
    prefs.putChar("l_lbt_rssi", config.lbt_rssi_threshold);
    prefs.putUShort("l_lbt_time", config.lbt_time);
    prefs.putUChar("l_lbt_rtry", config.lbt_retry);
    prefs.putULong("tx_timeout", config.tx_timeout);

    prefs.putULong("mb_bd", config.modbus_baudrate);
    prefs.putUShort("mb_delay", config.modbus_read_delay);
    prefs.putUShort("mb_buf", config.buffer_size);
    // Do not store debug mode
    prefs.putBool("beacon", config.beaconEnabled);
    prefs.putULong("beacon_int", config.beaconIntervalMs);

    prefs.end();
}

void applyConfigToRadio()
{
    Radio.Standby();
    Radio.SetTxConfig(MODEM_LORA,
                      config.tx_output_power,
                      0, // frequency deviation (not used in LoRa)
                      config.lora_bandwidth,
                      config.lora_spreading_factor,
                      config.lora_codingrate,
                      config.lora_preamble_length,
                      config.lora_fix_length_payload_on,
                      true, // CRC on
                      0,    // Freq hop
                      0,    // Hop period
                      config.lora_iq_inversion_on,
                      config.tx_timeout);
    Radio.SetRxConfig(MODEM_LORA,
                      config.lora_bandwidth,
                      config.lora_spreading_factor,
                      config.lora_codingrate,
                      0,
                      config.lora_preamble_length,
                      config.lora_symbol_timeout,
                      config.lora_fix_length_payload_on,
                      0,    // No fixed payload len
                      true, // CRC on
                      0,    // Freq hop
                      0,    // Hop period
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
        Serial.println(BP_VERSION);
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
    else if (cmd.startsWith("AT+SETCR="))
    {
        int value = cmd.substring(strlen("AT+SETCR=")).toInt();
        if (value >= LORA_CODINGRATE_MIN && value <= LORA_CODINGRATE_MAX)
        {
            config.lora_codingrate = value;
            Serial.println("OK");
        }
        else
        {
            Serial.printf("ERR: Coding rate must be between %d and %d\n", LORA_CODINGRATE_MIN, LORA_CODINGRATE_MAX);
        }
    }

    else if (cmd.startsWith("AT+SETPREAMBLE="))
    {
        int value = cmd.substring(strlen("AT+SETPREAMBLE=")).toInt();
        if (value >= LORA_PREAMBLE_MIN && value <= LORA_PREAMBLE_MAX)
        {
            config.lora_preamble_length = value;
            Serial.println("OK");
        }
        else
        {
            Serial.printf("ERR: Preamble must be between %d and %d\n", LORA_PREAMBLE_MIN, LORA_PREAMBLE_MAX);
        }
    }

    else if (cmd.startsWith("AT+SETSYMTIMEOUT="))
    {
        int value = cmd.substring(strlen("AT+SETSYMTIMEOUT=")).toInt();
        if (value >= LORA_SYMTIMEOUT_MIN && value <= LORA_SYMTIMEOUT_MAX)
        {
            config.lora_symbol_timeout = value;
            Serial.println("OK");
        }
        else
        {
            Serial.printf("ERR: Symbol timeout must be between %d and %d\n", LORA_SYMTIMEOUT_MIN, LORA_SYMTIMEOUT_MAX);
        }
    }

    else if (cmd.startsWith("AT+SETFIXLEN="))
    {
        int value = cmd.substring(strlen("AT+SETFIXLEN=")).toInt();
        if (value >= LORA_BOOL_MIN && value <= LORA_BOOL_MAX)
        {
            config.lora_fix_length_payload_on = value;
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERR: FixLen must be 0 or 1");
        }
    }

    else if (cmd.startsWith("AT+SETIQINV="))
    {
        int value = cmd.substring(strlen("AT+SETIQINV=")).toInt();
        if (value >= LORA_BOOL_MIN && value <= LORA_BOOL_MAX)
        {
            config.lora_iq_inversion_on = value;
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERR: IQ Inversion must be 0 or 1");
        }
    }
    else if (cmd.startsWith("AT+SETLBT_RSSI="))
    {
        int value = cmd.substring(strlen("AT+SETLBT_RSSI=")).toInt();
        if (value >= LBT_RSSI_MIN && value <= LBT_RSSI_MAX)
        {
            config.lbt_rssi_threshold = value;
            Serial.println("OK");
        }
        else
        {
            Serial.printf("ERR: RSSI must be between %d and %d\n", LBT_RSSI_MIN, LBT_RSSI_MAX);
        }
    }

    else if (cmd.startsWith("AT+SETLBT_TIME="))
    {
        int value = cmd.substring(strlen("AT+SETLBT_TIME=")).toInt();
        if (value >= LBT_TIME_MIN && value <= LBT_TIME_MAX)
        {
            config.lbt_time = value;
            Serial.println("OK");
        }
        else
        {
            Serial.printf("ERR: Time must be between %d and %d ms\n", LBT_TIME_MIN, LBT_TIME_MAX);
        }
    }

    else if (cmd.startsWith("AT+SETLBT_RETRY="))
    {
        int value = cmd.substring(strlen("AT+SETLBT_RETRY=")).toInt();
        if (value >= LBT_RETRY_MIN && value <= LBT_RETRY_MAX)
        {
            config.lbt_retry = value;
            Serial.println("OK");
        }
        else
        {
            Serial.printf("ERR: Retry must be between %d and %d\n", LBT_RETRY_MIN, LBT_RETRY_MAX);
        }
    }
    else if (cmd.startsWith("AT+SETMODBUSBD="))
    {
        int val = cmd.substring(15).toInt();
        if (val >= MODBUS_BAUD_MIN && val <= MODBUS_BAUD_MAX)
        {
            config.modbus_baudrate = val;
            Serial.updateBaudRate(config.modbus_baudrate);
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
    else if (cmd.startsWith("AT+SETBEACONINT="))
    {
        unsigned long value = cmd.substring(strlen("AT+SETBEACONINT=")).toInt();
        if (value >= 1000 && value <= 24*60*60*1000)
        { // 1s to 24h
            config.beaconIntervalMs = value;
            Serial.println("OK");
        }
        else
        {
            Serial.println("ERR: Interval must be 1000 to 60000 ms");
        }
    }

    else if (cmd == "AT+STATUS")
    {
        Serial.println("=== Device Configuration ===");
        Serial.printf("RF Frequency:           %lu Hz\n", config.rf_frequency);
        Serial.printf("TX Output Power:        %d dBm\n", config.tx_output_power);
        Serial.printf("LoRa Bandwidth:         %u\n", config.lora_bandwidth);
        Serial.printf("LoRa Spreading Factor:  %u\n", config.lora_spreading_factor);
        Serial.printf("LoRa Coding Rate:       %u\n", config.lora_codingrate);
        Serial.printf("LoRa Preamble Length:   %u\n", config.lora_preamble_length);
        Serial.printf("LoRa Symbol Timeout:    %u\n", config.lora_symbol_timeout);
        Serial.printf("Fix Length Payload:     %s\n", config.lora_fix_length_payload_on ? "ON" : "OFF");
        Serial.printf("IQ Inversion:           %s\n", config.lora_iq_inversion_on ? "ON" : "OFF");

        Serial.printf("LBT RSSI Threshold:     %d dBm\n", config.lbt_rssi_threshold);
        Serial.printf("LBT Time:               %u ms\n", config.lbt_time);
        Serial.printf("LBT Retry:              %u\n", config.lbt_retry);
        Serial.printf("TX Timeout:             %lu ms\n", config.tx_timeout);

        Serial.printf("Modbus Baudrate:        %lu\n", config.modbus_baudrate);
        Serial.printf("Modbus Read Delay:      %u ms\n", config.modbus_read_delay);
        Serial.printf("Modbus Buffer Size:     %u bytes\n", config.buffer_size);

        Serial.printf("Debug Output:           %s\n", config.print_debug ? "ENABLED" : "DISABLED");
        Serial.printf("Beacon Mode:            %s\n", config.beaconEnabled ? "ENABLED" : "DISABLED");
        Serial.printf("Beacon Interval:        %lu ms\n", config.beaconIntervalMs);
        Serial.println("=============================");
    }
    else if (cmd == "AT+SAVE")
    {
        saveConfig();
        Serial.println("OK");
    }
    else if (cmd == "AT+HELP")
    {
        Serial.println("Available AT Commands:");
        Serial.println("AT+");
        Serial.println("AT+VERSION");
        Serial.println("AT+SETDEBUG=<0|1>");
        Serial.println("AT+SETRF=<freq Hz>");
        Serial.println("AT+SETTXPWR=<2-22>");
        Serial.println("AT+SETSF=<6-12>");
        Serial.println("AT+SETBW=<0-2>");
        Serial.println("AT+SETCR=<1-4>");
        Serial.println("AT+SETPREAMBLE=<4-64>");
        Serial.println("AT+SETSYMTIMEOUT=<0-10000>");
        Serial.println("AT+SETFIXLEN=<0|1>");
        Serial.println("AT+SETIQINV=<0|1>");
        Serial.println("AT+SETLBT_RSSI=<-120 to 0>");
        Serial.println("AT+SETLBT_TIME=<10-5000>");
        Serial.println("AT+SETLBT_RETRY=<0-10>");
        Serial.println("AT+SETMODBUSBD=<baud>");
        Serial.println("AT+SETMODBUSDELAY=<ms>");
        Serial.println("AT+SETBUFSIZE=<bytes>");
        Serial.println("AT+SETTIMEOUT=<ms>");
        Serial.println("AT+BEACON=<0|1>");
        Serial.println("AT+SETBEACONINT=<ms>");
        Serial.println("AT+SAVE");
        Serial.println("AT+STATUS");
        Serial.println("OK");
    }
    else
    {
        Serial.println("ERROR: Invalid command");
    }
}