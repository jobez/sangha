#include "api.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>
#include <thread>
#include "libaengine.h"

/* ----------------------------------------------------------------------- */



struct a_state_t * s = NULL;

static void * AppInit() {

  void * state = mmap(0, 256L * 1024L * 1024L * 1024L,
                      PROT_READ |
                      PROT_WRITE,
                      MAP_ANONYMOUS |
                      MAP_PRIVATE |
                      MAP_NORESERVE, -1, 0);

  s = (a_state_t*)state;

  s->audio_engine = SanghaFaust::init_jack("example", SanghaFaust::str_to_dsp("example", (char*)get_file_contents("./osc.dsp").c_str()));
  s->audio_engine->start();
  std::cout << "audio engine" << std::endl;
  printf("Init\n");

  return state;
}


static void AppLoad(void * state) {
  s = (a_state_t*)state;

  printf("Reload\n");

}

static int AppStep(void * state) {
  s = (a_state_t*)state;
  return 0;
}

static int AppStep2(void * state, void * state2) {
  s = (a_state_t*)state;
  // if (s->test) {

  return 0;
}

static void AppUnload(void * state) {

  s = (a_state_t*)state;
  printf("Unload\n");
}

static void AppDeinit(void * state) {
  s = (a_state_t*)state;

  printf("Finalize\n");
  munmap(state, 256L * 1024L * 1024L * 1024L);
}

struct api_t APP_API = {
  .Init   = AppInit,
  .Load   = AppLoad,
  .Step   = AppStep,
  .Step2   = AppStep2,
  .Unload = AppUnload,
  .Deinit = AppDeinit
};
