#pragma once
#include <faust/dsp/llvm-dsp.h>
#include "SanghaAudio.h"
#include <tuple>
#include "utils.h"

// what is the relationship between a chain of dsps and a track/instrument?
// track = <instrument, notes/beat> enact(track, beat)
// instrument x rack



namespace SanghaFaust {

  struct DspSrc {
    std::string name;
    std::string filename;
    time_t last_mod = 0;
  };

  // typedef std::tuple<DspSrc, Dsp*> ReloadableDsp;
  // typedef std::vector<ReloadableDsp> ReloadableDsps;
  typedef std::vector<dsp*> Dsps;
  typedef std::vector<DspSrc> DspSrcs;


  struct DspManager {
    DspSrcs dspSrcs;
  };


  dsp* join_dsps(DspSrcs dsps);
  dsp* str_to_dsp (std::string dsp_name, std::string dsp_str);
  SanghaAudio* init_audio_engine(char *caudio_name, dsp* dsp);
  void poll_dsp_files(DspManager& dsp_manager, SanghaAudio* audio);
}
