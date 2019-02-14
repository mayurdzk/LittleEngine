#pragma once
#include "CoreTypes.h"
#include "SFMLAPI/SFSystem.h"
#include "SFML/Audio.hpp"

namespace LittleEngine
{
// \brief Base class for audio Asset playback
class AudioPlayer
{
public:
	enum class Status
	{
		NoMedia,
		Stopped,
		Playing,
		Paused
	};

public:
	Fixed m_volume = Fixed(80, 100);
	Status m_status;
	bool m_bLooping;

public:
	virtual ~AudioPlayer();
	virtual void Play() = 0;
	virtual void Stop() = 0;
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual void Reset(Time time = Time::Zero) = 0;
	virtual bool IsPlaying() const = 0;
	virtual void Tick(Time dt) = 0;

protected:
	AudioPlayer();
	Status Cast(sf::Sound::Status status);
	virtual bool ApplyParams() = 0;
};

// \brief Concrete class for Sound playback (uses pre-loaded SoundAsset)
class SoundPlayer : public AudioPlayer
{
private:
	sf::Sound m_sfSound;
	SoundAsset* m_pSoundAsset = nullptr;

public:
	SoundPlayer(SoundAsset* pSoundAsset = nullptr);
	~SoundPlayer();

	bool SetSoundAsset(SoundAsset& soundAsset);
	void SetDirection(const Fixed& direction);

	virtual void Play() override;
	virtual void Stop() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void Reset(Time time = Time::Zero) override;
	virtual bool IsPlaying() const override;
	virtual void Tick(Time dt) override;

private:
	virtual bool ApplyParams() override;
};

// \brief Concrete class for Music playback (uses streamed MusicAsset)
class MusicPlayer : public AudioPlayer
{
private:
	Time m_fadeTime;
	Time m_elapsedTime;
	Fixed m_targetVolume = Fixed::One;
	Fixed m_startVolume = Fixed::One;
	GameClock m_clock;
	MusicAsset* m_pMainTrack = nullptr;
	bool m_bFadingIn = false;
	bool m_bFadingOut = false;

public:
	MusicPlayer(MusicAsset* pMusicAsset = nullptr);
	~MusicPlayer();

	bool SetTrack(MusicAsset& track);
	bool HasTrack() const;
	Time GetDuration() const;
	Time GetElapsed() const;
	bool IsFading() const;
	void FadeIn(Time time, const Fixed& targetVolume = Fixed::One);
	void FadeOut(Time time, const Fixed& targetVolume = Fixed::Zero);
	void EndFade();

	virtual void Play() override;
	virtual void Stop() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void Reset(Time time = Time::Zero) override;
	virtual bool IsPlaying() const override;
	virtual void Tick(Time dt) override;

private:
	void BeginFade();

	virtual bool ApplyParams() override;

	friend class EngineAudio;
};
} // namespace LittleEngine
