#include "SanghaAudio.h"

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
