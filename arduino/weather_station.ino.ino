#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>  // WiFiManager library
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

// ============================================
// CONFIGURATION
// ============================================
const char* serverUrl = "https://haneefputtur.com/esp12e/receive_data.php";

// Device name for WiFi setup portal
const char* apName = "Haneef_Weather_Setup";
const char* apPassword = "12345678";  // Password for setup portal (min 8 chars)

// Reset button pin (optional - connect button between D3 and GND)
#define RESET_BUTTON_PIN D3  // GPIO0
#define BUTTON_PRESS_TIME 5000  // 5 seconds to reset WiFi

// ============================================
// SENSOR SETUP
// ============================================
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C

// Timing variables
unsigned long previousMillis = 0;
const long interval = 180000; // 3 minutes in milliseconds

// Button timing
unsigned long buttonPressStart = 0;
bool buttonPressed = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("========================================");
  Serial.println("   HANEEF WEATHER STATION v2.0");
  Serial.println("   With WiFi Configuration Portal");
  Serial.println("========================================\n");
  
  // Setup reset button (optional)
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize BME280 sensor
  Serial.println("Initializing BME280 sensor...");
  if (!bme.begin(0x76)) {
    Serial.println("✗ ERROR: Could not find BME280 sensor!");
    Serial.println("\nCheck wiring:");
    Serial.println("  BME280 VCC -> 3.3V");
    Serial.println("  BME280 GND -> GND");
    Serial.println("  BME280 SCL -> D1 (GPIO5)");
    Serial.println("  BME280 SDA -> D2 (GPIO4)");
    Serial.println("\nContinuing without sensor...");
  } else {
    Serial.println("✓ BME280 sensor initialized successfully\n");
  }

  // WiFiManager Setup
  WiFiManager wifiManager;
  
  // Uncomment to reset saved WiFi credentials (for testing)
  // wifiManager.resetSettings();
  
  // Set custom AP name and password
  Serial.println("Starting WiFi configuration...");
  Serial.println("If not connected, device will create WiFi hotspot:\n");
  Serial.print("  Network Name: ");
  Serial.println(apName);
  Serial.print("  Password: ");
  Serial.println(apPassword);
  Serial.println("\nConnect to this network and configure WiFi\n");
  
  // Timeout for configuration portal (3 minutes)
  wifiManager.setConfigPortalTimeout(180);
  
  // Custom parameters (optional - can add server URL configuration)
  // WiFiManagerParameter custom_server("server", "Server URL", serverUrl, 100);
  // wifiManager.addParameter(&custom_server);
  
  // Try to connect to saved WiFi or start configuration portal
  if (!wifiManager.autoConnect(apName, apPassword)) {
    Serial.println("✗ Failed to connect and timeout reached");
    Serial.println("Restarting device...");
    delay(3000);
    ESP.restart();
  }
  
  // Successfully connected
  Serial.println("\n========================================");
  Serial.println("✓ WiFi Connected Successfully!");
  Serial.println("========================================");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.println("========================================\n");
  
  // Send first reading immediately
  Serial.println("Sending initial data...\n");
  sendSensorData();
  
  Serial.println("\n✓ Setup complete!");
  Serial.println("Device will send data every 3 minutes");
  Serial.println("Hold RESET button (D3) for 5 seconds to reconfigure WiFi\n");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check reset button
  checkResetButton();
  
  // Check if it's time to send data (every 3 minutes)
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendSensorData();
  }
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠ WiFi disconnected. Attempting to reconnect...");
    WiFi.reconnect();
    delay(5000);
    
    // If still not connected after 30 seconds, restart
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 6) {
      delay(5000);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n✗ Cannot reconnect. Restarting...");
      ESP.restart();
    } else {
      Serial.println("\n✓ Reconnected!");
    }
  }
}

void checkResetButton() {
  // Check if reset button is pressed
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressStart = millis();
      Serial.println("\n⚠ Reset button pressed...");
    }
    
    // Check if button held for required time
    if (millis() - buttonPressStart >= BUTTON_PRESS_TIME) {
      Serial.println("\n========================================");
      Serial.println("   RESETTING WIFI CONFIGURATION");
      Serial.println("========================================\n");
      
      WiFiManager wifiManager;
      wifiManager.resetSettings();
      
      Serial.println("✓ WiFi credentials cleared!");
      Serial.println("Device will restart in configuration mode...\n");
      delay(2000);
      ESP.restart();
    }
  } else {
    buttonPressed = false;
  }
}

void sendSensorData() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    HTTPClient http;
    
    // Disable SSL certificate verification
    client.setInsecure();
    
    // Read sensor data
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    
    // Check if readings are valid
    if (isnan(temperature) || isnan(humidity) || isnan(pressure)) {
      Serial.println("✗ Failed to read from BME280 sensor!");
      return;
    }
    
    // Display readings on Serial Monitor
    Serial.println("========================================");
    Serial.println("         SENSOR READINGS");
    Serial.println("========================================");
    Serial.print("  Temperature: "); 
    Serial.print(temperature, 2); 
    Serial.println(" °C");
    Serial.print("  Humidity:    "); 
    Serial.print(humidity, 2); 
    Serial.println(" %");
    Serial.print("  Pressure:    "); 
    Serial.print(pressure, 2); 
    Serial.println(" hPa");
    Serial.print("  Altitude:    "); 
    Serial.print(altitude, 2); 
    Serial.println(" m");
    Serial.println("========================================\n");
    
    // Prepare POST data
    String postData = "temperature=" + String(temperature, 2) +
                      "&humidity=" + String(humidity, 2) +
                      "&pressure=" + String(pressure, 2) +
                      "&altitude=" + String(altitude, 2);
    
    Serial.println("Sending data to server...");
    Serial.print("URL: ");
    Serial.println(serverUrl);
    
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.setTimeout(15000); // 15 second timeout
    
    int httpResponseCode = http.POST(postData);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("✓ Server Response Code: ");
      Serial.println(httpResponseCode);
      
      if (httpResponseCode == 200) {
        Serial.println("✓ Data sent successfully!");
      }
      
      Serial.print("Response: ");
      Serial.println(response);
    } else {
      Serial.print("✗ Error sending data. Code: ");
      Serial.println(httpResponseCode);
      Serial.print("Error: ");
      Serial.println(http.errorToString(httpResponseCode));
    }
    
    http.end();
    
    Serial.println("\nNext update in 3 minutes...\n");
    
  } else {
    Serial.println("✗ WiFi not connected. Cannot send data.");
  }
}
