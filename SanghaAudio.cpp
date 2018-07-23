#include "SanghaAudio.h"


SanghaFFT::SanghaFFT(int length) :
  m_FFTLength(length),
  m_In(new float[length]),
  m_Out(new fftwf_complex[length])
{

  m_Plan = fftwf_plan_dft_r2c_1d(m_FFTLength, m_In, m_Out, FFTW_ESTIMATE);

}

SanghaFFT::~SanghaFFT()
{
  delete[] m_In;
  fftwf_destroy_plan(m_Plan);
}

void SanghaFFT::syncSource(float *source)
{
  unsigned int i;

  for (i=0; i<m_FFTLength; i++)
  {
    m_In[i] = source[i];
  }
}

void SanghaFFT::syncFFTExec(float *out)
{

  fftwf_execute(m_Plan);
  for (unsigned int i=0; i<m_FFTLength; i++)
    {
    out[i] = m_Out[i][0];
  }
}

SanghaAudio::SanghaAudio(int length) {

  fft = new SanghaFFT(length);

}

void SanghaAudio::connectPorts()
{
	auto incudine_ports = jack_get_ports(fClient, "incudine", 0, JackPortIsInput);
	for(int i = 0; i < fOutputPorts.size(); i = i + 1) {
		std::cout << incudine_ports[i] << std::endl;
		std::cout << jack_port_name(fOutputPorts[i]) << std::endl;
		std::cout << jack_connect(fClient, jack_port_name(fOutputPorts[i]), incudine_ports[i]) << std::endl;
	}
}

void SanghaAudio::updateDsp(dsp* new_dsp)
{
	for(int i = 0; i < new_dsp->getNumInputs(); i = i + 1) {
		char buf[256];
		snprintf(buf, 256, "in_%d", i);
		jack_port_unregister(fClient, fInputPorts[i]);
		fInputPorts[i] = jack_port_register(fClient, buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	}
	for(int i = 0; i < new_dsp->getNumOutputs(); i = i + 1) {
		char buf[256];
		snprintf(buf, 256, "out_%d", i);
		jack_port_unregister(fClient, fOutputPorts[i]);
		fOutputPorts[i] = jack_port_register(fClient, buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	}
	dsp* oldDSP = fDSP;
	new_dsp->init(jack_get_sample_rate(fClient));
	fDSP = new_dsp;
	delete oldDSP;
}


int	SanghaAudio::process(jack_nframes_t nframes)
{
  AVOIDDENORMALS;
  // Retrieve JACK inputs/output audio buffers
  float** fInChannel = (float**)alloca(fInputPorts.size() * sizeof(float*));

  for (size_t i = 0; i < fInputPorts.size(); i++) {
    fInChannel[i] = (float*)jack_port_get_buffer(fInputPorts[i], nframes);
  }

  float** fOutChannel = (float**)alloca(fOutputPorts.size() * sizeof(float*));

  for (size_t i = 0; i < fOutputPorts.size(); i++) {
    fOutChannel[i] = (float*)jack_port_get_buffer(fOutputPorts[i], nframes);
  }

  fDSP->compute(nframes, reinterpret_cast<FAUSTFLOAT**>(fInChannel), reinterpret_cast<FAUSTFLOAT**>(fOutChannel));

  fft->syncSource(fOutChannel[0]);

  return 0;
}
