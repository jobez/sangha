#pragma once
#include <faust/audio/jack-dsp.h>

class SanghaAudio
 : public jackaudio_midi
{
public:

	void updateDsp(dsp* new_dsp);

	void connectPorts();
};
