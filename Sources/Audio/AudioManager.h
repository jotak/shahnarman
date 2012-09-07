#ifndef _AUDIOMANAGER_H
#define _AUDIOMANAGER_H

#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>
#include "../SystemHeaders.h"
#include "../utils.h"

#define MUSIC_INTRO     0
#define MUSIC_INGAME1   1
#define MUSIC_INGAME2   2
#define NB_MUSICS       3

#define SOUND_CLICK           0
#define SOUND_CAST_SPELL      1
#define SOUND_CANCEL_SPELL    2
#define SOUND_MAP_CLICK       3
#define NB_SOUNDS             4

class LocalClient;
class DebugManager;

enum MusicState {
  Unset = 0,
  Stopped,
  Playing
};

class AudioManager
{
public:
  // Constructor / destructor
  ~AudioManager();
  static AudioManager * getInstance() { if (mInst == NULL) mInst = new AudioManager(); return mInst; };

  // Manager functions
  void Init(LocalClient * m_pLocalClient);
  void Update(double delta);

  // Specific functions
  void playMusic(int iMusicId);
  void stopMusic();
  void playSound(int iSoundId);
  void updateVolume();

private:
	AudioManager();
  static AudioManager * mInst;

  void initMusicFiles();
  void initSoundData();
  bool readOggSound(const wchar_t * sName, int iSound);
  bool stream(ALuint buffer);
  void empty();

  LocalClient * m_pLocalClient;
  DebugManager * m_pDebug;
  ALCcontext * m_pContext;
  ALCdevice * m_pDevice;
  ALuint m_iAllSoundBuffers[NB_SOUNDS];
  ALuint m_iAllSoundSources[NB_SOUNDS];
  wchar_t m_sAllMusicFiles[NB_MUSICS][MAX_PATH];
  int m_iChainedMusics[NB_MUSICS];
  int m_iCurrentMusic;
  ALuint m_uBuffersID[2];  // Front & back buffers
  ALuint m_uSourceID;
  MusicState m_State;
  OggVorbis_File * m_pOggStream;
  ALuint m_iFrequency;
  ALenum m_Format;
};

#endif
