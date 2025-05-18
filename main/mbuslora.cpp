#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <RadioLib.h>
#include "Esp32c3Hal.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)         \
    ((byte) & 0x80 ? '1' : '0'),     \
        ((byte) & 0x40 ? '1' : '0'), \
        ((byte) & 0x20 ? '1' : '0'), \
        ((byte) & 0x10 ? '1' : '0'), \
        ((byte) & 0x08 ? '1' : '0'), \
        ((byte) & 0x04 ? '1' : '0'), \
        ((byte) & 0x02 ? '1' : '0'), \
        ((byte) & 0x01 ? '1' : '0')

#define SX_PIN_MOSI 7
#define SX_PIN_MISO 6
#define SX_PIN_RST 5
#define SX_PIN_BUSY 4
#define SX_PIN_DIO1 3
#define SX_PIN_CLK 10
#define SX_PIN_NSS 8

constexpr float sx_freq = 868.0f;
constexpr float sx_bw = 125.0f;
constexpr uint8_t sx_sf = 9U;
constexpr uint8_t sx_cr = 7U;
constexpr uint8_t sx_sync = 18U;
constexpr int8_t sx_power = 0;
constexpr uint16_t sx_preamble = 8U;
constexpr uint8_t sx_gain = 0U;

EspHal *hal = new EspHal(SX_PIN_CLK, SX_PIN_MISO, SX_PIN_MOSI);

// now we can create the radio module
// NSS gpio:   8
// BUSY gpio:  4
// NRST gpio:  5
// DIO1 gpio:  3
Module* module = new Module(hal, SX_PIN_NSS, SX_PIN_DIO1, SX_PIN_RST, SX_PIN_BUSY);
SX1262 radio(module);


void readRegisters(uint16_t regAddress, uint8_t *buffer, uint8_t length)
{
    uint8_t tx[3 + length];
    uint8_t rx[4 + length]; // 1 RFU + 3 status + N bytes

    // Command + Address
    tx[0] = 0x1D;
    tx[1] = (regAddress >> 8) & 0xFF;
    tx[2] = regAddress & 0xFF;

    // Fill in NOPs for the data length
    for (uint8_t i = 0; i < length; i++)
    {
        tx[3 + i] = 0x00;
    }

    gpio_set_level((gpio_num_t)SX_PIN_NSS, 0); // Begin SPI transaction
    printf("BUSY PIN: %d \n", gpio_get_level((gpio_num_t)SX_PIN_BUSY));
    printf("RST PIN: %d \n", gpio_get_level((gpio_num_t)SX_PIN_RST));
    hal->spiTransfer(tx, sizeof(tx), rx);
    gpio_set_level((gpio_num_t)SX_PIN_NSS, 1); // End SPI transaction

    // Copy actual data, skipping 4 status bytes
    for (uint8_t i = 0; i < length; i++)
    {
        buffer[i] = rx[4 + i];
    }
}

void sendCommand(uint8_t opcode, uint8_t *buffer, uint8_t length)
{
    uint8_t tx[1 + length];
    uint8_t rx[1 + length];

    // Command + Address
    tx[0] = opcode;

    // Fill in NOPs for the data length
    for (uint8_t i = 0; i < length; i++)
    {
        tx[i + i] = 0x00;
    }

    gpio_set_level((gpio_num_t)SX_PIN_NSS, 0); // Begin SPI transaction
    vTaskDelay(pdMS_TO_TICKS(1));
    hal->spiTransfer(tx, sizeof(tx), rx);
    gpio_set_level((gpio_num_t)SX_PIN_NSS, 1); // End SPI transaction

    // Copy actual data, skipping 4 status bytes
    for (uint8_t i = 0; i < length; i++)
    {
        buffer[i] = rx[i + i];
    }
}

static const char *TAG = "main";

extern "C" void app_main(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SX_PIN_NSS) | (1ULL << SX_PIN_RST),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);

    gpio_set_level((gpio_num_t)SX_PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level((gpio_num_t)SX_PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level((gpio_num_t)SX_PIN_NSS, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    hal->init();
    uint8_t in[2] = {0};
    // Read from Node Address register (0x06CD)
    uint8_t random[4] = {1, 1, 1, 1};
    gpio_set_level((gpio_num_t)SX_PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    readRegisters(0x0819, random, 4);
    printf("Random Number: %02X %02X %02X %02X\n", random[0], random[1], random[2], random[3]);
    uint8_t buff[2];
    sendCommand(0xC0, buff, 2);
    printf("RFU: %02X Status: " BYTE_TO_BINARY_PATTERN "\n", buff[0], BYTE_TO_BINARY(buff[1]));

    // initialize just like with Arduino
    ESP_LOGI(TAG, "[SX1276] Initializing ... ");

    int state = radio.begin(sx_freq,
                            sx_bw,
                            sx_sf,
                            sx_cr,
                            sx_sync,
                            sx_power,
                            sx_preamble,
                            sx_gain);

    if (state != RADIOLIB_ERR_NONE)
    {
        ESP_LOGI(TAG, "failed, code %d\n", state);
        while (true)
        {
            hal->delay(1000);
        }
    }
    ESP_LOGI(TAG, "success!\n");

    // loop forever
    for (;;)
    {
        // send a packet
        ESP_LOGI(TAG, "[SX1276] Transmitting packet ... ");
        state = radio.transmit("Hello World!");
        if (state == RADIOLIB_ERR_NONE)
        {
            // the packet was successfully transmitted
            ESP_LOGI(TAG, "success!");
        }
        else
        {
            ESP_LOGI(TAG, "failed, code %d\n", state);
        }

        // wait for a second before transmitting again
        hal->delay(1000);
    }
}
