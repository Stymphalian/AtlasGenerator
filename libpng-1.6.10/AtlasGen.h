#pragma once

#include <map>
#include <string>

class AtlasGen {
public:
	AtlasGen();
	virtual ~AtlasGen();
	std::map<std::string, std::string>* parse_arguments(int argc, char** argv);
	void print_usage(std::string name);	
};

