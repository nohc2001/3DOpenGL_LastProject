#pragma once
#include <fmod.hpp>

class Music {
public:
	//각 체널당 지정된 사운드 id
	//-1 일 경우 아무것도 들어있지 않은 상태이다.
	static constexpr int SOUND_DATA_MAX = 100;
	static FMOD::Sound* SoundData[SOUND_DATA_MAX];
	static int soundUpdate;

	static constexpr int CHANNEL_MAX = 20;
	static FMOD::Channel* Channels[CHANNEL_MAX];

	static constexpr int DSP_MAX = 100;
	static FMOD::DSP* DSPs[DSP_MAX];
	static int dspUpdate;

	static int channelInSound[CHANNEL_MAX];

	static FMOD::System* SoundSystem;

	static void Init();

	static void Release();

	static void Update();

	//fileName : 소리파일의 이름
	//loopSound : true - 소리가 반복된다. false - 한번 소리 난다.
	//isStream : 
	//	true - 길다란 소리일 경우 선호된다. 소리를 파일에서 조금씩 빼와서 쓴다.
	//	false - 효과음 같은 짧은 소리일 경우 선호된다. 소리를 완전히 로드하여 쓴다.	
	static int AddSound(const char* fileName, const bool& loopSound, const bool& isStream);

	static void ConnectSound(const int& channelNum, const int& soundID);

	static void SetChannelVolume(const int& channelNum, const float& sv_Volume);

	static void SetChannelPos(const int& channelNum, const int& ms_position);

	static void Play(const int& channelNum, bool willplay);

	static void Stop(const int& channelNum);

	static void PlayOnce(const int& channelNum);

	static void SetChannelPan(const int& channelNum, float PanRate);

	static void ClearSoundsAndChannels();

	static void ClearSound(const int& index);
};