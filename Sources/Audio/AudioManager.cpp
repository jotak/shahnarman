// -----------------------------------------------------------------
// Name : AudioManager
// -----------------------------------------------------------------
#include "AudioManager.h"
#include "../LocalClient.h"
#include "../Debug/DebugManager.h"
#include "../Data/Parameters.h"

#define BUFFER_SIZE   131072     // 128 KB buffers

AudioManager * AudioManager::mInst = NULL;

size_t ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
int ov_seek_func(void *datasource, ogg_int64_t offset, int whence);
int ov_close_func(void *datasource);
long ov_tell_func(void *datasource);

// -----------------------------------------------------------------
// Name : AudioManager
// -----------------------------------------------------------------
AudioManager::AudioManager()
{
  m_pLocalClient = NULL;
  m_pDebug = NULL;
  m_pDevice = NULL;
  m_pContext = NULL;
  m_State = Unset;
  m_iCurrentMusic = -1;
  m_uSourceID = 0;
  m_uBuffersID[0] = m_uBuffersID[1] = 0;
  m_pOggStream = NULL;
  for (int i = 0; i < NB_MUSICS; i++)
  {
    wsafecpy(m_sAllMusicFiles[i], MAX_PATH, L"");
    m_iChainedMusics[i] = -1;
  }
  for (int i = 0; i < NB_SOUNDS; i++)
  {
    m_iAllSoundBuffers[i] = -1;
    m_iAllSoundSources[i] = -1;
  }
}

// -----------------------------------------------------------------
// Name : ~AudioManager
// -----------------------------------------------------------------
AudioManager::~AudioManager()
{
  if (m_uBuffersID[0] > 0)
    alDeleteBuffers(1, m_uBuffersID);
  if (m_uSourceID > 0)
    alDeleteSources(1, &m_uSourceID);
  for (int i = 0; i < NB_SOUNDS; i++)
  {
    if (m_iAllSoundBuffers[i] >= 0)
      alDeleteBuffers(1, &(m_iAllSoundBuffers[i]));
    if (m_iAllSoundSources[i] >= 0)
      alDeleteSources(1, &(m_iAllSoundSources[i]));
  }
  alcMakeContextCurrent(NULL);
  if (m_pContext != NULL)
    alcDestroyContext(m_pContext);
  if (m_pDevice != NULL)
    alcCloseDevice(m_pDevice);
  if (m_pOggStream != NULL)
    ov_clear(m_pOggStream);
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void AudioManager::Init(LocalClient * pLocalClient)
{
  m_pLocalClient = pLocalClient;
  m_pDebug = pLocalClient->getDebug();
  // Initialize OpenAL
  m_pDevice = alcOpenDevice(NULL);  // use default system device
  if (m_pDevice == NULL)
  {
    m_pDebug->notifyErrorMessage(L"Error in AudioManager: can't initialize device");
    return;
  }
  m_pContext = alcCreateContext(m_pDevice, NULL);
  if (m_pContext == NULL)
  {
    m_pDebug->notifyErrorMessage(L"Error in AudioManager: can't initialize context");
    return;
  }
  alcMakeContextCurrent(m_pContext);
  initMusicFiles();
  initSoundData();
  updateVolume();

  m_State = Stopped;
}

// -----------------------------------------------------------------
// Name : initMusicFiles
// -----------------------------------------------------------------
void AudioManager::initMusicFiles()
{
  wsafecpy(m_sAllMusicFiles[MUSIC_INTRO], MAX_PATH, L"LMK-Prologue");
  wsafecpy(m_sAllMusicFiles[MUSIC_INGAME1], MAX_PATH, L"LMK-LaSerenissima");
  wsafecpy(m_sAllMusicFiles[MUSIC_INGAME2], MAX_PATH, L"LMK-TheMummersDance");
  m_iChainedMusics[MUSIC_INGAME1] = MUSIC_INGAME2;
  m_iChainedMusics[MUSIC_INGAME2] = MUSIC_INGAME1;
}

// -----------------------------------------------------------------
// Name : initSoundData
// -----------------------------------------------------------------
void AudioManager::initSoundData()
{
  readOggSound(L"click", SOUND_CLICK);
  readOggSound(L"cast", SOUND_CAST_SPELL);
  readOggSound(L"cancelspell", SOUND_CANCEL_SPELL);
  readOggSound(L"mapclick", SOUND_MAP_CLICK);
}

// -----------------------------------------------------------------
// Name : readOggSound
// -----------------------------------------------------------------
bool AudioManager::readOggSound(wchar_t * sName, int iSound)
{
  // Decode Ogg file
  // Open for binary reading
  wchar_t sFilePath[MAX_PATH] = GAME_SOUNDS_PATH;
  wsafecat(sFilePath, MAX_PATH, sName);
  wsafecat(sFilePath, MAX_PATH, L".ogg");
  FILE * f = NULL;
  errno_t err = wfopen(&f, sFilePath, L"rb");
  if (err != 0)
  {
    switch (err)
    {
    case ENOENT:
      {
        wchar_t sError[1024];
        swprintf_s(sError, 1024, L"Can't read music, file %s not found", sName);
        m_pDebug->notifyErrorMessage(sError);
        return false;
      }
    default:
      {
        wchar_t sError[1024];
        swprintf_s(sError, 1024, L"Can't read music, error on reading (%s)", sName);
        m_pDebug->notifyErrorMessage(sError);
        return false;
      }
    }
  }
  OggVorbis_File oggStream;
	ov_callbacks sCallbacks;
  sCallbacks.read_func = ov_read_func;
  sCallbacks.seek_func = ov_seek_func;
  sCallbacks.close_func = ov_close_func;
  sCallbacks.tell_func = ov_tell_func;
  int result = ov_open_callbacks(f, &oggStream, NULL, 0, sCallbacks);
  if (result < 0)
  {
    fclose(f);
    wchar_t sError[1024];
    swprintf_s(sError, 1024, L"Can't read music file %s, can't read ogg", sName);
    m_pDebug->notifyErrorMessage(sError);
    return false;
  }

  // Retrieve info
  vorbis_info * pInfo = ov_info(&oggStream, -1);
  ALenum format;
  if (pInfo->channels == 1)
    format = AL_FORMAT_MONO16;
  else
    format = AL_FORMAT_STEREO16;
  ALuint freq = pInfo->rate;

  // Prepare buffer and source
  if (m_iAllSoundBuffers[iSound] > 0)
  {
    alDeleteBuffers(1, &(m_iAllSoundBuffers[iSound]));
    m_iAllSoundBuffers[iSound] = 0;
  }
  alGenBuffers(1, &(m_iAllSoundBuffers[iSound]));
  if (m_iAllSoundSources[iSound] > 0)
  {
    alDeleteSources(1, &(m_iAllSoundSources[iSound]));
    m_iAllSoundSources[iSound] = 0;
  }
  alGenSources(1, &(m_iAllSoundSources[iSound]));

  // Fill buffer
  char buffer[BUFFER_SIZE];
  char * totalBuffer = NULL;
  long bytes = 0;
  long total = 0;
  int bitstream;
  do {
    bytes = ov_read(&oggStream, buffer, BUFFER_SIZE, 0, 2, 1, &bitstream);
    if (bytes > 0)
    {
      if (totalBuffer == NULL)
      {
        totalBuffer = new char[bytes];
        memcpy(totalBuffer, buffer, bytes);
        total = bytes;
      }
      else
      {
        char * tmp = new char[total + bytes];
        memcpy(tmp, totalBuffer, total);
        memcpy(&(tmp[total]), buffer, bytes);
        delete[] totalBuffer;
        totalBuffer = tmp;
        total += bytes;
      }
    }
  } while (bytes > 0);
  ov_clear(&oggStream);

  // If bytes is negative, error
  if (bytes < 0)
  {
    if (totalBuffer != NULL)
      delete[] totalBuffer;
    wchar_t sError[1024];
    swprintf_s(sError, 1024, L"Can't read content of ogg file %s", sName);
    m_pDebug->notifyErrorMessage(sError);
    return false;
  }

  // If we got data, upload it to OpenAL buffer and attach it to its source
  if (totalBuffer != NULL)
  {
    alBufferData(m_iAllSoundBuffers[iSound], format, totalBuffer, total, freq);
    alSourcei(m_iAllSoundSources[iSound], AL_BUFFER, m_iAllSoundBuffers[iSound]);
    delete[] totalBuffer;
  }
  return true;
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void AudioManager::Update(double delta)
{
  // Query the state of the souce
  if (m_State == Playing)
  {
    ALint state;
    bool active = true;
    alGetSourcei(m_uSourceID, AL_BUFFERS_PROCESSED, &state);
    while (state--)
    {
      ALuint buffer;
      alSourceUnqueueBuffers(m_uSourceID, 1, &buffer);
      active = stream(buffer);
      alSourceQueueBuffers(m_uSourceID, 1, &buffer);
    }
    if (!active)
    {
      if (m_iCurrentMusic >= 0 && m_iChainedMusics[m_iCurrentMusic] >= 0)
        playMusic(m_iChainedMusics[m_iCurrentMusic]);
      else
        stopMusic();
    }
  }
}

size_t ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return fread(ptr, size, nmemb, (FILE*)datasource);
}

int ov_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	return fseek((FILE*)datasource, (long)offset, whence);
}

int ov_close_func(void *datasource)
{
   return fclose((FILE*)datasource);
}

long ov_tell_func(void *datasource)
{
	return ftell((FILE*)datasource);
}

// -----------------------------------------------------------------
// Name : playMusic
// -----------------------------------------------------------------
void AudioManager::playMusic(int iMusicId)
{
  if (m_State == Unset || iMusicId == m_iCurrentMusic || iMusicId < 0 || iMusicId >= NB_MUSICS)
    return;

  if (m_State == Playing)
  {
    stopMusic();
    m_State = Stopped;
  }
  if (m_pLocalClient->getClientParameters()->iMusicVolume == 0) // mute
    return;

  m_iCurrentMusic = iMusicId;

  // Decode Ogg file
  // Open for binary reading
  wchar_t sFilePath[MAX_PATH] = GAME_MUSICS_PATH;
  wsafecat(sFilePath, MAX_PATH, m_sAllMusicFiles[iMusicId]);
  wsafecat(sFilePath, MAX_PATH, L".ogg");
  FILE * f = NULL;
  errno_t err = wfopen(&f, sFilePath, L"rb");
  if (err != 0)
  {
    switch (err)
    {
    case ENOENT:
      {
        wchar_t sError[1024];
        swprintf_s(sError, 1024, L"Can't read music, file %s not found", m_sAllMusicFiles[iMusicId]);
        m_pDebug->notifyErrorMessage(sError);
        return;
      }
    default:
      {
        wchar_t sError[1024];
        swprintf_s(sError, 1024, L"Can't read music, error on reading (%s)", m_sAllMusicFiles[iMusicId]);
        m_pDebug->notifyErrorMessage(sError);
        return;
      }
    }
  }
  m_State = Playing;
  if (m_pOggStream != NULL)
  {
    ov_clear(m_pOggStream);
    delete m_pOggStream;
  }
  m_pOggStream = new OggVorbis_File();
	ov_callbacks sCallbacks;
  sCallbacks.read_func = ov_read_func;
  sCallbacks.seek_func = ov_seek_func;
  sCallbacks.close_func = ov_close_func;
  sCallbacks.tell_func = ov_tell_func;
  int result = ov_open_callbacks(f, m_pOggStream, NULL, 0, sCallbacks);
  if (result < 0)
  {
    fclose(f);
    wchar_t sError[1024];
    swprintf_s(sError, 1024, L"Can't read music file %s, can't read ogg", m_sAllMusicFiles[iMusicId]);
    m_pDebug->notifyErrorMessage(sError);
    return;
  }

  // Retrieve info
  vorbis_info * pInfo = ov_info(m_pOggStream, -1);
  if (pInfo->channels == 1)
    m_Format = AL_FORMAT_MONO16;
  else
    m_Format = AL_FORMAT_STEREO16;
  m_iFrequency = pInfo->rate;

  // Upload sound data to buffer and attach sound
  if (m_uBuffersID[0] > 0)
  {
    alDeleteBuffers(1, m_uBuffersID);
    m_uBuffersID[0] = m_uBuffersID[1] = 0;
  }
  alGenBuffers(2, m_uBuffersID);
  if (m_uSourceID > 0)
  {
    alDeleteSources(1, &m_uSourceID);
    m_uSourceID = 0;
  }
  alGenSources(1, &m_uSourceID);
  float fVol = (float)(m_pLocalClient->getClientParameters()->iMusicVolume) / 10.0f;
  if (m_uSourceID >= 0)
    alSourcef(m_uSourceID, AL_GAIN, fVol);

  if (!stream(m_uBuffersID[0]))
    return;

  if (!stream(m_uBuffersID[1]))
    return;

  alSourceQueueBuffers(m_uSourceID, 2, m_uBuffersID);
  alSourcePlay(m_uSourceID);
}

// -----------------------------------------------------------------
// Name : stopMusic
// -----------------------------------------------------------------
void AudioManager::stopMusic()
{
  if (m_uSourceID > 0)
  {
    alSourceStop(m_uSourceID);
    empty();
    alDeleteSources(1, &m_uSourceID);
    alDeleteBuffers(1, m_uBuffersID);
    m_uSourceID = 0;
    m_uBuffersID[0] = m_uBuffersID[1] = 0;
  }
  if (m_pOggStream != NULL)
  {
    ov_clear(m_pOggStream);
    m_pOggStream = NULL;
  }
  m_State = Stopped;
  m_iCurrentMusic = -1;
}

// -----------------------------------------------------------------
// Name : stream
// -----------------------------------------------------------------
bool AudioManager::stream(ALuint buffer)
{
  char data[BUFFER_SIZE];
  int size = 0;
  int section;
  int result;

  while (size < BUFFER_SIZE)
  {
    result = ov_read(m_pOggStream, data + size, BUFFER_SIZE - size, 0, 2, 1, &section);

    if (result > 0)
      size += result;
    else
    {
      if (result < 0)
      {
        m_pDebug->notifyErrorMessage(L"Can't read music stream");
        return false;
      }
      else
        break;
    }
  }
    
  if (size == 0)
      return false;

  alBufferData(buffer, m_Format, data, size, m_iFrequency);
  return true;
}

// -----------------------------------------------------------------
// Name : empty
// -----------------------------------------------------------------
void AudioManager::empty()
{
  int queued;
  alGetSourcei(m_uSourceID, AL_BUFFERS_QUEUED, &queued);
  while (queued--)
  {
    ALuint buffer;
    alSourceUnqueueBuffers(m_uSourceID, 1, &buffer);
  }
}

// -----------------------------------------------------------------
// Name : playSound
// -----------------------------------------------------------------
void AudioManager::playSound(int iSoundId)
{
  if (m_pLocalClient->getClientParameters()->iSoundVolume == 0) // mute
    return;
  if (iSoundId < 0 || iSoundId >= NB_SOUNDS)
    return;

  alSourcePlay(m_iAllSoundSources[iSoundId]);
}

// -----------------------------------------------------------------
// Name : updateVolume
// -----------------------------------------------------------------
void AudioManager::updateVolume()
{
  float fVol = (float)(m_pLocalClient->getClientParameters()->iSoundVolume) / 10.0f;
  for (int i = 0; i < NB_SOUNDS; i++)
  {
    if (m_iAllSoundSources[i] >= 0)
      alSourcef(m_iAllSoundSources[i], AL_GAIN, fVol);
  }
  fVol = (float)(m_pLocalClient->getClientParameters()->iMusicVolume) / 10.0f;
  if (m_uSourceID >= 0)
    alSourcef(m_uSourceID, AL_GAIN, fVol);
}
