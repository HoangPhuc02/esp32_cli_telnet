#include <Arduino.h>
#include <WiFi.h>
#include <TimeLib.h>
#include <TelnetStream.h>
#include "cli.h"

const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// WiFi credentials
const char ssid[] = "Phuc Phuc";
const char pass[] = "phuc05042002";

// Function prototypes
void setupWiFi();
void setupTime();
void logSensorData();
void showStatus();
void restartESP();

void setup() {
  // Start CLI (which initializes Serial)
  CLI.begin(115200);
  CLI.println("ESP32 CLI Demo");
  
  // Setup network
  setupWiFi();
  setupTime();
  
  // Initialize telnet
  TelnetStream.begin();
  
  // Add custom commands
  CLI.addCommand("status", "Show system status", [](const std::vector<String>& args) {
    showStatus();
  });
  
  CLI.addCommand("restart", "Restart the ESP32", [](const std::vector<String>& args) {
    restartESP();
  });
  
  CLI.addCommand("read", "Read sensor (usage: read adc)", [](const std::vector<String>& args) {
    if (args.size() > 1 && args[1].equalsIgnoreCase("adc")) {
      int value = analogRead(A0);
      CLI.print("ADC value: ");
      CLI.println(String(value));
    } else {
      CLI.println("Usage: read adc");
    }
  });
  
  CLI.println("Type 'help' for available commands");
  CLI.print("> ");
}

void loop() {
  // Process CLI input
  CLI.update();
  
  // Log sensor data periodically
  static unsigned long nextLog = 0;
  if (millis() - nextLog > 5000) {
    nextLog = millis();
    logSensorData();
  }
}

void setupWiFi() {
  CLI.print("Connecting to WiFi: ");
  CLI.println(ssid);
  
  WiFi.begin(ssid, pass);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    CLI.println("Connection failed, retrying...");
    WiFi.begin(ssid, pass);
    delay(100);
  }
  
  CLI.println("WiFi connected!");
  CLI.print("IP address: ");
  CLI.println(WiFi.localIP().toString());
  CLI.println("Connect with Telnet client to this IP");
}

void setupTime() {
  CLI.println("Synchronizing time...");
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
  
  time_t now = time(nullptr);
  while (now < SECS_YR_2000) {
    delay(100);
    now = time(nullptr);
  }
  setTime(now);
  
  CLI.println("Time synchronized");
}

void logSensorData() {
  char timeStr[20];
  sprintf(timeStr, "%02d-%02d-%02d %02d:%02d:%02d", 
          year(), month(), day(), hour(), minute(), second());
  
  int adcValue = analogRead(A0);
  
  // Only log if in BOTH interface mode or if client is connected in TELNET mode
  if (CLI.getCurrentInterface() == OutputInterface::BOTH || 
      (CLI.getCurrentInterface() == OutputInterface::telnet && CLI.isClientConnected())) {
    CLI.print(timeStr);
    CLI.print(" ADC: ");
    CLI.println(String(adcValue));
  }
}

void showStatus() {
  CLI.println("--- System Status ---");
  
  // WiFi status
  CLI.print("WiFi: ");
  CLI.print(WiFi.status() == WL_CONNECTED ? "Connected to " : "Disconnected");
  if (WiFi.status() == WL_CONNECTED) {
    CLI.println(ssid);
    CLI.print("IP: ");
    CLI.println(WiFi.localIP().toString());
    CLI.print("Signal: ");
    CLI.print(String(WiFi.RSSI()));
    CLI.println(" dBm");
  } else {
    CLI.println("");
  }
  
  // Time
  char timeStr[20];
  sprintf(timeStr, "%02d-%02d-%02d %02d:%02d:%02d", 
          year(), month(), day(), hour(), minute(), second());
  CLI.print("Current time: ");
  CLI.println(timeStr);
  
  // Memory
  CLI.print("Free heap: ");
  CLI.print(String(ESP.getFreeHeap()));
  CLI.println(" bytes");
  
  // Telnet status
  //TODO: check if TelnetStream is connected
  // CLI.print("Telnet client: ");
  // CLI.println(CLI.isClientConnected() ? "Connected" : "Not connected");
  
  // Current interface
  CLI.print("Current interface: ");
  switch (CLI.getCurrentInterface()) {
    case OutputInterface::serial:
      CLI.println("SERIAL");
      break;
    case OutputInterface::telnet:
      CLI.println("TELNET");
      break;
    case OutputInterface::BOTH:
      CLI.println("BOTH");
      break;
  }
}

void restartESP() {
  CLI.println("Restarting ESP32...");
  delay(500);
  ESP.restart();
}
