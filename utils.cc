#include "utils.h"

HostProcessSymbolTable hostProcessSymbolTable;

void addToPST(std::string name, voidFunctionType fn) {

  hostProcessSymbolTable[name] = fn;

}

void debug_output_of_array(std::string path, float* ffw_buffer) {


  std::ofstream output_file (path);
  if (output_file.is_open())
  {

    for(int count = 0; count < 512; count++){
      output_file << ffw_buffer[count] << std::endl;
    }
    output_file.close();
  }
}

std::string get_file_contents(const char *filename)
{
  std::FILE *fp = std::fopen(filename, "rb");
  if (fp)
    {
      std::string contents;
      std::fseek(fp, 0, SEEK_END);
      contents.resize(std::ftell(fp));
      std::rewind(fp);
      std::fread(&contents[0], 1, contents.size(), fp);
      std::fclose(fp);
      return(contents);
    }
  throw(errno);
}

int file_is_modified(struct stat& file_stat, const char *path, time_t oldMTime) {
  int err = stat(path, &file_stat);

  if (err != 0) {
    perror(" [file_is_modified] stat");
    exit(errno);
  }
  return file_stat.st_mtime > oldMTime;
}


voidFunctionType lookupPST(std::string mangledName, Mangler mangler) {

  for( const auto& pair : hostProcessSymbolTable )
    {
      if(mangledName == mangler(pair.first)) {
        return pair.second;
      }
    }

  return nullptr;

}
