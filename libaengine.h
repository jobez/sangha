#pragma once
#include "utils.h"
#include "SanghaFaust.hpp"

struct a_state_t {
  SanghaAudio* audio_engine;
  SanghaFaust::dsp_manager_t dsp_manager;
};
