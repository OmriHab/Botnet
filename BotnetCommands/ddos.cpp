#include "ddos.h"

using namespace botnet;

void ddos::PassCtrl() {
	std::string input;
	std::string command_output;
	int command = -1;

	// Cancel command will always be the last command after all the commands.
	int cancel_command = this->GetCommandsList().size() + 1;

	while (1) {
		ThreadSafeLogPrintAlways(this->GetCLIPrompt());
		
		input = ThreadSafeGetLine();

		try {
			command = std::stoi(input);
		}
		catch (const std::invalid_argument& e) {
			ThreadSafeLogPrintAlways("Please enter a valid number between 1 and " + std::to_string(cancel_command) + "\n");
			continue;
		}
		catch (const std::exception& e) {
			ThreadSafeLogPrintAlways("Internal error. Please enter your choice again\n");
			continue;
		}
		if (command > cancel_command || command < 1) {
			ThreadSafeLogPrintAlways("Please enter a valid number between 1 and " + std::to_string(cancel_command) + "\n");
			continue;
		}
		if (command == cancel_command) {
			this->continue_serving = false;
			break;
		}

		Commands command_given = static_cast<Commands>(command);
		try {
			// Execute the given command
			(this->*GetCommandFunction(command_given))();
		}
		catch (const std::exception& e) {
			ThreadSafeLog("Error on command: " + std::string(e.what()));
			continue;
		}

	}

}
