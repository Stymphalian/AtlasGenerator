#include "AtlasGen.h"

#include <cstdio>
#include <string>


AtlasGen::AtlasGen()
{}
AtlasGen::~AtlasGen()
{}

void AtlasGen::print_usage(std::string name){
	printf("Usage: %s\n", name);
	printf("inputfile = [required]\n");
	printf("width = number[required]\n");
	printf("height = number[reqiured]\n");
	printf("outputDirectory = outputDirectory[default = . / output]\n");
	printf("atlasfilename = out.atlas[default = out.atlas]\n");
	printf("pngfilename = out.png[default = out.png]\n");
}

std::map<std::string, std::string>* AtlasGen::parse_arguments(int argc, char** argv){
	static char* required[3] = {
		"inputfile",
		"width",
		"height",
	};
	static char* optional[3] = {
		"outputDirectory",
		"atlasfilename",
		"pngfilename"
	};
	std::map<std::string, std::string>* map = new std::map<std::string, std::string>;

	// check for arguments
	if(argc <= 1){ return NULL; }

	// iterate through all the possible arguments and add them to the hash map
	// argument format  -<arg_name>=<arg_value>
	size_t temp_pivot;
	std::string temp;

	std::string name;
	std::string value;
	for(int i = 0; i < argc; i++){
		temp = argv[i];

		// make sure that it has a valid format..
		temp_pivot = temp.find_first_of('=');
		if(temp_pivot == std::string::npos){
			continue;
		}

		// add the key-value pair
		name = temp.substr(0, temp_pivot);
		value = temp.substr(temp_pivot + 1);
		(*map)[name] = value;
	}

	// check for required parameters		
	for(int i = 0; i < 3; ++i){
		if(map->find(required[i]) == map->end()){
			delete map;
			return NULL;
		}
	}

	// defaults for some arugments.
	if(map->find("outputDirectory") == map->end()){
		(*map)["outputDirectory"] = "./";
	}
	if(map->find("atlasfilename") == map->end()){
		(*map)["atlasfilename"] = "out.atlas";
	}
	if(map->find("pngfilename") == map->end()){
		(*map)["pngfilename"] = "out.png";
	}

	std::map<std::string, std::string>::iterator it;
	for(it = map->begin(); it != map->end(); ++it){
		printf("%s --> %s\n", it->first.c_str(), it->second.c_str());
	}

	return map;
}
