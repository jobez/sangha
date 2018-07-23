#pragma once
#include <faust/audio/jack-dsp.h>
#include <aubio/aubio.h>
#include <fftw3.h>

#define NUM_POINTS 64

/* typedef float sample; */
/* const size_t buffer_max = 16384; */
/* size_t buffer_size, buffer_jack_size, buffer_wnd_size, wnd_width; */
/* sample *buffer_1, *buffer_2, *buffer_input; */
/* bool buffer_locked, buffer_size_changed; */

/* int nelements_in = 1024; */
/* int nelements_out = (nelements_in / 2) + 1; */
/* in = (double*) fftw_malloc(sizeof(double) * nelements_in); */
/* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nelements_out); */
/* plan = fftw_plan_dft_r2c_1d(n, in, out, FFTW_MEASURE); */

class SanghaFFT
{
 public:
  SanghaFFT(int length);
  ~SanghaFFT();
	void syncFFTExec(float *out);
  void syncSource(float *source);
 private:
  fftwf_plan m_Plan;
	unsigned int m_FFTLength;
	float *m_In;
  fftwf_complex *m_Out;
};


class SanghaAudio
 : public jackaudio_midi
{
 private:
 public:
  SanghaAudio(int length);
  void updateDsp(dsp* new_dsp);
  SanghaFFT* fft;
  void connectPorts();
  int process(jack_nframes_t nframes);
};
