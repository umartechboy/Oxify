#include <WiFi.h>
#include <SocketIoClient.h>
#include <ArduinoJson.h>

// --- C++ Standard Library Headers (Fixes Missing Definitions for std::map, std::function, etc.) ---
#include <functional> 
#include <map>        
#include <vector>     
// -----------------------------------------------------------------

// ===================================
// 1. CONFIGURATION & GLOBALS
// ===================================
// ** UPDATE THESE VALUES **
const char* WIFI_SSID = "M3D";
const char* WIFI_PASSWORD = "m3d159357";

const char* SERVER_HOST = "192.168.8.100"; // e.g., Your PC's local IP address
const int SERVER_PORT = 3000; 
// -------------------------

// Coordination Settings
const char* ROOM_NAME = "ESP32_NET";
const char* MAILBOX_A = "MAILBOX_A_STATUS"; // Periodic status updates
const char* MAILBOX_B = "MAILBOX_B_SYNC";   // Synchronization data

// Global State
SocketIoClient socketIO;
String CLIENT_USERNAME;
bool isConnected = false; // Manually track connection status

unsigned long lastStatusTime = 0;
const long STATUS_INTERVAL_MS = 2000; // Send status every 2 seconds


// Send data to a specific 'mailbox' event within the shared room
void sendMailbox(const char* mailbox_name, JsonObject data) {
    // Use our internal flag to check connection status
    if (!isConnected) { 
        Serial.println("[IO] Cannot send, not connected.");
        return;
    }
    
    // 1. Create the inner message JSON (the 'mailbox' payload)
    StaticJsonDocument<256> mailbox_doc;
    mailbox_doc["mailbox"] = mailbox_name;
    mailbox_doc["from"] = CLIENT_USERNAME;
    mailbox_doc.as<JsonObject>().set(data); 

    String mailbox_payload;
    serializeJson(mailbox_doc, mailbox_payload);

    // 2. Create the outer 'send_message' payload (as required by the ChatEngine server)
    // Format: {"roomId":"ESP32_NET", "message":"{...mailbox_json...}"}
    StaticJsonDocument<256> emit_doc;
    emit_doc["roomId"] = ROOM_NAME;
    emit_doc["message"] = mailbox_payload; 

    String emit_payload;
    serializeJson(emit_doc, emit_payload);

    // 3. Emit to the server's 'send_message' event
    socketIO.emit("send_message", emit_payload.c_str());
    Serial.printf("[TX] Sent to %s: %s\n", mailbox_name, mailbox_payload.c_str());
}

// ===================================
// 3. SOCKET.IO EVENT HANDLERS (using .on() )
// ===================================

// Handler for the system 'connect' event
void onConnect(const char * payload, size_t length) {
    Serial.println("[IO] Connected to Socket.IO server!");
    isConnected = true; // Update internal state
    
    // ** ACTION: Join the Room **
    // Payload for 'join_room' event: {"username":"ESP32_ID_XXXX", "roomId":"ESP32_NET"}
    StaticJsonDocument<128> join_doc;
    join_doc["username"] = CLIENT_USERNAME;
    join_doc["roomId"] = ROOM_NAME;
    String join_payload;
    serializeJson(join_doc, join_payload);
    
    // Emit the join_room event
    socketIO.emit("join_room", join_payload.c_str());
    Serial.printf("[IO] Attempting to join room: %s\n", ROOM_NAME);
}

// Handler for the system 'disconnect' event
void onDisconnect(const char * payload, size_t length) {
    Serial.println("[IO] Disconnected from Socket.IO server!");
    isConnected = false; // Update internal state
}

// Handler for the application 'incoming_message' event
void onIncomingMessage(const char * payload, size_t length) {
    // Payload format: {"roomId":"ESP32_NET", "fromUser":"ESP32_XXXX", "message":"{...}"}
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
        Serial.printf("[RX-ERR] Incoming JSON failed: %s\n", error.c_str());
        return;
    }

    String fromUser = doc["fromUser"];
    String rawMessage = doc["message"];
    
    // Ignore messages sent by this device
    if (fromUser == CLIENT_USERNAME) {
        return;
    }

    // Parse the inner mailbox message JSON string
    StaticJsonDocument<256> mailbox_doc;
    DeserializationError mailbox_error = deserializeJson(mailbox_doc, rawMessage);
    
    if (mailbox_error) {
        Serial.printf("[RX-ERR] Mailbox JSON failed: %s\n", mailbox_error.c_str());
        return;
    }
    
    String mailbox = mailbox_doc["mailbox"];
    String sender  = mailbox_doc["from"];

    if (mailbox == MAILBOX_A) {
        // --- MAILBOX_A: Periodic Status Update ---
        int rssi = mailbox_doc["rssi"].as<int>();
        String status_text = mailbox_doc["status"];
        Serial.printf("[RX - %s] from %s: Status: %s, RSSI: %d dBm\n", 
                      MAILBOX_A, sender.c_str(), status_text.c_str(), rssi);
                      
    } else if (mailbox == MAILBOX_B) {
        // --- MAILBOX_B: Synchronization Data ---
        bool sync_flag = mailbox_doc["sync"].as<bool>();
        long timestamp = mailbox_doc["timestamp"].as<long>();
        Serial.printf("[RX - %s] from %s: Sync Flag: %s, Time (ms): %lu\n", 
                      MAILBOX_B, sender.c_str(), sync_flag ? "TRUE" : "FALSE", timestamp);

        // ** Cross-Device Coordination: Trigger Action **
        if (sync_flag) {
            Serial.println(">>> SYNCHRONIZATION TRIGGERED! Executing coordination task...");
        }
        
    } else if (fromUser.startsWith("admin:")) {
         // Handle Admin/System messages
         Serial.printf("[Admin] %s\n", rawMessage.c_str());
    }
}

// ===================================
// 4. ARDUINO SETUP AND LOOP
// ===================================

void setup() {
    Serial.begin(115200);
    delay(100);
    CLIENT_USERNAME = "Pod 1";
    Serial.printf("\n[SETUP] Starting ESP32 client: %s\n", CLIENT_USERNAME.c_str());

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[WIFI] Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[WIFI] Connected! IP address: " + WiFi.localIP().toString());

    // ** Correct Event Binding using the .on() method **
    socketIO.on("connect", onConnect);
    socketIO.on("disconnect", onDisconnect);
    socketIO.on("incoming_message", onIncomingMessage); // The event broadcast by the Node.js server

    socketIO.begin(SERVER_HOST, SERVER_PORT);
    Serial.printf("[IO] Attempting to connect to: %s:%d\n", SERVER_HOST, SERVER_PORT);
}

void loop() {
    // CRUCIAL: Must be called frequently to process websocket traffic and keep connection alive
    socketIO.loop();

    // Periodic Status Update (MAILBOX_A)
    if (isConnected && millis() - lastStatusTime > STATUS_INTERVAL_MS) {
        lastStatusTime = millis();
        
        // Example: Collect local status data for MAILBOX_A
        StaticJsonDocument<128> status_data_doc;
        status_data_doc["status"] = "ACTIVE";
        status_data_doc["rssi"] = WiFi.RSSI(); 

        sendMailbox(MAILBOX_A, status_data_doc.as<JsonObject>());
    }

    // Manual/Triggered Data Synchronization (MAILBOX_B)
    if (isConnected && Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        if (cmd.startsWith("SYNC")) {
            // Create sync data payload for MAILBOX_B
            StaticJsonDocument<128> sync_data_doc;
            sync_data_doc["sync"] = true;
            sync_data_doc["timestamp"] = millis();

            sendMailbox(MAILBOX_B, sync_data_doc.as<JsonObject>());
        }
    }
    
    delay(10);
}