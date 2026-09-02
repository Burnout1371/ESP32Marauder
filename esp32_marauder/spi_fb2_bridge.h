/*
 * SPI FireBeetle 2 Bridge Header
 * Real-time bidirectional SPI communication between ESP32-S3 and FireBeetle 2
 * 
 * S3 acts as SPI Slave on GPIO 19/20/21/47
 * FB2 acts as SPI Master on GPIO 30/29/28/27
 */

#ifndef SPI_FB2_BRIDGE_H
#define SPI_FB2_BRIDGE_H

#include <SPI.h>
#include "driver/spi_slave.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// ===== COMMAND PROTOCOL =====
// Byte 0: Command ID
// Bytes 1-31: Parameters/Data
#define FB2_CMD_PING           0x00
#define FB2_CMD_WIFI_SCAN      0x01
#define FB2_CMD_BLE_SCAN       0x02
#define FB2_CMD_NRF_SCAN       0x03
#define FB2_CMD_WIFI_DEAUTH    0x04
#define FB2_CMD_BLE_SPAM       0x05
#define FB2_CMD_NRF_ATTACK     0x06
#define FB2_CMD_STOP           0x0F
#define FB2_CMD_STATUS         0x10

// Response codes
#define FB2_RESP_ACK           0x00
#define FB2_RESP_BUSY          0x01
#define FB2_RESP_ERROR         0x02
#define FB2_RESP_DATA          0x03
#define FB2_RESP_DONE          0x04

// Buffer size for SPI transactions
#define FB2_SPI_BUFFER_SIZE    32

// SPI Slave pins (S3)
#define FB2_SPI_MOSI           20  // MO from FB2
#define FB2_SPI_MISO           19  // MI to FB2
#define FB2_SPI_SCK            21  // SCK from FB2
#define FB2_SPI_CS             47  // CS from FB2

// ===== GLOBAL STATE =====
typedef struct {
    uint8_t command;
    uint8_t status;
    uint8_t data[FB2_SPI_BUFFER_SIZE];
    uint32_t timestamp;
    bool data_ready;
    bool tx_pending;
} FB2_SPI_State;

volatile FB2_SPI_State fb2_state = {
    .command = 0x00,
    .status = FB2_RESP_ACK,
    .data = {0},
    .timestamp = 0,
    .data_ready = false,
    .tx_pending = false
};

// DMA-aligned buffers (required by ESP32 SPI Slave)
WORD_ALIGNED_ATTR uint8_t fb2_spi_tx_buf[FB2_SPI_BUFFER_SIZE];
WORD_ALIGNED_ATTR uint8_t fb2_spi_rx_buf[FB2_SPI_BUFFER_SIZE];

QueueHandle_t fb2_command_queue = NULL;

// ===== INITIALIZATION =====
void FB2_SPI_Init() {
    Serial.println("[FB2] Initializing SPI Slave bridge...");
    
    spi_bus_config_t buscfg = {
        .mosi_io_num = FB2_SPI_MOSI,
        .miso_io_num = FB2_SPI_MISO,
        .sclk_io_num = FB2_SPI_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = FB2_SPI_BUFFER_SIZE * 8
    };

    spi_slave_interface_config_t slvcfg = {
        .spics_io_num = FB2_SPI_CS,
        .flags = 0,
        .queue_size = 3,
        .mode = 0,  // SPI Mode 0
        .post_setup_cb = NULL,
        .post_trans_cb = NULL
    };

    esp_err_t ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    
    if (ret == ESP_OK) {
        Serial.println("[FB2] SPI Slave initialized successfully on SPI2_HOST");
    } else {
        Serial.printf("[FB2] SPI Slave initialization failed: %d\n", ret);
    }

    // Create command queue for thread-safe communication
    fb2_command_queue = xQueueCreate(10, sizeof(uint8_t));
    
    // Initialize TX buffer
    memset(fb2_spi_tx_buf, 0, FB2_SPI_BUFFER_SIZE);
    fb2_spi_tx_buf[0] = FB2_RESP_ACK;
}

// ===== SPI SLAVE TRANSACTION HANDLER =====
void FB2_SPI_Poll() {
    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));
    
    // Prepare TX buffer with current state
    fb2_spi_tx_buf[0] = fb2_state.status;
    memcpy(&fb2_spi_tx_buf[1], fb2_state.data, FB2_SPI_BUFFER_SIZE - 1);
    
    t.length = FB2_SPI_BUFFER_SIZE * 8;  // 32 bytes = 256 bits
    t.tx_buffer = fb2_spi_tx_buf;
    t.rx_buffer = fb2_spi_rx_buf;
    
    // This blocks until FB2 initiates a transaction
    esp_err_t ret = spi_slave_transmit(SPI2_HOST, &t, 0);  // Non-blocking
    
    if (ret == ESP_OK) {
        // Process received command
        uint8_t cmd = fb2_spi_rx_buf[0];
        
        if (cmd != 0x00) {  // Non-zero = valid command
            fb2_state.command = cmd;
            fb2_state.timestamp = millis();
            fb2_state.data_ready = true;
            
            // Copy command data
            memcpy(fb2_state.data, &fb2_spi_rx_buf[1], FB2_SPI_BUFFER_SIZE - 1);
            
            Serial.printf("[FB2] Command received: 0x%02X at %lu ms\n", cmd, fb2_state.timestamp);
        }
    }
}

// ===== COMMAND HANDLER INTERFACE =====
bool FB2_CommandReady() {
    return fb2_state.data_ready;
}

uint8_t FB2_GetCommand() {
    return fb2_state.command;
}

uint8_t* FB2_GetData() {
    return (uint8_t*)fb2_state.data;
}

void FB2_SetStatus(uint8_t status) {
    fb2_state.status = status;
}

void FB2_SetResponse(uint8_t status, uint8_t* data, size_t len) {
    fb2_state.status = status;
    if (data && len > 0) {
        memcpy(fb2_state.data, data, (len < FB2_SPI_BUFFER_SIZE - 1) ? len : FB2_SPI_BUFFER_SIZE - 1);
    }
    fb2_state.tx_pending = true;
}

void FB2_ClearCommand() {
    fb2_state.command = 0x00;
    fb2_state.data_ready = false;
    memset(fb2_state.data, 0, FB2_SPI_BUFFER_SIZE);
}

void FB2_SendString(const char* str) {
    size_t len = strlen(str);
    if (len > FB2_SPI_BUFFER_SIZE - 1) len = FB2_SPI_BUFFER_SIZE - 1;
    memcpy(fb2_state.data, str, len);
    fb2_state.data[len] = '\0';
    fb2_state.status = FB2_RESP_DATA;
}

// ===== DEBUG PRINT =====
void FB2_PrintState() {
    Serial.printf("[FB2 State] CMD:0x%02X | Status:0x%02X | Ready:%d | TS:%lu\n",
                  fb2_state.command, fb2_state.status, fb2_state.data_ready, fb2_state.timestamp);
}

#endif // SPI_FB2_BRIDGE_H
