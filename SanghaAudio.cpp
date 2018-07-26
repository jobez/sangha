#include "SanghaAudio.h"

// https://github.com/dgranosa/liveW/blob/master/include/pulsefft.h
inline void weights_init(float *dest, int samples, enum w_type w)
{
    switch(w) {
        case WINDOW_TRIANGLE:
            for (int i = 0; i < samples; i++)
                dest[i] = 1 - 2*fabsf((i - ((samples - 1)/2.0f))/(samples - 1));
            break;
        case WINDOW_HANNING:
            for (int i = 0; i < samples; i++)
                dest[i] = 0.5f*(1 - cos((2*M_PI*i)/(samples - 1)));
            break;
        case WINDOW_HAMMING:
            for (int i = 0; i < samples; i++)
                dest[i] = 0.54 - 0.46*cos((2*M_PI*i)/(samples - 1));
            break;
        case WINDOW_BLACKMAN:
            for (int i = 0; i < samples; i++) {
                const float c1 = cos((2*M_PI*i)/(samples - 1));
                const float c2 = cos((4*M_PI*i)/(samples - 1));
                dest[i] = 0.42659 - 0.49656*c1 + 0.076849*c2;
            }
            break;
        case WINDOW_BLACKMAN_HARRIS:
            for (int i = 0; i < samples; i++) {
                const float c1 = cos((2*M_PI*i)/(samples - 1));
                const float c2 = cos((4*M_PI*i)/(samples - 1));
                const float c3 = cos((6*M_PI*i)/(samples - 1));
                dest[i] = 0.35875 - 0.48829*c1 + 0.14128*c2 - 0.001168*c3;
            }
            break;
        case WINDOW_FLAT:
            for (int i = 0; i < samples; i++)
                dest[i] = 1.0f;
            break;
        case WINDOW_WELCH:
            for (int i = 0; i < samples; i++)
                dest[i] = 1 - pow((i - ((samples - 1)/2.0f))/((samples - 1)/2.0f), 2.0f);
            break;
        default:
            for (int i = 0; i < samples; i++)
                dest[i] = 0.0f;
            break;
    }
    float sum = 0.0f;
    for (int i = 0; i < samples; i++)
        sum += dest[i];
    for (int i = 0; i < samples; i++)
        dest[i] /= sum;
}


inline void apply_win(double *dest, float *src,
                      float *weights, int samples)
{
  for (int i = 0; i < samples; i++) {
    dest[i] = src[i]*weights[i];
  }
}


inline float frame_average(float mag, float *buf,
                           int avgs, int no_mod)
{
    if (!avgs)
        return mag;
    float val = mag;
    for (int i = 0; i < avgs; i++)
        val += buf[i];
    val /= avgs + 1;
    if (no_mod)
        return val;
    for (int i = avgs - 1; i > 0; i--)
        buf[i] = buf[i-1];
    buf[0] = mag;
    return val;
}

SanghaFFT::SanghaFFT(int length) :
  m_FFTLength(length),
  m_In(new double[length]),
  m_Weights(new float[length]),
  m_Out(new std::complex<double>[length])
{


  m_Plan = fftw_plan_dft_r2c_1d(m_FFTLength, m_In,
                                reinterpret_cast<fftw_complex*>(m_Out),
                                FFTW_PATIENT | FFTW_DESTROY_INPUT);

  fft_memb = (m_FFTLength / 2)+1;

  frame_avg_mag = (float**)malloc(fft_memb*sizeof(float *));
  for (int i = 0; i < fft_memb; i++)
    frame_avg_mag[i] = (float*)calloc(frame_avg, sizeof(float));

  weights_init(m_Weights, fft_memb, window);

}

SanghaFFT::~SanghaFFT()
{
  delete[] m_In;
  fftw_destroy_plan(m_Plan);
}

void SanghaFFT::syncSource(float *source, jack_nframes_t nframes)
{


  apply_win(m_In, source, m_Weights, fft_memb);


}


void SanghaFFT::syncFFTExec()
{
  fftw_execute(m_Plan);
  double mag_max = 0.0f;
  int start_low = 0;

  for (int i = start_low; i < fft_memb; i++) {
    auto num = m_Out[i];
    double mag = std::real(num)*std::real(num) + std::imag(num)*std::imag(num);
    mag = log10(mag)/10;
    mag = frame_average(mag, frame_avg_mag[i], frame_avg, 1);
    mag_max = mag > mag_max ? mag : mag_max;

  }

  for (int i = start_low; i < fft_memb; i++) {
    auto num = m_Out[i];
    double mag = std::real(num)*std::real(num) + std::imag(num)*std::imag(num);
    mag = log10(mag)/10;
    mag = frame_average(mag, frame_avg_mag[i], frame_avg, 0);
    fftBuffer[i - start_low] = ((float)mag + (float)mag_max + 0.5f) / 2.0f + 0.5f;
  }
}

SanghaAudio::SanghaAudio(int length)

{

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

  fDSP->compute(nframes,
                reinterpret_cast<FAUSTFLOAT**>(fInChannel),
                reinterpret_cast<FAUSTFLOAT**>(fOutChannel));

  fft->syncSource(fOutChannel[1], nframes);

  return 0;
}
