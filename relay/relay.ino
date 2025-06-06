#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "settings.h"
#include "command_parser.h"

char txpacket[LORA_BUFFER];
char rxpacket[LORA_BUFFER];

static RadioEvents_t RadioEvents;

typedef enum {
  IDLE,
  STATE_RX,
  STATE_TX
} States_t;

States_t state;
volatile bool rxRecieved = false;
bool dio_triggered = false;
int16_t Rssi, rxSize;
unsigned long bootTime = 0;

void OnTxDone(void);
void OnTxTimeout(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

void printfDebug(const char *fmt, ...) {
  if (!config.print_debug)
    return;
  va_list args;
  va_start(args, fmt);
  Serial.printf(fmt, args);
  va_end(args);
}

void printHex(const char *label, const uint8_t *data, size_t len) {
  Serial.print(label);
  for (size_t i = 0; i < len; i++) {
    Serial.printf("%02X ", data[i]);
  }
  Serial.println();
}

void setup() {
  bootTime = millis();  // store the time at boot

  Serial.setRxBufferSize(512);
  Serial.setTxBufferSize(512);
  Serial.begin(115200, SERIAL_8N1);
  delay(2000);  // Wait for serial to stabilize


  printfDebug("[INIT] Starting LoRa RS485 bridge...\n");
  loadConfig();  // Load LoRa config from NVS
  printfDebug("[INIT] Loaded NVS");
  Serial.updateBaudRate(config.modbus_baudrate);

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;

  int err = Radio.Init(&RadioEvents);
  if (err != 0) {
    Serial.printf("[ERROR] Radio Init failed! Code: %d\n", err);
  }
  Radio.SetTxConfig(MODEM_LORA,
                    config.tx_output_power,
                    0,  // frequency deviation (not used in LoRa)
                    config.lora_bandwidth,
                    config.lora_spreading_factor,
                    config.lora_codingrate,
                    config.lora_preamble_length,
                    config.lora_fix_length_payload_on,
                    true,  // CRC on
                    0,     // Freq hop
                    0,     // Hop period
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
                    0,     // No fixed payload len
                    true,  // CRC on
                    0,     // Freq hop
                    0,     // Hop period
                    config.lora_iq_inversion_on,
                    true);

  Radio.SetChannel(config.rf_frequency);

  state = STATE_TX;

  Serial.println("[INIT] Radio initialized, entering TX mode...");
}

void loop() {
  if (config.beaconEnabled && (millis() - config.lastBeaconMillis >= config.beaconIntervalMs)) {
    config.lastBeaconMillis = millis();
    String beaconMsg = "BEACON: Device alive at " + String(millis()) + " ms\n";

    Serial.print(beaconMsg);
    Radio.Send((uint8_t *)beaconMsg.c_str(), beaconMsg.length());
  }

  // Trigger TX if new RS485 data available
  if (Serial.available()) {
    state = STATE_TX;
  } else if (rxRecieved == true) {
    // Send available data to serial
    Serial.write((uint8_t *)rxpacket, rxSize);
    memset(rxpacket, 0, sizeof(rxpacket));
    rxRecieved = false;
  }

  switch (state) {
    case STATE_TX:
      {
        // Read non terminated data from input serial
        unsigned long lastByteTime = millis();
        size_t len = 0;
        while (millis() - lastByteTime < config.modbus_read_delay && len < LORA_BUFFER - 1) {
          if (Serial.available()) {
            int available = Serial.available();
            int toRead = min(available, (int)(LORA_BUFFER - 1 - len));  // prevent overflow

            int bytesRead = Serial.readBytes(txpacket + len, toRead);
            len += bytesRead;
            lastByteTime = millis();  // reset timer on data reception
          }
        }
        // Sent data or handle at command
        if (len > 0) {
          if (config.print_debug) {
            printHex("[TX] Read serial:", (uint8_t *)txpacket, len);
            printfDebug("MBSUDELAY: %d\n", config.modbus_read_delay);
          }
          // Handle AT command
          if (len >= 3 && strncmp(txpacket, "AT+", 3) == 0) {
            txpacket[len] = '\0';
            String cmd = String(txpacket);
            printfDebug("Recieved AT command %s\n", cmd.c_str());
            cmd.trim();
            handleATCommand(cmd);
            memset(txpacket, 0, sizeof(txpacket));
            state = STATE_RX;
            break;
          }
          // Handle data send
          for (size_t i = 0; i < config.lbt_retry; i++) {
            Radio.Standby();
            if (Radio.IsChannelFree(MODEM_LORA, config.rf_frequency, config.lbt_rssi_threshold, config.lbt_time)) {
              Radio.Send((uint8_t *)txpacket, len);
              printfDebug("[TX] LBT passed, sent packet.\n");

              break;
            }
            Rssi = Radio.Rssi(MODEM_LORA);
            printfDebug("[TX] LBT failed, Rsii: %d, retrying...\n", Rssi);
            delay(50);
          }
        }

        state = IDLE;
        break;
      }

    case STATE_RX:
      printfDebug("[FSM] STATE_RX: enabling LoRa receive...\n");
      Radio.Rx(0);
      state = IDLE;
      break;

    case IDLE:
      Radio.IrqProcess();
      break;

    default:
      break;
  }

  // Trigger TX if new RS485 data available
  if (Serial.available()) {
    state = STATE_TX;
  }
  // Reset the device after 12 hours
  if (millis() - bootTime >= RESET_INTERVAL_MS) {
    Serial.println("[RESET] 12 hours passed. Restarting device...");
    delay(100);  // optional: allow serial message to send
    ESP.restart();
  }
}

void OnTxDone(void) {
  printfDebug("[ISR] TX done.\n");
  memset(txpacket, 0, sizeof(txpacket));
  state = STATE_RX;
}

void OnTxTimeout(void) {
  printfDebug("[ISR] TX timeout.\n");
  state = STATE_RX;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  rxSize = size;
  Rssi = rssi;

  if (config.print_debug) {
    rxpacket[size] = '\0';
    printfDebug("[RX] Received from LoRa: %s\n", rxpacket);
    printfDebug("[RX] RSSI: %d\n", rssi);
    printfDebug("[RX] SNR: %d\n", snr);
    delay(100);
  }

  memcpy(rxpacket, payload, size);
  rxRecieved = true;
  //Serial.write((uint8_t *)rxpacket, size);
  //memset(rxpacket, 0, sizeof(rxpacket));

  state = STATE_TX;
}
