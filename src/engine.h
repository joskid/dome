// Forward-declaring some methods for interacting with the AudioEngine
// for managing memory and initialization
struct AUDIO_ENGINE_t;
internal struct AUDIO_ENGINE_t* AUDIO_ENGINE_init(void);
internal void AUDIO_ENGINE_free(struct AUDIO_ENGINE_t*);

typedef struct {
  double avgFps;
  double alpha;
  int32_t elapsed;
  FILE* logFile;
  size_t errorBufLen;
  size_t errorBufMax;
  char* errorBuf;
} ENGINE_DEBUG;

typedef struct {
  SDL_Window* window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Rect viewport;
  void* pixels;
  ABC_FIFO fifo;
  MAP moduleMap;
  uint32_t width;
  uint32_t height;
  mtar_t* tar;
  bool running;
  bool lockstep;
  int exit_status;
  struct AUDIO_ENGINE_t* audioEngine;
  bool debugEnabled;
  bool vsyncEnabled;
  ENGINE_DEBUG debug;
} ENGINE;

typedef enum {
  EVENT_NOP,
  EVENT_LOAD_FILE,
  EVENT_WRITE_FILE,
  EVENT_WRITE_FILE_APPEND
} EVENT_TYPE;

typedef enum {
  TASK_NOP,
  TASK_PRINT,
  TASK_LOAD_FILE,
  TASK_WRITE_FILE,
  TASK_WRITE_FILE_APPEND
} TASK_TYPE;

typedef enum {
  ENGINE_WRITE_SUCCESS,
  ENGINE_WRITE_PATH_INVALID
} ENGINE_WRITE_RESULT;

global_variable uint32_t ENGINE_EVENT_TYPE;

