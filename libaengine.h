#pragma once
#include "utils.h"
#include "SanghaFaust.hpp"
#include <ableton/Link.hpp>

struct a_state_t {
  SanghaAudio* audio_engine;
  SanghaFaust::DspManager dsp_manager;
};
