#pragma once
#include "utils.h"
#include "SanghaFaust.hpp"
#include <ableton/Link.hpp>

/* struct State */
/* { */
/*   std::atomic<bool> running; */

/*   State() */
/*     : running(true) */
/*     , link(120.) */
/*   { */
/*     link.enable(true); */
/*   } */
/* }; */

struct a_state_t {
  SanghaAudio* audio_engine;
  SanghaFaust::dsp_manager_t dsp_manager;
  /* State link;- */
};
