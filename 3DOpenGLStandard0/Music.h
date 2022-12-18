#pragma once
#include <fmod.hpp>

class Music {
public:
	//�� ü�δ� ������ ���� id
	//-1 �� ��� �ƹ��͵� ������� ���� �����̴�.
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

	//fileName : �Ҹ������� �̸�
	//loopSound : true - �Ҹ��� �ݺ��ȴ�. false - �ѹ� �Ҹ� ����.
	//isStream : 
	//	true - ��ٶ� �Ҹ��� ��� ��ȣ�ȴ�. �Ҹ��� ���Ͽ��� ���ݾ� ���ͼ� ����.
	//	false - ȿ���� ���� ª�� �Ҹ��� ��� ��ȣ�ȴ�. �Ҹ��� ������ �ε��Ͽ� ����.	
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