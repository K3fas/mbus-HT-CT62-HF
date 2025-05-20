#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "command_parser.h"

#define RF_FREQUENCY 865000000 // Hz
#define TX_OUTPUT_POWER 20     // dBm
#define LORA_BANDWIDTH 0       // 125 kHz
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define LORA_LBT_RSII -90 // Listen before talk threshold
#define LORA_LBT_TIME 20  // Listen before talk time in ms
#define LORA_LBT_RETRY 5  // Listen before talk retry count
#define LORA_BUFFER 255

#define RX_TIMEOUT_VALUE 1000

// Modbus serial
#define MODBUS_BD 9600
#define MODBUS_READ_DELAY 5
#define BUFFER_SIZE 512

// Uncomment this to enable debug output
// #define PRINT_DEBUG

char txpacket[LORA_BUFFER];
char rxpacket[LORA_BUFFER];

static RadioEvents_t RadioEvents;

typedef enum
{
  IDLE,
  STATE_RX,
  STATE_TX
} States_t;

States_t state;
bool sleepMode = false;
bool dio_triggered = false;
int16_t Rssi, rxSize;

void OnTxDone(void);
void OnTxTimeout(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

void printfDebug(const char *fmt, ...)
{
  if (!config.print_debug)
    return;
  va_list args;
  va_start(args, fmt);
  Serial.printf(fmt, args);
  va_end(args);
}

void setup()
{
#ifdef PRINT_DEBUG
  Serial.begin(115200); // Shared RS485 and debug line
#else
  Serial.begin(MODBUS_BD); // RS485 line only
#endif
  delay(2000); // Wait for serial to stabilize

  printfDebug("[INIT] Starting LoRa RS485 bridge...");

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;

  int err = Radio.Init(&RadioEvents);
  if (err != 0)
  {
    Serial.printf("[ERROR] Radio Init failed! Code: %d\n", err);
  }
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  state = STATE_TX;

  Serial.println("[INIT] Radio initialized, entering TX mode...");
}

void loop()
{

  if (config.beaconEnabled && (millis() - config.lastBeaconMillis >= config.beaconIntervalMs))
  {
    config.lastBeaconMillis = millis();
    String beaconMsg = "BEACON: Device alive at " + String(millis()) + " ms\n";

    Serial.print(beaconMsg);
    Radio.Send((uint8_t *)beaconMsg.c_str(), beaconMsg.length());
  }

  switch (state)
  {
  case STATE_TX:
  {
    printfDebug("[FSM] STATE_TX: checking RS485 data...");

    // Read non terminated data from input serial
    unsigned long lastByteTime = millis();
    size_t len = 0;
    while (millis() - lastByteTime < MODBUS_READ_DELAY && len < LORA_BUFFER - 1)
    {
      if (Serial.available())
      {
        txpacket[len++] = Serial.read();
        lastByteTime = millis();
      }
    }
    // Sent data or handle at command
    if (len > 0)
    {
      // Handle AT command
      if (len >= 3 && strncmp(txpacket, "AT+", 3) == 0)
      {
        String cmd = String(txpacket);
        cmd.trim();
        handleATCommand(cmd);
        memset(txpacket, 0, sizeof(txpacket));
        break;
      }
      // Handle data send
      for (size_t i = 0; i < LORA_LBT_RETRY; i++)
      {
        Radio.Standby();
        if (Radio.IsChannelFree(MODEM_LORA, RF_FREQUENCY, LORA_LBT_RSII, LORA_LBT_TIME))
        {
          Radio.Send((uint8_t *)txpacket, len);
          printfDebug("[TX] LBT passed, sent packet.");

          break;
        }
        printfDebug("[TX] LBT failed, retrying...");
        delay(100);
      }
    }

    printfDebug("[TX] RS485 over LoRa: ");
    printfDebug("%s\n", txpacket);
    state = IDLE;
    break;
  }

  case STATE_RX:
    printfDebug("[FSM] STATE_RX: enabling LoRa receive...");
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
  if (Serial.available())
  {
    state = STATE_TX;
  }
}

void OnTxDone(void)
{
  printfDebug("[ISR] TX done.");
  memset(txpacket, 0, sizeof(txpacket));
  state = STATE_RX;
}

void OnTxTimeout(void)
{
  printfDebug("[ISR] TX timeout.");
  state = STATE_RX;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  memcpy(rxpacket, payload, size);
  rxpacket[size] = '\0';
  Rssi = rssi;
  rxSize = size;

  printfDebug("[RX] Received from LoRa: %s", rxpacket);
  printfDebug("[RX] RSSI: %d", Rssi);

  // Send to RS485 (Serial shared)
  Serial.write((uint8_t *)rxpacket, rxSize);
  memset(rxpacket, 0, sizeof(rxpacket));

  state = STATE_RX;
}
