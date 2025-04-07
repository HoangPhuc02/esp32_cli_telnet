#ifndef __CLI_COMMAND_H__
#define __CLI_COMMAND_H__  

#include <Arduino.h>
#include <TelnetStream.h>
#include <TimeLib.h>
#include <vector>
#include <functional>
#include <cli.h>

// Error message constants
#define CMD_MSG_INVALID_ARGS   "Error: Invalid arguments"
#define CMD_MSG_EXEC_ERROR     "Error: Command execution failed"
#define CMD_MSG_NO_ACCESS      "Error: Permission denied"

/*
* Command result codes
*/

enum class CommandResult {
    OK = 0,              // Command executed successfully
    INVALID_ARGS = -1,   // Invalid arguments
    ERROR = -2,          // Execution error
    NOT_FOUND = -3,      // Command not found
    NO_ACCESS = -4       // Permission denied
};

enum class CommandGroup {
    GENERAL = 0,     // General commands
    SYSTEM,          // System management commands
    PERIPHERALS,     // Peripheral control commands
    NETWORK,         // Network related commands
    DEBUG,           // Debug/diagnostic commands
    APPLICATION,     // Application-specific commands
    USER             // User-defined commands
};

// Advanced command class
class CommandAdvanced: public Command {
    public:
        String usage;
        CommandGroup group;
        uint8_t min_args;
        uint8_t max_args;

        //  CommandAdvanced(const String& cmd, const String& description, std::function<void(const std::vector<String>&)> callback, 
        // const String& usage = "", CommandGroup group = CommandGroup::GENERAL, uint8_t min_args = 0, uint8_t max_args = 0) 
        // : Command(cmd,description,callback)command(cmd), description(description), callback(callback), usage(usage), group(group), min_args(min_args), max_args(max_args) {}        
        CommandAdvanced(const String& cmd, const String& description, std::function<void(const std::vector<String>&)> callback, 
            const String& usage = "", CommandGroup group = CommandGroup::GENERAL, uint8_t min_args = 0, uint8_t max_args = 0) 
        : Command(cmd,description,callback), usage(usage), group(group), min_args(min_args), max_args(max_args) {}
};
class CommandManager {
    public:
        /**
         * Constructor that accepts a reference to the CLI instance
         * @param cliRef Reference to the CLI instance
         */
        CommandManager(ESP32_CLI& cliRef) : m_cli(cliRef) {};
        /**
         * Initialize the command manager
         */
        void begin();
        
        /**
         * Register a new command
         * @param command Command definition
         * @return true if command was registered, false if error
         */
        bool registerCommand(const CommandAdvanced& command);
        
        /**
         * Process a command with the given arguments
         * @param args Arguments (including command as args[0])
         * @return Command result code
         */
        CommandResult processCommand(const std::vector<String>& args);
        
        /**
         * Show help for a specific command
         * @param commandName Command to show help for
         * @return true if command found, false otherwise
         */
        bool showCommandHelp(const String& commandName);
        
        /**
         * Show all commands in a group
         * @param group Command group to show
         * @return Number of commands shown
         */
        int showGroupCommands(CommandGroup group);
        
        /**
         * Get group name as string
         * @param group Command group
         * @return String name of the group
         */
        String getGroupName(CommandGroup group);
        
        /**
         * Register all built-in commands
         */
        void registerBuiltinCommands();

        /***
         * Inteface cli println function
         */
        void cliPrintln(const String& text) ;

        /**
         * Inteface cli print function
         */
        void cliPrint(const String& text);
        
    private:
        ESP32_CLI& m_cli;  // Pointer to the CLI instance
        std::vector<CommandAdvanced> _commands;
        static const char* GROUP_NAMES[];
        // Built-in command handlers
        void cmdHelp(const std::vector<String>& args);
        void cmdInfo(const std::vector<String>& args);
        void cmdStatus(const std::vector<String>& args);
        void cmdRestart(const std::vector<String>& args);
        void cmdMemory(const std::vector<String>& args);
        void cmdWifi(const std::vector<String>& args);
        void cmdGPIO(const std::vector<String>& args);
        void cmdInterface(const std::vector<String>& args);
        void cmdReadSensor(const std::vector<String>& args);
};

// Global instance
extern CommandManager Commands;




#endif