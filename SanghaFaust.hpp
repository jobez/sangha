#pragma once
#include <faust/dsp/llvm-dsp.h>
#include "SanghaAudio.h"
namespace SanghaFaust {

  dsp* str_to_dsp (char *cdsp_name, char *cdsp_str);
  SanghaAudio* init_jack(char *caudio_name, dsp* dsp);

}
