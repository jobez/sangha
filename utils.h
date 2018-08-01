#pragma once
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <string>


void debug_output_of_array(std::string path, float* ffw_buffer);
std::string get_file_contents(const char *filename);
int file_is_modified(struct stat& file_stat, const char *path, time_t oldMTime);
