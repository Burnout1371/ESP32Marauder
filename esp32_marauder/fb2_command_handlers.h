/*
 * FB2 Command Handlers
 * Maps SPI commands from FireBeetle 2 to Marauder scan functions
 */

#ifndef FB2_COMMAND_HANDLERS_H
#define FB2_COMMAND_HANDLERS_H

#include "spi_fb2_bridge.h"

// Forward declarations (these come from Marauder)
extern WiFiScan wifi_scan_obj;
extern Buffer buffer_obj;

// ===== HANDLER STATE =====
typedef struct {
    uint8_t current_cmd;
    bool scanning;
    uint32_t scan_start_time;
    uint32_t last_result_sent;
} FB2_HandlerState;

volatile FB2_HandlerState handler_state = {
    .current_cmd = 0x00,
    .scanning = false,
    .scan_start_time = 0,
    .last_result_sent = 0
};

// ===== WIFI SCAN HANDLER =====
void FB2_Handle_WiFiScan() {
    if (!handler_state.scanning) {
        Serial.println("[FB2-WiFi] Starting WiFi scan...");
        
        // Clear previous results
        buffer_obj.clearBuffer();
        
        // Start WiFi scan (non-blocking)
        wifi_scan_obj.startScan();
        handler_state.scanning = true;
        handler_state.scan_start_time = millis();
        
        FB2_SetStatus(FB2_RESP_ACK);
        FB2_SendString("WiFi scan started");
    }
}

// ===== BLE SCAN HANDLER =====
void FB2_Handle_BLEScan() {
    Serial.println("[FB2-BLE] Starting BLE scan...");
    
    // BLE scan would go here (requires BLE library integration)
    // For now, placeholder
    
    handler_state.scanning = true;
    handler_state.scan_start_time = millis();
    
    FB2_SetStatus(FB2_RESP_ACK);
    FB2_SendString("BLE scan started");
}

// ===== NRF SCAN HANDLER =====
void FB2_Handle_NRFScan() {
    Serial.println("[FB2-NRF] Starting NRF24 scan...");
    
    // NRF scan would go here (requires RF24 library integration)
    // For now, placeholder
    
    handler_state.scanning = true;
    handler_state.scan_start_time = millis();
    
    FB2_SetStatus(FB2_RESP_ACK);
    FB2_SendString("NRF24 scan started");
}

// ===== WIFI DEAUTH HANDLER =====
void FB2_Handle_WiFiDeauth() {
    Serial.println("[FB2-WiFi] WiFi deauth/attack command received");
    
    // Deauth logic would go here
    // For now, placeholder
    
    FB2_SetStatus(FB2_RESP_ACK);
    FB2_SendString("WiFi deauth ready");
}

// ===== BLE SPAM HANDLER =====
void FB2_Handle_BLESpam() {
    Serial.println("[FB2-BLE] BLE spam/attack command received");
    
    // BLE spam logic would go here
    // For now, placeholder
    
    FB2_SetStatus(FB2_RESP_ACK);
    FB2_SendString("BLE spam ready");
}

// ===== NRF ATTACK HANDLER =====
void FB2_Handle_NRFAttack() {
    Serial.println("[FB2-NRF] NRF24 attack command received");
    
    // NRF attack logic would go here
    // For now, placeholder
    
    FB2_SetStatus(FB2_RESP_ACK);
    FB2_SendString("NRF24 attack ready");
}

// ===== STOP HANDLER =====
void FB2_Handle_Stop() {
    Serial.println("[FB2] Stop command - halting all scans");
    
    handler_state.scanning = false;
    handler_state.current_cmd = 0x00;
    
    FB2_SetStatus(FB2_RESP_DONE);
    FB2_SendString("All scans stopped");
}

// ===== STATUS HANDLER =====
void FB2_Handle_Status() {
    char status_str[32];
    
    if (handler_state.scanning) {
        uint32_t elapsed = millis() - handler_state.scan_start_time;
        snprintf(status_str, sizeof(status_str), "Scanning: %lu ms", elapsed);
    } else {
        snprintf(status_str, sizeof(status_str), "Ready");
    }
    
    FB2_SetStatus(FB2_RESP_ACK);
    FB2_SendString(status_str);
}

// ===== MAIN COMMAND DISPATCHER =====
void FB2_ProcessCommand() {
    if (!FB2_CommandReady()) {
        return;  // No command waiting
    }
    
    uint8_t cmd = FB2_GetCommand();
    uint8_t* data = FB2_GetData();
    
    Serial.printf("[FB2] Dispatching command: 0x%02X\n", cmd);
    
    switch (cmd) {
        case FB2_CMD_PING:
            FB2_SetStatus(FB2_RESP_ACK);
            FB2_SendString("PONG");
            break;
            
        case FB2_CMD_WIFI_SCAN:
            FB2_Handle_WiFiScan();
            break;
            
        case FB2_CMD_BLE_SCAN:
            FB2_Handle_BLEScan();
            break;
            
        case FB2_CMD_NRF_SCAN:
            FB2_Handle_NRFScan();
            break;
            
        case FB2_CMD_WIFI_DEAUTH:
            FB2_Handle_WiFiDeauth();
            break;
            
        case FB2_CMD_BLE_SPAM:
            FB2_Handle_BLESpam();
            break;
            
        case FB2_CMD_NRF_ATTACK:
            FB2_Handle_NRFAttack();
            break;
            
        case FB2_CMD_STOP:
            FB2_Handle_Stop();
            break;
            
        case FB2_CMD_STATUS:
            FB2_Handle_Status();
            break;
            
        default:
            Serial.printf("[FB2] Unknown command: 0x%02X\n", cmd);
            FB2_SetStatus(FB2_RESP_ERROR);
            FB2_SendString("Unknown command");
            break;
    }
    
    handler_state.current_cmd = cmd;
    FB2_ClearCommand();
}

// ===== CONTINUOUS POLLING (call in main loop) =====
void FB2_Update() {
    // Poll SPI for new commands
    FB2_SPI_Poll();
    
    // Process any received command
    FB2_ProcessCommand();
    
    // If scanning, stream results periodically
    if (handler_state.scanning) {
        uint32_t now = millis();
        
        // Stream results every 500ms to avoid flooding
        if ((now - handler_state.last_result_sent) > 500) {
            // TODO: Send scan results based on current command
            // This would fetch results from wifi_scan_obj, BLE lib, RF24 lib, etc.
            
            handler_state.last_result_sent = now;
        }
    }
}

#endif // FB2_COMMAND_HANDLERS_H
