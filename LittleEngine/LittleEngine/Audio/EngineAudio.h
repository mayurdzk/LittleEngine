#pragma once
#include "CoreTypes.h"
#include "SFMLAPI/Audio/SFAudioPlayer.h"

namespace LittleEngine
{
class EngineService;

// \brief Handler for all SFX and Music playback
class EngineAudio final
{
private:
	// \brief Helper to store next track until current has faded out
	struct SwitchTrackRequest
	{
		Time fadeTime;
		Fixed targetVolume;
		String newTrackPath;
		bool bFadingOldTrack = false;
		bool bFadingNewTrack = false;

		SwitchTrackRequest(const String& newTrackPath, Time fadeTime, const Fixed& targetVolume)
			: fadeTime(fadeTime), targetVolume(targetVolume), newTrackPath(newTrackPath)
		{
		}
	};

private:
	MusicPlayer m_musicPlayerA;
	MusicPlayer m_musicPlayerB;
	Vec<UPtr<SoundPlayer>> m_sfxPlayers;
	UPtr<SwitchTrackRequest> m_uSwitchTrackRequest;
	bool m_bSideA = true;
	const char* m_szRootMusicDir = "GameMusic";

public:
	EngineAudio();
	~EngineAudio();

	// Returns nullptr if asset could not be loaded
	SoundPlayer* PlaySFX(const String& id,
						 const Fixed& volume = Fixed::One,
						 const Fixed& direction = Fixed::Zero,
						 bool bLoop = false);
	SoundPlayer* PlaySFX(SoundAsset& sound,
						 const Fixed& volume = Fixed::One,
						 const Fixed& direction = Fixed::Zero,
						 bool bLoop = false);
	bool IsSFXPlaying() const;

	// Returns true if asset is loaded successfully
	bool PlayMusic(const String& id, const Fixed& volume = Fixed::One, Time fadeTime = Time::Seconds(1), bool bLoop = true);
	bool IsMusicPlaying() const;
	void StopMusic(Time fadeTime = Time::Zero);
	bool ResumeMusic(Time fadeTime = Time::Zero, const Fixed& volume = Fixed::One);
	void SwitchTrack(const String& id, const Fixed& volume = Fixed::One, Time fadeTime = Time::Seconds(1));
	void SetMusicVolume(const Fixed& volume);

	void PauseAll();
	void ResumeAll();

	// Stop all audio and flush all SFX players
	void Clear(bool bImmediate = true);

private:
	class EngineRepository& GetRepository();
	SoundPlayer& GetOrCreateSFXPlayer();
	MusicPlayer& GetActivePlayer();
	MusicPlayer& GetStandbyPlayer();
	const MusicPlayer& GetActivePlayer() const;
	const MusicPlayer& GetStandbyPlayer() const;

	// Engine to call
	void Tick(Time dt);
	String GetPath(const String& id) const;

	friend class EngineService;
};
} // namespace LittleEngine
