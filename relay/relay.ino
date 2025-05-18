#include "LoRaWan_APP.h"
#include "Arduino.h"

#define RF_FREQUENCY 865000000 // Hz
#define TX_OUTPUT_POWER 5      // dBm
#define LORA_BANDWIDTH 0       // 125 kHz
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE 1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define LORA_LBT_RSII -90  // Listen before talk threshold
#define LORA_LBT_TIME 100 // Listen before talk time in ms
#define LORA_LBT_RETRY 5   // Listen before talk retry count
#define LORA_RESET 5
#define LORA_CSS 8

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 512

// Modbus serial
#define MODBUS_BD 9600
#define MODBUS_READ_DELAY 5

// Uncomment this to enable debug output
// #define PRINT_DEBUG

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

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

void setup()
{
#ifdef PRINT_DEBUG
  Serial.begin(115200); // Shared RS485 and debug line
#else
  Serial.begin(MODBUS_BD); // RS485 line only
#endif
  delay(2000); // Wait for serial to stabilize

#ifdef PRINT_DEBUG
  Serial.println("[INIT] Starting LoRa RS485 bridge...");
#endif

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;

  int err = Radio.Init(&RadioEvents);
  if (err != 0)
  {
#ifdef PRINT_DEBUG
    Serial.printf("[ERROR] Radio Init failed! Code: %d\n", err);
#endif
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
  switch (state)
  {
  case STATE_TX:
  {
#ifdef PRINT_DEBUG
    Serial.println("[FSM] STATE_TX: checking RS485 data...");
#endif
    // Read non terminated data from input serial
    unsigned long lastByteTime = millis();
    size_t len = 0;
    while (millis() - lastByteTime < MODBUS_READ_DELAY && len < BUFFER_SIZE - 1)
    {
      if (Serial.available())
      {
        txpacket[len++] = Serial.read();
        lastByteTime = millis();
      }
    }
    // Sent data
    if (len > 0)
    {
      for (size_t i = 0; i < LORA_LBT_RETRY; i++)
      {
        Radio.Standby();
        if (Radio.IsChannelFree(MODEM_LORA, RF_FREQUENCY, LORA_LBT_RSII, LORA_LBT_TIME))
        {
          Radio.Send((uint8_t *)txpacket, len);
#ifdef PRINT_DEBUG
          Serial.println("[TX] LBT passed, sent packet.");
#endif
          break;
        }
#ifdef PRINT_DEBUG
        Serial.println("[TX] LBT failed, retrying...");
#endif
        delay(100);
      }
    }

#ifdef PRINT_DEBUG
    Serial.print("[TX] RS485 over LoRa: ");
    Serial.println(txpacket);
#endif
    state = IDLE;
    break;
  }

  case STATE_RX:
#ifdef PRINT_DEBUG
    Serial.println("[FSM] STATE_RX: enabling LoRa receive...");
#endif
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
#ifdef PRINT_DEBUG
  Serial.println("[ISR] TX done.");
#endif
  memset(txpacket, 0, sizeof(txpacket));
  state = STATE_RX;
}

void OnTxTimeout(void)
{
#ifdef PRINT_DEBUG
  Serial.println("[ISR] TX timeout.");
#endif
  state = STATE_RX;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  memcpy(rxpacket, payload, size);
  rxpacket[size] = '\0';
  Rssi = rssi;
  rxSize = size;

#ifdef PRINT_DEBUG
  Serial.print("[RX] Received from LoRa: ");
  Serial.println(rxpacket);
  Serial.print("[RX] RSSI: ");
  Serial.println(Rssi);
#endif

  // Send to RS485 (Serial shared)
  Serial.write((uint8_t *)rxpacket, rxSize);
  memset(rxpacket, 0, sizeof(rxpacket));

  state = STATE_RX;
}
