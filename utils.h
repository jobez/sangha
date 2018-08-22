#pragma once
#include <iostream>
#include <fstream>
#include <functional>
#include <sys/stat.h>
#include <string>
#include <map>


typedef int (*voidFunctionType)(int);
typedef std::map< std::string , voidFunctionType > HostProcessSymbolTable;
typedef std::function<std::string(std::string)> Mangler;

voidFunctionType lookupPST(std::string mangledName, Mangler mangler);
void addToPST(std::string name, voidFunctionType fn);
void debug_output_of_array(std::string path, float* ffw_buffer);
std::string get_file_contents(const char *filename);
int file_is_modified(struct stat& file_stat, const char *path, time_t oldMTime);
