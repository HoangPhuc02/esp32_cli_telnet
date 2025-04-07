#include "cli_command.h"

// Define the static group names

const char* CommandManager::GROUP_NAMES[] = {
    "General",
    "System",
    "Peripherals",
    "Network",
    "Debug",
    "Application",
    "User"
};

// Create global instance connected to the global CLI instance
CommandManager Commands(CLI);

void CommandManager::begin() {
    // Register built-in commands
    _commands.clear(); // Clear any existing commands
    //debug message
    cliPrintln("Registering built-in commands...");

    registerBuiltinCommands();
    cliPrintln("Type 'help' for available commands");
    cliPrint("> ");
}

//Register a command
bool CommandManager::registerCommand(const CommandAdvanced& command)
{
    // Check command list if command already exists
    for (const auto& cmd : _commands){
        if (cmd.command.equalsIgnoreCase(command.command)) {
            return false; // Command already exists
        }
    }
    //add to internal command list
    _commands.push_back(command);
    //Register withe the CLI system
    m_cli.addCommand(command.command, command.description, command.callback);
    return true;
}

CommandResult CommandManager::processCommand(const std::vector<String>& args){

    // Check if the args from m_cli is valid
    if(args.empty()) {
        return CommandResult::INVALID_ARGS; // Empty arguments
    }

    // If arguments are provided, check if the first argument is a command
    // Find the command
    for (const auto& cmd : _commands)
    {
        if (cmd.command.equalsIgnoreCase(args[0])) {
            // Check argument count
            if (args.size() < cmd.min_args ) {
                cliPrintln(CMD_MSG_INVALID_ARGS);
                cliPrint("Usage: ");
                return CommandResult::INVALID_ARGS; // Invalid number of arguments
            }
            if(args.size() > cmd.max_args) {
                cliPrintln("Warning: Too many arguments, ignoring extra ones.");
            }

            // Execute the command
            try {
                cmd.callback(args); // Call the command's callback function
                return CommandResult::OK; // Command executed successfully
            // 
            } catch (...) {
                cliPrintln(CMD_MSG_EXEC_ERROR);
                return CommandResult::ERROR; // Execution error
            }
        }
    }

    //Command not found
    cliPrint("Unknown command:");
    cliPrintln(args[0]);
    cliPrintln("Type 'help' for available commands");
    return CommandResult::NOT_FOUND; // Command not found
}

// Show help to fine command specific
bool CommandManager::showCommandHelp(const String& commandName){
    // find command
    for(const auto& cmd : _commands) {
        if (cmd.command.equalsIgnoreCase(commandName)) {
            cliPrint("Command: ");
            cliPrintln(cmd.command);
            cliPrint("Description: ");
            cliPrintln(cmd.description);
            cliPrint("Usage: ");
            cliPrintln(cmd.usage.length() > 0 ? cmd.usage : cmd.command);
            cliPrint("Group: ");
            cliPrintln(getGroupName(cmd.group));
            cliPrint("Arguments: ");
            cliPrint(cmd.min_args > 1 ? String(cmd.min_args - 1) : "0");
            cliPrint(" to ");
            cliPrint(cmd.max_args > 1 ? String(cmd.max_args - 1) : "0");
            cliPrintln(" arguments");
            return true;
        }
    }
    return false; // command not found
}
//explain this line
String CommandManager::getGroupName(CommandGroup group){
    int index = static_cast<int>(group);
    if (index >= 0 && index < sizeof(GROUP_NAMES) / sizeof(GROUP_NAMES[0])) {
        return GROUP_NAMES[index];
    }
    return "Unknown";
}
int CommandManager::showGroupCommands(CommandGroup group) {
    // Check if the group is valid
    cliPrintln("");
    cliPrint("=== ");
    cliPrint(getGroupName(group));
    cliPrintln(" Commands ===");


    int count = 0;
    for (const auto& cmd : _commands) {
        // check command is linked to the specific group
        if (cmd.group == group) {
            cliPrint("  ");
            cliPrint(cmd.command);
            // Add padding to align descriptions
            for (size_t i = cmd.command.length(); i < 15; i++) {
                cliPrint(" ");
            }
            cliPrint("- ");
            cliPrintln(cmd.description);
            count++;
        }
    }
    if(count == 0){
        cliPrintln("No commands found in this group.");
    }
    return count;
}

void CommandManager::registerBuiltinCommands(){
    //Help command

    registerCommand(CommandAdvanced(
        "help", 
        "List all available commands",
         [this](const std::vector<String>& args) {cmdHelp(args);},
         "help [command]",
         CommandGroup::GENERAL,
         1,2
    ));

    //Status command
    registerCommand(CommandAdvanced(
        "status", 
        "Show system status",
         [this](const std::vector<String>& args) {cmdStatus(args);},
         "status",
         CommandGroup::SYSTEM,
         1,1
    ));
    
    //Info command
    registerCommand(CommandAdvanced(
        "info", 
        "Show system information",
         [this](const std::vector<String>& args) {cmdInfo(args);},
         "info [detail]",
         CommandGroup::SYSTEM,
         1,22
    ));

    // Restart command
    registerCommand(CommandAdvanced(
        "restart",
        "Restart the ESP32",
        [this](const std::vector<String>& args) { cmdRestart(args); },
        "restart",
        CommandGroup::SYSTEM,
        1, 1
    ));
    
    // Memory command
    registerCommand(CommandAdvanced(
        "memory",
        "Show memory usage",
        [this](const std::vector<String>& args) { cmdMemory(args); },
        "memory",
        CommandGroup::SYSTEM,
        1, 1
    ));
    
    // WiFi command
    registerCommand(CommandAdvanced(
        "wifi",
        "WiFi operations and information",
        [this](const std::vector<String>& args) { cmdWifi(args); },
        "wifi <status|scan|connect|disconnect>",
        CommandGroup::NETWORK,
        2, 4
    ));
    
    // GPIO command
    registerCommand(CommandAdvanced(
        "gpio",
        "Control GPIO pins",
        [this](const std::vector<String>& args) { cmdGPIO(args); },
        "gpio <pin> <read|set|clear|toggle>",
        CommandGroup::PERIPHERALS,
        3, 3
    ));
    
    // Interface command
    registerCommand(CommandAdvanced(
        "interface",
        "Change output interface (serial/telnet/both)",
        [this](const std::vector<String>& args) { cmdInterface(args); },
        "interface [serial|telnet|both]",
        CommandGroup::GENERAL,
        1, 2
    ));

    //Read sensor data
    registerCommand(CommandAdvanced(
        "read",
        "Read sensor data",
        [this](const std::vector<String>& args) {cmdReadSensor(args);},
        "read adc",
        CommandGroup::PERIPHERALS,
        2, 2
    ));

}
void CommandManager::cliPrintln(const String& text) {
    m_cli.println(text);
}

void CommandManager::cliPrint(const String& text) {
    m_cli.print(text);
}

//---------- Command Implementations ----------

void CommandManager::cmdHelp(const std::vector<String>& args) {
    if (args.size() > 1) {
        // Show help for specific command
        if (!showCommandHelp(args[1])) {
            cliPrint("Unknown command: ");
            cliPrintln(args[1]);
        }
    } else {
        // Show all command groups
        for (int i = 0; i <= static_cast<int>(CommandGroup::USER); i++) {
            showGroupCommands(static_cast<CommandGroup>(i));
        }
    }
}
void CommandManager::cmdStatus(const std::vector<String>& args) {
    
    cliPrintln("--- System Status ---");
    // WiFi status
    cliPrint("WiFi: ");
    cliPrint(WiFi.status() == WL_CONNECTED ? "Connected to " : "Disconnected");
    if (WiFi.status() == WL_CONNECTED) {
      cliPrintln(WiFi.SSID());
      cliPrint("IP: ");
      cliPrintln(WiFi.localIP().toString());
      cliPrint("Signal: ");
      cliPrint(String(WiFi.RSSI()));
      cliPrintln(" dBm");
    } else {
      cliPrintln("");
    }
    
    // Time
    char timeStr[20];
    sprintf(timeStr, "%02d-%02d-%02d %02d:%02d:%02d", 
            year(), month(), day(), hour(), minute(), second());
    cliPrint("Current time: ");
    cliPrintln(timeStr);
    
    // Memory
    cliPrint("Free heap: ");
    cliPrint(String(ESP.getFreeHeap()));
    cliPrintln(" bytes");
    
    // Telnet status
    //TODO: check if TelnetStream is connected
    // cliPrint("Telnet client: ");
    // cliPrintln(CLI.isClientConnected() ? "Connected" : "Not connected");
    
    // Current interface
    cliPrint("Current interface: ");
    switch (CLI.getCurrentInterface()) {
      case OutputInterface::serial:
        cliPrintln("SERIAL");
        break;
      case OutputInterface::telnet:
        cliPrintln("TELNET");
        break;
      case OutputInterface::BOTH:
        cliPrintln("BOTH");
        break;
    }
}
void CommandManager::cmdInfo(const std::vector<String>& args) {
    cliPrintln("ESP32 System Information:");
    cliPrint("- Chip model: ");
    cliPrintln(ESP.getChipModel());
    cliPrint("- Chip cores: ");
    cliPrintln(String(ESP.getChipCores()));
    cliPrint("- CPU frequency: ");
    cliPrint(String(ESP.getCpuFreqMHz()));
    cliPrintln(" MHz");
    cliPrint("- Flash size: ");
    cliPrint(String(ESP.getFlashChipSize() / 1024 / 1024));
    cliPrintln(" MB");
    cliPrint("- SDK version: ");
    cliPrintln(ESP.getSdkVersion());
    
    if (args.size() > 1 && args[1].equalsIgnoreCase("detail")) {
        cliPrintln("\nDetailed Information:");
        cliPrint("- Heap size: ");
        cliPrint(String(ESP.getHeapSize() / 1024));
        cliPrintln(" KB");
        cliPrint("- MAC address: ");
        cliPrintln(WiFi.macAddress());
        cliPrint("- Sketch size: ");
        cliPrint(String(ESP.getSketchSize() / 1024));
        cliPrintln(" KB");
        cliPrint("- Free sketch space: ");
        cliPrint(String(ESP.getFreeSketchSpace() / 1024));
        cliPrintln(" KB");
    }
}

void CommandManager::cmdRestart(const std::vector<String>& args) {
    cliPrintln("Restarting ESP32...");
    delay(500);
    ESP.restart();
}

// Implement the remaining command handlers...
void CommandManager::cmdMemory(const std::vector<String>& args) {
    cliPrintln("Memory Information:");
    cliPrint("- Free heap: ");
    cliPrint(String(ESP.getFreeHeap() / 1024));
    cliPrintln(" KB");
    cliPrint("- Heap size: ");
    cliPrint(String(ESP.getHeapSize() / 1024));
    cliPrintln(" KB");
    cliPrint("- Min free heap: ");
    cliPrint(String(ESP.getMinFreeHeap() / 1024));
    cliPrintln(" KB");
    cliPrint("- Max alloc heap: ");
    cliPrint(String(ESP.getMaxAllocHeap() / 1024));
    cliPrintln(" KB");
}

void CommandManager::cmdWifi(const std::vector<String>& args) {
    if (args.size() < 2) {
        cliPrintln("Usage: wifi <status|scan|connect|disconnect>");
        return;
    }
    
    if (args[1].equalsIgnoreCase("status")) {
        cliPrintln("WiFi Status:");
        if (WiFi.status() == WL_CONNECTED) {
            cliPrintln("- Status: Connected");
            cliPrint("- SSID: ");
            cliPrintln(WiFi.SSID());
            cliPrint("- IP address: ");
            cliPrintln(WiFi.localIP().toString());
            cliPrint("- Signal strength: ");
            cliPrint(String(WiFi.RSSI()));
            cliPrintln(" dBm");
        } else {
            cliPrintln("- Status: Disconnected");
        }
    } else if (args[1].equalsIgnoreCase("scan")) {
        cliPrintln("Scanning for WiFi networks...");
        int networks = WiFi.scanNetworks();
        
        if (networks == 0) {
            cliPrintln("No networks found");
        } else {
            cliPrint(String(networks));
            cliPrintln(" networks found:");
            
            for (int i = 0; i < networks; i++) {
                cliPrint(String(i + 1));
                cliPrint(": ");
                cliPrint(WiFi.SSID(i));
                cliPrint(" (");
                cliPrint(String(WiFi.RSSI(i)));
                cliPrint(" dBm) ");
                cliPrintln((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Encrypted");
                delay(10);
            }
        }
        
        WiFi.scanDelete();
    } else if (args[1].equalsIgnoreCase("connect") && args.size() >= 4) {
        cliPrint("Connecting to: ");
        cliPrintln(args[2]);
        
        WiFi.begin(args[2].c_str(), args[3].c_str());
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            cliPrint(".");
            attempts++;
        }
        cliPrintln("");
        
        if (WiFi.status() == WL_CONNECTED) {
            cliPrintln("Connected successfully!");
            cliPrint("IP address: ");
            cliPrintln(WiFi.localIP().toString());
        } else {
            cliPrintln("Failed to connect");
        }
    } else if (args[1].equalsIgnoreCase("disconnect")) {
        WiFi.disconnect();
        cliPrintln("WiFi disconnected");
    } else {
        cliPrintln("Unknown WiFi command");
    }
}

void CommandManager::cmdGPIO(const std::vector<String>& args) {
    if (args.size() < 3) {
        cliPrintln("Usage: gpio <pin> <read|set|clear|toggle>");
        return;
    }
    
    int pin = args[1].toInt();
    if (pin < 0 || pin > 39) {
        cliPrintln("Invalid pin number. Use 0-39");
        return;
    }
    
    if (args[2].equalsIgnoreCase("read")) {
        cliPrint("GPIO ");
        cliPrint(String(pin));
        cliPrint(" value: ");
        pinMode(pin, INPUT);
        cliPrintln(String(digitalRead(pin)));
    } else if (args[2].equalsIgnoreCase("set")) {
        cliPrint("Setting GPIO ");
        cliPrint(String(pin));
        cliPrintln(" HIGH");
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    } else if (args[2].equalsIgnoreCase("clear")) {
        cliPrint("Setting GPIO ");
        cliPrint(String(pin));
        cliPrintln(" LOW");
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    } else if (args[2].equalsIgnoreCase("toggle")) {
        cliPrint("Toggling GPIO ");
        cliPrintln(String(pin));
        pinMode(pin, OUTPUT);
        digitalWrite(pin, !digitalRead(pin));
    } else {
        cliPrintln("Unknown GPIO operation. Use read, set, clear, or toggle");
    }
}

void CommandManager::cmdInterface(const std::vector<String>& args) {
    if (args.size() > 1) {
        if (args[1].equalsIgnoreCase("serial")) {
            m_cli.setInterface(OutputInterface::serial);
        } else if (args[1].equalsIgnoreCase("telnet")) {
            m_cli.setInterface(OutputInterface::telnet);
        } else if (args[1].equalsIgnoreCase("both")) {
            m_cli.setInterface(OutputInterface::BOTH);
        } else {
            cliPrintln("Invalid interface. Use: serial, telnet, or both");
        }
    } else {
        // Show current interface
        cliPrint("Current interface: ");
        switch (m_cli.getCurrentInterface()) {
            case OutputInterface::serial:
                cliPrintln("SERIAL");
                break;
            case OutputInterface::telnet:
                cliPrintln("TELNET");
                break;
            case OutputInterface::BOTH:
                cliPrintln("BOTH");
                break;
        }
    }
}

void CommandManager::cmdReadSensor(const std::vector<String>& args) {
    if (args.size() > 1 && args[1].equalsIgnoreCase("adc")) {
        int value = analogRead(A0);
        cliPrint("ADC value: ");
        cliPrintln(String(value));
    } else {
        cliPrintln("Usage: read adc");
    }
}