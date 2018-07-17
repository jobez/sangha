#include "api.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>
#include <thread>
#include "SanghaFaust.hpp"
/* ----------------------------------------------------------------------- */

struct state_t {
  SanghaAudio* audio_engine;
};

struct state_t * s = NULL;

static void * AppInit() {

  void * state = mmap(0, 256L * 1024L * 1024L * 1024L,
                      PROT_READ |
                      PROT_WRITE,
                      MAP_ANONYMOUS |
                      MAP_PRIVATE |
                      MAP_NORESERVE, -1, 0);

  s = (state_t*)state;
  s->audio_engine = SanghaFaust::init_jack("example", SanghaFaust::str_to_dsp("example", "process = 1;"));
  s->audio_engine->start();
  std::cout << "audio engine" << std::endl;
  printf("Init\n");

  return state;
}


static void AppLoad(void * state) {
  s = (state_t*)state;

  printf("Reload\n");

}

static int AppStep(void * state) {
  s = (state_t*)state;
  return 0;
}

static void AppUnload(void * state) {

  s = (state_t*)state;
  printf("Unload\n");
}

static void AppDeinit(void * state) {
  s = (state_t*)state;

  printf("Finalize\n");
  munmap(state, 256L * 1024L * 1024L * 1024L);
}

struct api_t APP_API = {
  .Init   = AppInit,
  .Load   = AppLoad,
  .Step   = AppStep,
  .Unload = AppUnload,
  .Deinit = AppDeinit
};
