/*
 * FireBeetle 2 (DFR1237) Marauder Remote Control UI
 * SPI Master communicating with ESP32-S3 over GPIO 30/29/28/27
 * 
 * FB2 Pins:
 * - GPIO 30: MI (MISO - receive from S3)
 * - GPIO 29: MO (MOSI - send to S3)
 * - GPIO 28: SCK (Clock)
 * - GPIO 27: CS (Chip Select)
 */

#include <SPI.h>
#include <Wire.h>
#include "DFRobot_ST7789_240x320_HW_SPI.h"  // Touchscreen display

// ===== DISPLAY SETUP =====
// Adjust these based on your actual FB2 display pins
#define TFT_DC    9     // Data/Command pin
#define TFT_CS    10    // Chip Select
#define TFT_RST   8     // Reset
#define TFT_BL    11    // Backlight

DFRobot_ST7789_240x320_HW_SPI screen(TFT_DC, TFT_CS, TFT_RST);

// ===== SPI MASTER SETUP =====
#define FB2_SPI_MOSI   29   // MO - Send to S3
#define FB2_SPI_MISO   30   // MI - Receive from S3
#define FB2_SPI_SCK    28   // SCK
#define FB2_SPI_CS     27   // CS

SPIClass spi_master(HSPI);

// ===== COMMAND PROTOCOL (matching S3 definitions) =====
#define FB2_CMD_PING           0x00
#define FB2_CMD_WIFI_SCAN      0x01
#define FB2_CMD_BLE_SCAN       0x02
#define FB2_CMD_NRF_SCAN       0x03
#define FB2_CMD_WIFI_DEAUTH    0x04
#define FB2_CMD_BLE_SPAM       0x05
#define FB2_CMD_NRF_ATTACK     0x06
#define FB2_CMD_STOP           0x0F
#define FB2_CMD_STATUS         0x10

#define FB2_RESP_ACK           0x00
#define FB2_RESP_BUSY          0x01
#define FB2_RESP_ERROR         0x02
#define FB2_RESP_DATA          0x03
#define FB2_RESP_DONE          0x04

// ===== UI STATE =====
typedef struct {
    uint8_t current_screen;
    uint8_t selected_option;
    bool menu_dirty;
    char status_line[32];
    char result_buffer[256];
    uint32_t last_update;
} FB2_UIState;

FB2_UIState ui_state = {
    .current_screen = 0,  // 0 = main menu
    .selected_option = 0,
    .menu_dirty = true,
    .status_line = "Ready",
    .result_buffer = "",
    .last_update = 0
};

// ===== SPI COMMUNICATION =====
uint8_t spi_tx_buf[32];
uint8_t spi_rx_buf[32];

void FB2_SPI_Init() {
    spi_master.begin(FB2_SPI_SCK, FB2_SPI_MISO, FB2_SPI_MOSI, FB2_SPI_CS);
    spi_master.setFrequency(1000000);  // 1 MHz (adjust if needed)
    spi_master.setDataMode(SPI_MODE0);
    
    pinMode(FB2_SPI_CS, OUTPUT);
    digitalWrite(FB2_SPI_CS, HIGH);
    
    Serial.println("[FB2] SPI Master initialized");
}

// Send command to S3, receive response
void FB2_SendCommand(uint8_t cmd, uint8_t* params = NULL) {
    memset(spi_tx_buf, 0, 32);
    spi_tx_buf[0] = cmd;
    
    if (params) {
        memcpy(&spi_tx_buf[1], params, 31);
    }
    
    // Chip select low
    digitalWrite(FB2_SPI_CS, LOW);
    delayMicroseconds(10);
    
    // Transfer 32 bytes
    spi_master.transferBytes(spi_tx_buf, spi_rx_buf, 32);
    
    delayMicroseconds(10);
    // Chip select high
    digitalWrite(FB2_SPI_CS, HIGH);
    
    // Parse response
    uint8_t status = spi_rx_buf[0];
    
    Serial.printf("[FB2-SPI] Sent: 0x%02X | Response: 0x%02X\n", cmd, status);
    Serial.print("[FB2-SPI] Data: ");
    for (int i = 1; i < 16; i++) {
        Serial.printf("%02X ", spi_rx_buf[i]);
    }
    Serial.println();
    
    // Store response in UI state
    memcpy(ui_state.result_buffer, &spi_rx_buf[1], 31);
    ui_state.menu_dirty = true;
}

// ===== DISPLAY FUNCTIONS =====
void FB2_DisplayInit() {
    screen.begin();
    screen.setRotation(0);
    screen.fillScreen(COLOR_BLACK);
    screen.setTextColor(COLOR_WHITE);
    screen.setTextSize(2);
    
    Serial.println("[FB2] Display initialized");
}

void FB2_DrawMainMenu() {
    screen.fillScreen(COLOR_BLACK);
    screen.setCursor(20, 20);
    screen.setTextSize(3);
    screen.print("MARAUDER");
    
    screen.setTextSize(2);
    screen.setCursor(20, 80);
    screen.print("1. WiFi Scan");
    screen.setCursor(20, 110);
    screen.print("2. BLE Scan");
    screen.setCursor(20, 140);
    screen.print("3. NRF Scan");
    screen.setCursor(20, 170);
    screen.print("4. WiFi Attack");
    screen.setCursor(20, 200);
    screen.print("5. BLE Attack");
    screen.setCursor(20, 230);
    screen.print("6. NRF Attack");
    
    screen.setCursor(20, 270);
    screen.setTextSize(1);
    screen.print("Status: ");
    screen.print(ui_state.status_line);
}

void FB2_DrawResults() {
    screen.fillScreen(COLOR_BLACK);
    screen.setCursor(20, 20);
    screen.setTextSize(2);
    screen.print("Results:");
    
    screen.setCursor(20, 60);
    screen.setTextSize(1);
    screen.print(ui_state.result_buffer);
}

void FB2_UpdateDisplay() {
    if (!ui_state.menu_dirty) {
        return;
    }
    
    if (ui_state.current_screen == 0) {
        FB2_DrawMainMenu();
    } else {
        FB2_DrawResults();
    }
    
    ui_state.menu_dirty = false;
}

// ===== MENU NAVIGATION =====
void FB2_HandleMenuSelect(uint8_t option) {
    Serial.printf("[FB2] Menu option selected: %d\n", option);
    
    uint8_t cmd = 0x00;
    
    switch (option) {
        case 1:
            cmd = FB2_CMD_WIFI_SCAN;
            strcpy(ui_state.status_line, "WiFi Scan...");
            break;
        case 2:
            cmd = FB2_CMD_BLE_SCAN;
            strcpy(ui_state.status_line, "BLE Scan...");
            break;
        case 3:
            cmd = FB2_CMD_NRF_SCAN;
            strcpy(ui_state.status_line, "NRF Scan...");
            break;
        case 4:
            cmd = FB2_CMD_WIFI_DEAUTH;
            strcpy(ui_state.status_line, "WiFi Attack...");
            break;
        case 5:
            cmd = FB2_CMD_BLE_SPAM;
            strcpy(ui_state.status_line, "BLE Attack...");
            break;
        case 6:
            cmd = FB2_CMD_NRF_ATTACK;
            strcpy(ui_state.status_line, "NRF Attack...");
            break;
        default:
            return;
    }
    
    if (cmd != 0x00) {
        FB2_SendCommand(cmd);
        ui_state.current_screen = 1;  // Switch to results screen
    }
}

// ===== SETUP =====
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n[FB2] Marauder Remote Control Starting...");
    
    FB2_DisplayInit();
    FB2_SPI_Init();
    
    // Test communication
    Serial.println("[FB2] Testing SPI connection...");
    FB2_SendCommand(FB2_CMD_PING);
    
    delay(1000);
    ui_state.menu_dirty = true;
}

// ===== MAIN LOOP =====
void loop() {
    uint32_t now = millis();
    
    // Update display if needed
    FB2_UpdateDisplay();
    
    // Periodic status check
    if ((now - ui_state.last_update) > 2000) {
        FB2_SendCommand(FB2_CMD_STATUS);
        ui_state.last_update = now;
    }
    
    // TODO: Add touchscreen input handling here
    // For now, simulate menu selections with serial input
    if (Serial.available()) {
        char cmd = Serial.read();
        
        if (cmd >= '1' && cmd <= '6') {
            FB2_HandleMenuSelect(cmd - '0');
        } else if (cmd == 's' || cmd == 'S') {
            FB2_SendCommand(FB2_CMD_STATUS);
        } else if (cmd == 'x' || cmd == 'X') {
            FB2_SendCommand(FB2_CMD_STOP);
            ui_state.current_screen = 0;
            strcpy(ui_state.status_line, "Stopped");
            ui_state.menu_dirty = true;
        } else if (cmd == 'b' || cmd == 'B') {
            ui_state.current_screen = 0;
            ui_state.menu_dirty = true;
        }
    }
    
    delay(100);
}
