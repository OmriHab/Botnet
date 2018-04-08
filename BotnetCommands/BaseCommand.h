#ifndef BASECOMMAND_H
#define BASECOMMAND_H

namespace botnet {

#include <iostream>

class BaseCommand {
private:
	std::string name;
public:
	BaseCommand();
	~BaseCommand();

	void GetName() const {
		return this->name;
	}

	virtual void PassCtrl() = 0;
	
	virtual void PrintOptions() = 0;

};


}


#endif