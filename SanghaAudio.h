#pragma once
#include <iostream>
#include <fstream>
#include <faust/audio/jack-dsp.h>
#include <aubio/aubio.h>
#include "utils.h"
#include <tgmath.h>
#include <complex>
#include <fftw3.h>
#include <ableton/link/HostTimeFilter.hpp>
#include <ableton/Link.hpp>

// https://github.com/dgranosa/liveW/blob/master/include/pulsefft.h
enum w_type {
  WINDOW_TRIANGLE,
  WINDOW_HANNING,
  WINDOW_HAMMING,
  WINDOW_BLACKMAN,
  WINDOW_BLACKMAN_HARRIS,
  WINDOW_WELCH,
  WINDOW_FLAT,
};

class SanghaFFT
{
 public:
  SanghaFFT(int length);
  ~SanghaFFT();
	void syncFFTExec();
  void syncSource(float *source, jack_nframes_t nframes);
  float fftBuffer[512];
 private:
  w_type window = WINDOW_HAMMING;
  fftw_plan m_Plan;
  float *m_Weights;
  float** frame_avg_mag;
  unsigned int fft_memb;
  unsigned int frame_avg = 2;
	unsigned int m_FFTLength;
	double *m_In;

  std::complex<double> *m_Out;
};


class SanghaAudio
: public jackaudio_midi
{
 private:
 public:
  SanghaAudio(int length);

  // link stuff
  struct EngineData
  {
    double requestedTempo;
    bool requestStart;
    bool requestStop;
    double quantum;
    bool startStopSyncOn;
  };

  std::mutex mEngineDataGuard;
  double mSampleRate;
  double mSampleTime;
  std::chrono::microseconds mOutputLatency;
  std::chrono::microseconds mTimeAtLastClick;
  EngineData pullEngineData();

  ableton::Link* mLink;
  ableton::link::HostTimeFilter<ableton::link::platform::Clock> mHostTimeFilter;

  EngineData mSharedEngineData;
  EngineData mLockfreeEngineData;


  void updateDsp(dsp* new_dsp);

  std::vector<double> mBuffer;

  bool mIsPlaying;
  void startPlaying();
  void stopPlaying();
  bool isPlaying() const;
  double beatTime() const;
  void setTempo(double tempo);
  double quantum() const;
  void setQuantum(double quantum);
  bool isStartStopSyncEnabled() const;
  void renderMetronomeIntoBuffer(ableton::Link::SessionState sessionState,
                                 double quantum,
                                 std::chrono::microseconds beginHostTime,
                                 std::size_t numSamples);
  void setStartStopSyncEnabled(bool enabled);
  void setBufferSize(std::size_t size);
  void setSampleRate(double sampleRate);

  // fft  / faust stuff
  SanghaFFT* fft;
  void connectPorts();
  int process(jack_nframes_t nframes);

};
