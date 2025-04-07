#include <Arduino.h>
#include <WiFi.h>
#include <TimeLib.h>
#include <TelnetStream.h>
#include "cli.h"
#include "cli_command.h"

//TODO
/**
 * TODO add CLI command
 * Add Circular buffer to handle uart command
 * How Telnet work
 * 
 */
const long gmtOffset_sec = 25200;//3600 * Time
const int daylightOffset_sec = 3600;

// WiFi credentials
const char ssid[] = "WIFI_SSID";
const char pass[] = "WIFI_PASS";



// Function prototypes
void setupWiFi();
void setupTime();
void logSensorData();
void restartESP();

void setup() {
  // Start CLI (which initializes Serial)
  CLI.begin(115200);
  CLI.println("ESP32 CLI Demo");
  
  // Initialize CommandManager
  // debug message
  // CLI.println("Initializing CommandManager...");

  

  
  // Commands.registerCommand(CommandAdvanced(
  //   "read",
  //   "Read sensor data",
  //   [](const std::vector<String>& args) {
  //     if (args.size() > 1 && args[1].equalsIgnoreCase("adc")) {
  //       int value = analogRead(A0);
  //       CLI.print("ADC value: ");
  //       CLI.println(String(value));
  //     } else {
  //       CLI.println("Usage: read adc");
  //     }
  //   },
  //   "read adc",
  //   CommandGroup::PERIPHERALS,
  //   2, 2
  // ));
  
  // Setup network
  //debug message 
  // CLI.println("Setting up WiFi...");
  setupWiFi();

  //debug message
  // CLI.println("Setting up time...");
  setupTime();
  
  // Initialize telnet
  Commands.begin();
  TelnetStream.begin();
  

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



void restartESP() {
  CLI.println("Restarting ESP32...");
  delay(500);
  ESP.restart();
}
