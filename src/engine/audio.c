#define CHANNEL_MAX 4


typedef struct {
  SDL_AudioSpec spec;
  uint32_t length;
  char name[256];
  uint8_t* buffer;
} AUDIO_DATA;

typedef struct {
  bool enabled;
  bool requestEnabled;
  int16_t channelId;
  uint32_t position;
  uint8_t volume;
  AUDIO_DATA* audio;
} AUDIO_CHANNEL;

typedef struct {
  SDL_AudioDeviceID deviceId;
  uint8_t* outputBuffer;
  SDL_AudioSpec spec;
  int16_t audioScale;
  AUDIO_CHANNEL* channels[CHANNEL_MAX];
} AUDIO_ENGINE;

const uint16_t bytesPerSample = 2;
const uint16_t channels = 2;


// audio callback function
// here you have to copy the data of your audio buffer into the
// requesting audio buffer (stream)
// you should only copy as much as the requested length (len)
void AUDIO_ENGINE_mix(AUDIO_ENGINE* audioEngine) {
  uint16_t* writeCursor = (uint16_t*)(audioEngine->outputBuffer);
  uint32_t totalSamples = audioEngine->spec.samples;
  uint32_t outputBufferSize = totalSamples * channels * bytesPerSample;
  SDL_memset(writeCursor, 0, totalSamples);
  uint32_t samplesWritten = 0;

  // Get channel
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)(audioEngine->channels[0]);
  if (channel != NULL && channel->enabled) {
    AUDIO_DATA* audio = channel->audio;
    int32_t samplesInChannel = audio->length / bytesPerSample - channel->position;
    int32_t samplesToWrite = totalSamples - (SDL_GetQueuedAudioSize(audioEngine->deviceId) / (bytesPerSample * channels));
    uint16_t* readCursor = (uint16_t*)(audio->buffer);
    readCursor += channel->position;

    // Mono data needs to be interleaved to output to stereo
    for (size_t i = 0; i < min(samplesToWrite, samplesInChannel); i++) {
      writeCursor[i*channels] = readCursor[i];
      writeCursor[i*channels+1] = readCursor[i];
      samplesWritten++;
    }
    channel->position += samplesWritten; // account for channel
    channel->enabled = channel->position < audio->length / bytesPerSample;
    SDL_QueueAudio(audioEngine->deviceId, audioEngine->outputBuffer, samplesWritten*bytesPerSample*channels);
  }
}

internal void AUDIO_allocate(WrenVM* vm) {
  AUDIO_DATA* data = (AUDIO_DATA*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_DATA));
  const char* path = wrenGetSlotString(vm, 1);
  strncpy(data->name, path, 255);
  data->name[255] = '\0';
  SDL_LoadWAV(path, &data->spec, &data->buffer, &data->length);
  printf("%i\n", data->spec.freq);
  printf("%i\n", data->spec.samples);
  printf("%i\n", data->spec.format);
  printf("%i\n", data->spec.channels);
  printf("Audio loaded: %s\n", path);
}

internal void AUDIO_finalize(void* data) {
  AUDIO_DATA* audioData = (AUDIO_DATA*)data;
  if (audioData->buffer != NULL) {
    SDL_FreeWAV(audioData->buffer);
    audioData->buffer = NULL;
    printf("Audio unloaded: %s\n", audioData->name);
  }
}
internal void AUDIO_unload(WrenVM* vm) {
  AUDIO_DATA* data = (AUDIO_DATA*)wrenGetSlotForeign(vm, 0);
  AUDIO_finalize(data);
}

internal void AUDIO_ENGINE_allocate(WrenVM* vm) {
  AUDIO_ENGINE* engine = (AUDIO_ENGINE*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_ENGINE));
  engine->audioScale = 15;
  for (int i = 0; i < 4; i++) {
    engine->channels[i] = NULL;
  }
  // SETUP player
  // set the callback function
  (engine->spec).freq = 44100;
  (engine->spec).format = AUDIO_S16LSB;
  (engine->spec).channels = channels; // TODO: consider mono/stereo
  (engine->spec).samples = 4096;
  (engine->spec).callback = NULL;
  (engine->spec).userdata = engine;

  engine->outputBuffer = calloc(engine->spec.samples * channels * bytesPerSample, sizeof(uint8_t));

  // open audio device
  engine->deviceId = SDL_OpenAudioDevice(NULL, 0, &(engine->spec), NULL, 0);
  // TODO: Handle if we can't get a device!

  // Unpause audio so we can begin taking over the buffer
  SDL_PauseAudioDevice(engine->deviceId, 0);
}

internal void AUDIO_ENGINE_update(WrenVM* vm) {
  // We need additional slots to parse a list
  wrenEnsureSlots(vm, 3);
  AUDIO_ENGINE* data = (AUDIO_ENGINE*)wrenGetSlotForeign(vm, 0);
  uint8_t soundCount = wrenGetListCount(vm, 1);
  for (int i = 0; i < CHANNEL_MAX; i++) {
    if (i < soundCount) {
      wrenGetListElement(vm, 1, i, 2);
      if (wrenGetSlotType(vm, 2) != WREN_TYPE_NULL) {
        data->channels[i] = wrenGetSlotForeign(vm, 2);
        data->channels[i]->enabled = data->channels[i]->requestEnabled;
      }
    } else {
      data->channels[i] = NULL;
    }
  }
  AUDIO_ENGINE_mix(data);
}

internal void AUDIO_ENGINE_finalize(void* audioEngine) {
  // We might need to free contained audio here
  AUDIO_ENGINE* engine = (AUDIO_ENGINE*)audioEngine;
  SDL_PauseAudioDevice(engine->deviceId, 1);
  SDL_CloseAudioDevice(engine->deviceId);
  free(engine->outputBuffer);
}

internal void AUDIO_CHANNEL_allocate(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(AUDIO_CHANNEL));
  int16_t id = (int16_t)wrenGetSlotDouble(vm, 1);
  data->channelId = id;
  data->enabled = true;
  data->requestEnabled = true;
  data->audio = (AUDIO_DATA*)wrenGetSlotForeign(vm, 2);
}

internal void AUDIO_CHANNEL_isFinished(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotBool(vm, 0, !data->enabled);
}

internal void AUDIO_CHANNEL_getId(WrenVM* vm) {
  AUDIO_CHANNEL* data = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, data->channelId);
}

internal void AUDIO_CHANNEL_setEnabled(WrenVM* vm) {
  AUDIO_CHANNEL* channel = (AUDIO_CHANNEL*)wrenGetSlotForeign(vm, 0);
  channel->requestEnabled = wrenGetSlotBool(vm, 1);
}

internal void AUDIO_CHANNEL_finalize(void* data) {
  // printf("Channel finished\n");
}

internal double
dbToVolume(double dB) {
  return pow(10.0, 0.05 * dB);
}

internal double
volumeToDb(double volume) {
  return 20.0 * log10(volume);
}
