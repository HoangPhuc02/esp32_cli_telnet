#include "cli.h"

ESP32_CLI CLI;  // Create global instance

ESP32_CLI::ESP32_CLI() {
  _interface = OutputInterface::serial;
  _inputBuffer = "";
}

void ESP32_CLI::begin(unsigned long baudRate) {
  Serial.begin(baudRate);
  while (!Serial) {
    ; // Wait for Serial to be ready
  }
  
  // Add built-in commands
  addCommand("help", "List all available commands", [this](const std::vector<String>& args) {
    this->listCommands();
  });
  
  addCommand("interface", "Switch or show interface (serial/telnet/both)", [this](const std::vector<String>& args) {
    if (args.size() > 1) {
      if (args[1].equalsIgnoreCase("serial")) {
        setInterface(OutputInterface::serial);
      } else if (args[1].equalsIgnoreCase("telnet")) {
        setInterface(OutputInterface::telnet);
      } else if (args[1].equalsIgnoreCase("both")) {
        setInterface(OutputInterface::BOTH);
      } else {
        println("Invalid interface. Use: serial, telnet, or both");
      }
    } else {
      // Show current interface
      switch (_interface) {
        case OutputInterface::serial:
          println("Current interface: SERIAL");
          break;
        case OutputInterface::telnet:
          println("Current interface: TELNET");
          break;
        case OutputInterface::BOTH:
          println("Current interface: BOTH");
          break;
      }
    }
  });
}

void ESP32_CLI::setInterface(OutputInterface interface) {
  _interface = interface;
  
  String msg = "Switched to ";
  switch (_interface) {
    case OutputInterface::serial:
      msg += "SERIAL";
      break;
    case OutputInterface::telnet:
      msg += "TELNET";
      break;
    case OutputInterface::BOTH:
      msg += "BOTH";
      break;
  }
  println(msg);
}

void ESP32_CLI::switchInterface() {
  switch (_interface) {
    case OutputInterface::serial:
      setInterface(OutputInterface::telnet);
      break;
    case OutputInterface::telnet:
      setInterface(OutputInterface::BOTH);
      break;
    case OutputInterface::BOTH:
      setInterface(OutputInterface::serial);
      break;
  }
}

OutputInterface ESP32_CLI::getCurrentInterface() {
  return _interface;
}

void ESP32_CLI::print(const String& text) {
  if (_interface == OutputInterface::serial || _interface == OutputInterface::BOTH) {
    Serial.print(text);
  }
  
  if (_interface == OutputInterface::telnet || _interface == OutputInterface::BOTH) {
    TelnetStream.print(text);
  }
}

void ESP32_CLI::println(const String& text) {
  if (_interface == OutputInterface::serial || _interface == OutputInterface::BOTH) {
    Serial.println(text);
  }
  
  if (_interface == OutputInterface::telnet || _interface == OutputInterface::BOTH) {
    TelnetStream.println(text);
  }
}

void ESP32_CLI::update() {
  // Check for telnet input
  if (TelnetStream.available()) {
    char c = TelnetStream.read();
    
    if (c == '\n' || c == '\r') {
      if (_inputBuffer.length() > 0) {
        processCommand(_inputBuffer);
        _inputBuffer = "";
      }
    } else if (c == 8 || c == 127) { // Backspace
      if (_inputBuffer.length() > 0) {
        _inputBuffer.remove(_inputBuffer.length() - 1);
        // Echo backspace
        if (_interface == OutputInterface::telnet || _interface == OutputInterface::BOTH) {
          TelnetStream.print("\b \b");
        }
      }
    } else {
      _inputBuffer += c;
      // Echo character
      if (_interface == OutputInterface::telnet || _interface == OutputInterface::BOTH) {
        TelnetStream.print(c);
      }
    }
  }
  
  // Check for serial input
  if (Serial.available()) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      if (_inputBuffer.length() > 0) {
        processCommand(_inputBuffer);
        _inputBuffer = "";
      }
    } else if (c == 8 || c == 127) { // Backspace
      if (_inputBuffer.length() > 0) {
        _inputBuffer.remove(_inputBuffer.length() - 1);
        // Echo backspace
        if (_interface == OutputInterface::serial || _interface == OutputInterface::BOTH) {
          Serial.print("\b \b");
        }
      }
    } else {
      _inputBuffer += c;
      // Echo character
      if (_interface == OutputInterface::serial || _interface == OutputInterface::BOTH) {
        Serial.print(c);
      }
    }
  }
}

void ESP32_CLI::addCommand(const String& command, const String& description, std::function<void(const std::vector<String>&)> callback) {
  _commands.push_back(Command(command, description, callback));
}

void ESP32_CLI::listCommands() {
  println("Available commands:");
  for (const auto& cmd : _commands) {
    print("  ");
    print(cmd.command);
    print(" - ");
    println(cmd.description);
  }
}

void ESP32_CLI::processCommand(const String& cmd) {
  println(""); // New line after command
  
  // Split the command and arguments
  std::vector<String> parts = splitString(cmd, ' ');
  
  if (parts.empty()) {
    return;
  }
  
  String command = parts[0];
  
  // Find and execute command
  bool found = false;
  for (const auto& c : _commands) {
    if (c.command.equalsIgnoreCase(command)) {
      c.callback(parts);
      found = true;
      break;
    }
  }
  
  if (!found) {
    print("Unknown command: ");
    println(command);
    println("Type 'help' for available commands");
  }
  
  // Print prompt
  print("> ");
}

std::vector<String> ESP32_CLI::splitString(const String& input, char delimiter) {
  std::vector<String> result;
  int start = 0;
  int end = 0;
  
  while (end < input.length()) {
    if (input[end] == delimiter) {
      if (end > start) {
        result.push_back(input.substring(start, end));
      }
      start = end + 1;
    }
    end++;
  }
  
  if (end > start) {
    result.push_back(input.substring(start, end));
  }
  
  return result;
}

bool ESP32_CLI::isClientConnected() {
//   return !TelnetStream.disconnected();


// lib/cli/cli.cpp: In member function 'bool ESP32_CLI::isClientConnected()':
// lib/cli/cli.cpp:228:37: error: 'boolean TelnetStreamClass::disconnected()' is private within this context
//    return !TelnetStream.disconnected();
//                                      ^
// In file included from lib/cli/cli.h:5,
//                  from lib/cli/cli.cpp:1:
// .pio/libdeps/esp32doit-devkit-v1/TelnetStream/src/TelnetStream.h:30:10: note: declared private here
//   boolean disconnected();
//           ^~~~~~~~~~~~
// *** [.pio\build\esp32doit-devkit-v1\lib040\cli\cli.cpp.o] Error 1
}