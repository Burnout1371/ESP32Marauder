/*
 * ESP32-S3 Marauder with FireBeetle 2 SPI Slave Integration
 * Modified main sketch with FB2 bridge support
 * 
 * Hardware:
 * - NRF24L01+: SCK=12, MISO=13, MOSI=11, CSN=10, CE=9
 * - FB2 SPI Slave: MOSI=20, MISO=19, SCK=21, CS=47
 */

// ===== CRITICAL: Add these includes BEFORE other Marauder includes =====
#include "spi_fb2_bridge.h"
#include "fb2_command_handlers.h"

// ===== Standard Marauder includes =====
#include "configs.h"

#ifndef HAS_SCREEN
  #define MenuFunctions_h
  #define Display_h
#endif

#include <stdio.h>

#ifdef HAS_GPS
  #include "GpsInterface.h"
#endif

#include "Assets.h"
#include "WiFiScan.h"

#ifdef HAS_SD
  #include "SDInterface.h"
#endif

#include "Buffer.h"
#include "settings.h"
#include "CommandLine.h"
#include "lang_var.h"

// ===== Global objects =====
WiFiScan wifi_scan_obj;
EvilPortal evil_portal_obj;
Buffer buffer_obj;
Settings settings_obj;
CommandLine cli_obj;

const String PROGMEM version_number = MARAUDER_VERSION;

uint32_t currentTime = 0;

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n\n");
  Serial.println("╔═══════════════════════════════════════════════════════════╗");
  Serial.println("║       ESP32-S3 Marauder with FireBeetle 2 Bridge          ║");
  Serial.println("║                   Initializing...                         ║");
  Serial.println("╚═══════════════════════════════════════════════════════════╝");
  
  // 1. Initialize standard Marauder subsystems
  Serial.println("\n[INIT] Starting standard Marauder initialization...");
  
  // Configure WiFi/BLE/General
  #ifdef HAS_BT
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
  #endif
  
  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  
  Serial.println("[INIT] Standard subsystems initialized");
  
  // 2. Initialize NRF24L01+ on FSPI
  Serial.println("\n[INIT] Initializing NRF24L01+ on FSPI...");
  Serial.println("  - CE: GPIO 9");
  Serial.println("  - CSN: GPIO 10");
  Serial.println("  - SCK: GPIO 12");
  Serial.println("  - MISO: GPIO 13");
  Serial.println("  - MOSI: GPIO 11");
  
  SPI.begin(12, 13, 11);  // SCK, MISO, MOSI
  
  if (wifi_scan_obj.begin()) {
    Serial.println("[INIT] ✓ WiFi scan initialized");
  } else {
    Serial.println("[INIT] ✗ WiFi scan initialization failed");
  }
  
  // 3. Initialize FB2 SPI Slave Bridge
  Serial.println("\n[INIT] Initializing FireBeetle 2 SPI Slave Bridge...");
  Serial.println("  - MOSI: GPIO 20 (MO from FB2)");
  Serial.println("  - MISO: GPIO 19 (MI to FB2)");
  Serial.println("  - SCK: GPIO 21");
  Serial.println("  - CS: GPIO 47");
  
  FB2_SPI_Init();
  
  Serial.println("\n╔═══════════════════════════════════════════════════════════╗");
  Serial.println("║              ✓ Initialization Complete                    ║");
  Serial.println("║         Waiting for FireBeetle 2 commands...              ║");
  Serial.println("╚═══════════════════════════════════════════════════════════╝\n");
  
  delay(1000);
}

// ===== MAIN LOOP =====
void loop() {
  currentTime = millis();
  
  // ===== Primary: Process FB2 Commands =====
  // Poll SPI and handle incoming commands from FireBeetle 2
  FB2_Update();
  
  // ===== Secondary: Standard Marauder processing =====
  // This allows Marauder to continue its normal operations
  // while FB2 commands are being processed
  
  // Update CLI if available
  if (Serial.available()) {
    cli_obj.serialEvent();
  }
  
  // Small delay to prevent watchdog timeout
  delay(10);
}

// ===== DEBUG: Print FB2 Bridge Status =====
void PrintFB2Status() {
  Serial.println("\n===== FireBeetle 2 Bridge Status =====");
  FB2_PrintState();
  Serial.printf("Handler State - Scanning: %d | Cmd: 0x%02X\n", 
                handler_state.scanning, handler_state.current_cmd);
  Serial.println("=====================================\n");
}
