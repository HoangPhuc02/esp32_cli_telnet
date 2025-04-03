#ifndef ESP32_CLI_H
#define ESP32_CLI_H

#include <Arduino.h>
#include <TelnetStream.h>
#include <vector>
#include <functional>
#include <string>

enum class OutputInterface {
  serial,
  telnet,
  BOTH
};

class Command {
public:
  Command(const String& cmd, const String& description, std::function<void(const std::vector<String>&)> callback) 
    : command(cmd), description(description), callback(callback) {}
  
  String command;
  String description;
  std::function<void(const std::vector<String>&)> callback;
};

class ESP32_CLI {
public:
  ESP32_CLI();
  
  void begin(unsigned long baudRate = 115200);
  void setInterface(OutputInterface interface);
  void switchInterface();
  OutputInterface getCurrentInterface();
  
  void print(const String& text);
  void println(const String& text);
  
  void update();  // Call this in loop()
  
  void addCommand(const String& command, const String& description, std::function<void(const std::vector<String>&)> callback);
  void listCommands();
  
  bool isClientConnected();

private:
  OutputInterface _interface;
  String _inputBuffer;
  std::vector<Command> _commands;
  
  void processCommand(const String& cmd);
  void help();
  std::vector<String> splitString(const String& input, char delimiter);
};

extern ESP32_CLI CLI;  // Global instance

#endif // ESP32_CLI_H