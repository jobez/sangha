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
  s->dsp_manager = {.dspSrcs = SanghaFaust::DspSrcs()};


  SanghaFaust::DspSrc drumSrc{"drums", "./faust/drums.dsp"};
  SanghaFaust::DspSrc oscSrc{"example", "./osc.dsp"};



  s->dsp_manager.dspSrcs.push_back(oscSrc);
  s->dsp_manager.dspSrcs.push_back(drumSrc);
  s->audio_engine = SanghaFaust::init_audio_engine("example",
                                                   join_dsps(s->dsp_manager.dspSrcs));
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
  SanghaFaust::poll_dsp_files(s->dsp_manager, s->audio_engine);
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
