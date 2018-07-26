#pragma once
#include <faust/dsp/llvm-dsp.h>
#include "SanghaAudio.h"
#include "utils.h"

namespace SanghaFaust {

  struct dsp_t {
    std::string name;
    std::string filename;
    time_t last_mod = 0;
    dsp_t(std::string filename, std::string name) :
      filename(filename),
      name(name)
  {
  }
  };

  struct dsp_manager_t {
    std::vector<dsp_t> dsp_files;
  };

  dsp* str_to_dsp (std::string dsp_name, std::string dsp_str);
  SanghaAudio* init_jack(char *caudio_name, dsp* dsp);
  void poll_dsp_files(dsp_manager_t& dsp_manager, SanghaAudio* audio);

}
