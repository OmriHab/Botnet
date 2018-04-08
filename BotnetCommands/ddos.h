#ifndef DDOS_H
#define DDOS_H

namespace botnet {

class ddos {
private:
	
public:
	ddos();
	~ddos();

	virtual void PassCtrl();


	/**
	* When adding a command add its name to the enum Commands,
	* and to the "commands" vector below in the same order.
	* Make sure also to write a void(void) function that executes the command
	* and it to the "func_vector" below, also in the same order as the enum.
	*/
	typedef enum { FLOOD=1, STOP } Commands;

	std::vector<std::string> GetCommandsList() const {
		static std::vector<std::string> commands = {"Flood", "Stop"};
		return commands;
	};
	typedef void (ddos::*CommandFunction)();
	CommandFunction GetCommandFunction(Commands command) {
		static const std::vector<CommandFunction> func_vector = {
			&ddos::Flood,
			&ddos::Stop
		};

		// Return at place command-1 because "Commands" starts at 1 and not 0
		return func_vector[command-1];
	};

	void Flood();
	void Stop();

};

}

#endif