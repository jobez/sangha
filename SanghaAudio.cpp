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
  :
   mSampleRate(44100.)
  , mOutputLatency(0)
  , mSharedEngineData({0., false, false, 4., false})
  , mLockfreeEngineData(mSharedEngineData)
  , mTimeAtLastClick{}
  , mIsPlaying(false)
{
  mLink = new ableton::Link(120.);
  fft = new SanghaFFT(length);
  const double bufferSize = jack_get_buffer_size(fClient);
  const double sampleRate = jack_get_sample_rate(fClient);

  this->setBufferSize(static_cast<std::size_t>(1024));
  this->setSampleRate(sampleRate);

  this->mOutputLatency =
    std::chrono::microseconds(llround(1.0e6 * bufferSize / sampleRate));
}

void SanghaAudio::connectPorts()
{
  // TODO: do not hardcode reconnect port
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

void SanghaAudio::startPlaying()
{
  std::lock_guard<std::mutex> lock(mEngineDataGuard);
  mSharedEngineData.requestStart = true;
}



void SanghaAudio::stopPlaying()
{
  std::lock_guard<std::mutex> lock(mEngineDataGuard);
  mSharedEngineData.requestStop = true;
}

bool SanghaAudio::isPlaying() const
{
  return mLink->captureAppSessionState().isPlaying();
}

double SanghaAudio::beatTime() const
{
  const auto sessionState = mLink->captureAppSessionState();
  return sessionState.beatAtTime(mLink->clock().micros(), mSharedEngineData.quantum);
}

void SanghaAudio::setTempo(double tempo)
{
  std::lock_guard<std::mutex> lock(mEngineDataGuard);
  mSharedEngineData.requestedTempo = tempo;
}

double SanghaAudio::quantum() const
{
  return mSharedEngineData.quantum;
}

void SanghaAudio::setQuantum(double quantum)
{
  std::lock_guard<std::mutex> lock(mEngineDataGuard);
  mSharedEngineData.quantum = quantum;
}

bool SanghaAudio::isStartStopSyncEnabled() const
{
  return mLink->isStartStopSyncEnabled();
}

void SanghaAudio::setStartStopSyncEnabled(const bool enabled)
{
  mLink->enableStartStopSync(enabled);
}

void SanghaAudio::setBufferSize(std::size_t size)
{
  mBuffer = std::vector<double>(size, 0.);
}

void SanghaAudio::setSampleRate(double sampleRate)
{
  mSampleRate = sampleRate;
}

SanghaAudio::EngineData SanghaAudio::pullEngineData()
{
  auto engineData = EngineData{};
  if (mEngineDataGuard.try_lock())
  {
    engineData.requestedTempo = mSharedEngineData.requestedTempo;
    mSharedEngineData.requestedTempo = 0;
    engineData.requestStart = mSharedEngineData.requestStart;
    mSharedEngineData.requestStart = false;
    engineData.requestStop = mSharedEngineData.requestStop;
    mSharedEngineData.requestStop = false;

    mLockfreeEngineData.quantum = mSharedEngineData.quantum;
    mLockfreeEngineData.startStopSyncOn = mSharedEngineData.startStopSyncOn;

    mEngineDataGuard.unlock();
  }
  engineData.quantum = mLockfreeEngineData.quantum;

  return engineData;
}

void SanghaAudio::renderMetronomeIntoBuffer(const ableton::Link::SessionState sessionState,
  const double quantum,
  const std::chrono::microseconds beginHostTime,
  const std::size_t numSamples)
{
  using namespace std::chrono;

  // Metronome frequencies
  static const double highTone = 1567.98;
  static const double lowTone = 1108.73;
  // 100ms click duration
  static const auto clickDuration = duration<double>{0.1};

  // The number of microseconds that elapse between samples
  const auto microsPerSample = 1e6 / mSampleRate;

  for (std::size_t i = 0; i < numSamples; ++i)
  {
    double amplitude = 0.;

    // Compute the host time for this sample and the last.
    const auto hostTime = beginHostTime + microseconds(llround(i * microsPerSample));


    const auto lastSampleHostTime = hostTime - microseconds(llround(microsPerSample));


    // Only make sound for positive beat magnitudes. Negative beat
    // magnitudes are count-in beats.
    if (sessionState.beatAtTime(hostTime, quantum) >= 0.)
    {

      // If the phase wraps around between the last sample and the
      // current one with respect to a 1 beat quantum, then a click
      // should occur.
      if (sessionState.phaseAtTime(hostTime, 1)
          < sessionState.phaseAtTime(lastSampleHostTime, 1))
      {

        mTimeAtLastClick = hostTime;
      }

      const auto secondsAfterClick =
        duration_cast<duration<double>>(hostTime - mTimeAtLastClick);

      // If we're within the click duration of the last beat, render
      // the click tone into this sample
      if (secondsAfterClick < clickDuration)
      {

        // If the phase of the last beat with respect to the current
        // quantum was zero, then it was at a quantum boundary and we
        // want to use the high tone. For other beats within the
        // quantum, use the low tone.
        const auto freq =
          floor(sessionState.phaseAtTime(hostTime, quantum)) == 0 ? highTone : lowTone;

        // Simple cosine synth
        amplitude = cos(2 * M_PI * secondsAfterClick.count() * freq)
                    * (1 - sin(5 * M_PI * secondsAfterClick.count()));
      }
    }

    mBuffer[i] = amplitude;
  }
}


int	SanghaAudio::process(jack_nframes_t nframes)
{

  using namespace std::chrono;
  // AudioEngine& engine = mEngine;

  const auto _hostTime = mHostTimeFilter.sampleTimeToHostTime(mSampleTime);

  mSampleTime += nframes;

  const auto hostTime = _hostTime + mOutputLatency;

  const std::size_t numSamples = nframes;
  const auto engineData = pullEngineData();

  auto sessionState = mLink->captureAudioSessionState();

  std::fill(mBuffer.begin(), mBuffer.end(), 0);

  if (engineData.requestStart)
  {
    sessionState.setIsPlaying(true, hostTime);
  }

  if (engineData.requestStop)
  {
    sessionState.setIsPlaying(false, hostTime);
  }

  if (!mIsPlaying && sessionState.isPlaying())
  {
    // Reset the timeline so that beat 0 corresponds to the time when transport starts
    sessionState.requestBeatAtStartPlayingTime(0, engineData.quantum);
    mIsPlaying = true;
  }
  else if (mIsPlaying && !sessionState.isPlaying())
  {
    mIsPlaying = false;
  }

  if (engineData.requestedTempo > 0)
  {
    // Set the newly requested tempo from the beginning of this buffer
    sessionState.setTempo(engineData.requestedTempo, hostTime);
  }

  // Timeline modifications are complete, commit the results
  mLink->commitAudioSessionState(sessionState);

  if (mIsPlaying)
  {
    // As long as the engine is playing, generate metronome clicks in
    // the buffer at the appropriate beats.
    renderMetronomeIntoBuffer(sessionState, engineData.quantum, hostTime, numSamples);


    for (int k = 0; k < 2; ++k)
  {
    float* buffer = static_cast<float*>(jack_port_get_buffer(fOutputPorts[k], nframes));
    for (unsigned long i = 0; i < nframes; ++i) {
      buffer[i] = static_cast<float>(mBuffer[i]);
  }
  }
  }

  // std::cout << mIsPlaying << std::endl;












  // AVOIDDENORMALS;
  // // Retrieve JACK inputs/output audio buffers
  // float** fInChannel = (float**)alloca(fInputPorts.size() * sizeof(float*));

  // for (size_t i = 0; i < fInputPorts.size(); i++) {
  //   fInChannel[i] = (float*)jack_port_get_buffer(fInputPorts[i], nframes);
  // }

  // float** fOutChannel = (float**)alloca(fOutputPorts.size() * sizeof(float*));

  // for (size_t i = 0; i < fOutputPorts.size(); i++) {
  //   fOutChannel[i] = (float*)jack_port_get_buffer(fOutputPorts[i], nframes);
  // }

  // fDSP->compute(nframes,
  //               reinterpret_cast<FAUSTFLOAT**>(fInChannel),
  //               reinterpret_cast<FAUSTFLOAT**>(fOutChannel));


  // fft->syncSource(fOutChannel[1], nframes);

  return 0;
}
