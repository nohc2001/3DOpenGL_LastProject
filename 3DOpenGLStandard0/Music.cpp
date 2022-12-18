#include "Music.h"

int Music::soundUpdate = 0;
int Music::dspUpdate = 0;
FMOD::Sound* Music::SoundData[SOUND_DATA_MAX] = {};
FMOD::Channel* Music::Channels[CHANNEL_MAX] = {};
int Music::channelInSound[CHANNEL_MAX] = {};
FMOD::System* Music::SoundSystem = nullptr;

void Music::Init() {
	if (Music::SoundSystem == nullptr) {
		void* extraDriverData;
		FMOD::System_Create(&Music::SoundSystem);
		Music::SoundSystem->init(32, FMOD_INIT_NORMAL, &extraDriverData);

		for (int index = 0; index < CHANNEL_MAX; ++index) {
			channelInSound[index] = -1;
		}
	}
}

void Music::Release()
{
	if (Music::SoundSystem != nullptr) {
		ClearSoundsAndChannels();
		Music::SoundSystem->release();
	}
}

void Music::Update()
{
	if (Music::SoundSystem != nullptr) {
		Music::SoundSystem->update();
	}
}

int Music::AddSound(const char* fileName, const bool& loopSound, const bool& isStream) {
	FMOD_MODE mod;
	if (loopSound) {
		mod = FMOD_LOOP_NORMAL;
	}
	else {
		mod = FMOD_DEFAULT;
	}

	FMOD_RESULT er;

	if (isStream) {
		er = SoundSystem->createStream(fileName, mod, 0, &SoundData[soundUpdate]);
	}
	else {
		er = SoundSystem->createSound(fileName, mod, 0, &SoundData[soundUpdate]);
	}

	soundUpdate += 1;

	return soundUpdate - 1;
}

void Music::ConnectSound(const int& channelNum, const int& soundID) {
	if (SoundSystem != nullptr && soundID < soundUpdate)
	{
		channelInSound[channelNum] = soundID;
		SoundSystem->playSound(SoundData[soundID], nullptr, true, &Channels[channelNum]);
	}
}

void Music::SetChannelVolume(const int& channelNum, const float& sv_Volume) {
	if (channelInSound[channelNum] != -1 && (0.0f <= sv_Volume && sv_Volume <= 10.0f)) {
		Channels[channelNum]->setVolume(sv_Volume);
	}
}

void Music::SetChannelPos(const int& channelNum, const int& ms_position) {
	if (channelInSound[channelNum] != -1) {
		FMOD_RESULT hr;
		hr = Channels[channelNum]->setPosition(ms_position, FMOD_TIMEUNIT_MS);

		if (hr) {
			int a = 0;
		}
	}
}

void Music::Play(const int& channelNum, bool willplay) {
	if (channelInSound[channelNum] != -1) {
		//ConnectSound(channelNum, channelInSound[channelNum]);
		Channels[channelNum]->setPaused(!willplay);
	}
}

void Music::Stop(const int& channelNum) {
	if (channelInSound[channelNum] != -1) {
		//ConnectSound(channelNum, channelInSound[channelNum]);
		Channels[channelNum]->stop();
	}
}

void Music::PlayOnce(const int& channelNum)
{
	if (channelInSound[channelNum] != -1) {
		ConnectSound(channelNum, channelInSound[channelNum]);
		Channels[channelNum]->setPaused(true);
		Channels[channelNum]->setPosition(0, FMOD_TIMEUNIT_MS);
		Channels[channelNum]->setPaused(false);
	}
}

void Music::SetChannelPan(const int& channelNum, float PanRate) {
	Channels[channelNum]->setPan(PanRate);
}

void Music::ClearSoundsAndChannels() {
	for (int i = 0; i < CHANNEL_MAX; ++i) {
		Play(i, false);
		channelInSound[i] = -1;
	}

	for (int i = 0; i < soundUpdate; ++i) {
		SoundData[i]->release();
		SoundData[i] = nullptr;
	}

	soundUpdate = 0;
}

void Music::ClearSound(const int& index) {
	if (index < soundUpdate) {
		SoundData[index]->release();
		SoundData[index] = nullptr;
	}
}