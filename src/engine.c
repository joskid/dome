internal void
ENGINE_openLogFile(ENGINE* engine) {
  // DOME-2020-02-02-090000.log
  char* filename = "DOME-out.log";
  engine->debug.logFile = fopen(filename, "w+");
}

internal void
ENGINE_printLog(ENGINE* engine, char* line, ...) {
  // Args is mutated by each vsnprintf call,
  // so it needs to be reinitialised.
  va_list args;
  va_start(args, line);
  size_t bufSize = vsnprintf(NULL, 0, line, args) + 1;
  va_end(args);

  char buffer[bufSize];
  buffer[0] = '\0';
  va_start(args, line);
  vsnprintf(buffer, bufSize, line, args);
  va_end(args);

  // Output to console
  printf("%s", buffer);

  if (engine->debug.logFile == NULL) {
    ENGINE_openLogFile(engine);
  }
  if (engine->debug.logFile != NULL) {
    // Output to file
    fputs(buffer, engine->debug.logFile);
  }
}

internal ENGINE_WRITE_RESULT
ENGINE_writeFile(ENGINE* engine, char* path, char* buffer, size_t length) {
  char* base = BASEPATH_get();
  char* fullPath = malloc(strlen(base)+strlen(path)+1);
  strcpy(fullPath, base); /* copy name into the new var */
  strcat(fullPath, path); /* add the extension */

  ENGINE_printLog(engine, "Writing to filesystem: %s\n", path);
  int result = writeEntireFile(fullPath, buffer, length);
  if (result == ENOENT) {
    result = ENGINE_WRITE_PATH_INVALID;
  } else {
    result = ENGINE_WRITE_SUCCESS;
  }
  free(fullPath);

  return result;
}

internal char*
ENGINE_readFile(ENGINE* engine, char* path, size_t* lengthPtr) {
  if (engine->tar != NULL) {
    char pathBuf[PATH_MAX];
    strcpy(pathBuf, "\0");
    if (strncmp(path, "./", 2) != 0) {
      strcpy(pathBuf, "./");
    }
    strcat(pathBuf, path);
    mtar_header_t h;
    int success = mtar_find(engine->tar, pathBuf, &h);
    if (success == MTAR_ESUCCESS) {
      ENGINE_printLog(engine, "Reading from bundle: %s\n", path);
      char* file = readFileFromTar(engine->tar, pathBuf, lengthPtr);
      if (file == NULL) {
        ENGINE_printLog(engine, "Error: Couldn't read the data from the bundle.");
      }
      return file;
    } else if (success != MTAR_ENOTFOUND) {
      ENGINE_printLog(engine, "Error: There was a problem reading %s from the bundle.\n", pathBuf);
      return NULL;
    }
    ENGINE_printLog(engine, "Couldn't find %s in bundle, falling back.\n", pathBuf);
  }

  char* base = BASEPATH_get();
  char* fullPath = malloc(strlen(base)+strlen(path)+1);
  strcpy(fullPath, base); /* copy name into the new var */
  strcat(fullPath, path); /* add the extension */
  if (!doesFileExist(fullPath)) {
    free(fullPath);
    return NULL;
  } else {
    ENGINE_printLog(engine, "Reading from filesystem: %s\n", path);
    char* data = readEntireFile(fullPath, lengthPtr);
    free(fullPath);
    return data;
  }
}

internal int
ENGINE_taskHandler(ABC_TASK* task) {
  if (task->type == TASK_PRINT) {
    printf("%s\n", (char*)task->data);
    task->resultCode = 0;
    // TODO: Push to SDL Event Queue
  } else if (task->type == TASK_LOAD_FILE) {
    FILESYSTEM_loadEventHandler(task->data);
  } else if (task->type == TASK_WRITE_FILE) {
  }
  return 0;
}

internal bool
ENGINE_setupRenderer(ENGINE* engine, bool vsync) {
  engine->vsyncEnabled = vsync;
  if (engine->renderer != NULL) {
    SDL_DestroyRenderer(engine->renderer);
  }

  int flags = SDL_RENDERER_ACCELERATED;
  if (vsync) {
    flags |= SDL_RENDERER_PRESENTVSYNC;
  }
  engine->renderer = SDL_CreateRenderer(engine->window, -1, flags);
  if (engine->renderer == NULL) {
    return false;
  }
  SDL_RenderSetLogicalSize(engine->renderer, engine->width, engine->height);

  engine->texture = SDL_CreateTexture(engine->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, engine->width, engine->height);
  if (engine->texture == NULL) {
    return false;
  }
  return true;
}

internal int
ENGINE_init(ENGINE* engine) {
  int result = EXIT_SUCCESS;
  engine->window = NULL;
  engine->renderer = NULL;
  engine->texture = NULL;
  engine->pixels = NULL;
  engine->lockstep = false;
  engine->debug.avgFps = 58;
  engine->debugEnabled = false;
  engine->debug.alpha = 0.9;

  engine->debug.errorBufMax = 0;
  engine->debug.errorBuf = NULL;
  engine->debug.errorBufLen = 0;



  engine->width = GAME_WIDTH;
  engine->height = GAME_HEIGHT;


  //Create window
  engine->window = SDL_CreateWindow("DOME", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
  if(engine->window == NULL)
  {
    char* message = "Window could not be created! SDL_Error: %s\n";
    ENGINE_printLog(engine, message, SDL_GetError());
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  ENGINE_setupRenderer(engine, true);
  if (engine->renderer == NULL)
  {
    char* message = "Could not create a renderer: %s";
    ENGINE_printLog(engine, message, SDL_GetError());
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  engine->pixels = calloc(engine->width * engine->height, sizeof(char) * 4);
  if (engine->pixels == NULL) {
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  engine->audioEngine = AUDIO_ENGINE_init();
  if (engine->audioEngine == NULL) {
    result = EXIT_FAILURE;
    goto engine_init_end;
  }

  ENGINE_EVENT_TYPE = SDL_RegisterEvents(1);

  ABC_FIFO_create(&engine->fifo);
  engine->fifo.taskHandler = ENGINE_taskHandler;

  MAP_init(&engine->moduleMap);

  engine->running = true;

engine_init_end:
  return result;
}

internal void
ENGINE_finishAsync(ENGINE* engine) {
  if (!engine->fifo.shutdown) {
    ABC_FIFO_close(&engine->fifo);
  }
}

internal void
ENGINE_free(ENGINE* engine) {

  if (engine == NULL) {
    return;
  }

  ENGINE_finishAsync(engine);

  if (engine->audioEngine) {
    AUDIO_ENGINE_free(engine->audioEngine);
    free(engine->audioEngine);
    engine->audioEngine = NULL;
  }

  if (engine->tar != NULL) {
    mtar_finalize(engine->tar);
    free(engine->tar);
  }

  if (engine->moduleMap.head != NULL) {
    MAP_free(&engine->moduleMap);
  }

  if (engine->pixels != NULL) {
    free(engine->pixels);
  }

  if (engine->texture != NULL) {
    SDL_DestroyTexture(engine->texture);
  }

  if (engine->renderer != NULL) {
    SDL_DestroyRenderer(engine->renderer);
  }

  if (engine->window != NULL) {
    SDL_DestroyWindow(engine->window);
  }

  // DEBUG features
  if (engine->debug.logFile != NULL) {
    fclose(engine->debug.logFile);
  }

  if (engine->debug.errorBuf != NULL) {
    free(engine->debug.errorBuf);
  }



}

inline internal void
ENGINE_pset(ENGINE* engine, int64_t x, int64_t y, uint32_t c) {
  // Draw pixel at (x,y)
  int32_t width = engine->width;
  int32_t height = engine->height;
  if ((c & (0xFF << 24)) == 0) {
    return;
  } else if (0 <= x && x < width && 0 <= y && y < height) {
    if (((c & (0xFF << 24)) >> 24) < 0xFF) {
      uint32_t current = ((uint32_t*)(engine->pixels))[width * y + x];

      // uint16_t oldA = (0xFF000000 & current) >> 24;
      uint16_t newA = (0xFF000000 & c) >> 24;

      uint16_t oldR = (255-newA) * ((0x00FF0000 & current) >> 16);
      uint16_t oldG = (255-newA) * ((0x0000FF00 & current) >> 8);
      uint16_t oldB = (255-newA) * (0x000000FF & current);
      uint16_t newR = newA * ((0x00FF0000 & c) >> 16);
      uint16_t newG = newA * ((0x0000FF00 & c) >> 8);
      uint16_t newB = newA * (0x000000FF & c);
      uint8_t a = newA;
      uint8_t r = (oldR + newR) / 255;
      uint8_t g = (oldG + newG) / 255;
      uint8_t b = (oldB + newB) / 255;

      c = (a << 24) | (r << 16) | (g << 8) | b;
    }
    ((uint32_t*)(engine->pixels))[width * y + x] = c;
  }
}

internal void
ENGINE_print(ENGINE* engine, char* text, int64_t x, int64_t y, uint32_t c) {
  int fontWidth = 8;
  int fontHeight = 8;
  int cursor = 0;
  for (size_t pos = 0; pos < strlen(text); pos++) {
    uint8_t letter = text[pos];

    uint8_t* glyph = (uint8_t*)font8x8_basic[letter];
    if (*glyph == '\n') {
      break;
    }
    for (int j = 0; j < fontHeight; j++) {
      for (int i = 0; i < fontWidth; i++) {
        uint8_t v = (glyph[j] >> i) & 1;
        // uint8_t v = glyph[j * fontWidth + i];
        if (v != 0) {
          ENGINE_pset(engine, x + cursor + i, y + j, c);
        }
      }
    }
    cursor += fontWidth;
  }
}

internal void
ENGINE_line_high(ENGINE* engine, int64_t x1, int64_t y1, int64_t x2, int64_t y2, uint32_t c) {
  int64_t dx = x2 - x1;
  int64_t dy = y2 - y1;
  int64_t xi = 1;
  if (dx < 0) {
    xi = -1;
    dx = -dx;
  }
  int64_t p = 2 * dx - dy;

  int64_t y = y1;
  int64_t x = x1;
  while(y <= y2) {
    ENGINE_pset(engine, x, y, c);
    if (p > 0) {
      x += xi;
      p = p - 2 * dy;
    } else {
      p = p + 2 * dx;
    }
    y++;
  }
}

internal void
ENGINE_line_low(ENGINE* engine, int64_t x1, int64_t y1, int64_t x2, int64_t y2, uint32_t c) {
  int64_t dx = x2 - x1;
  int64_t dy = y2 - y1;
  int64_t yi = 1;
  if (dy < 0) {
    yi = -1;
    dy = -dy;
  }
  int64_t p = 2 * dy - dx;

  int64_t y = y1;
  int64_t x = x1;
  while(x <= x2) {
    ENGINE_pset(engine, x, y, c);
    if (p > 0) {
      y += yi;
      p = p - 2 * dx;
    } else {
      p = p + 2 * dy;
    }
    x++;
  }
}

internal void
ENGINE_line(ENGINE* engine, int64_t x1, int64_t y1, int64_t x2, int64_t y2, uint32_t c) {
  if (llabs(y2 - y1) < llabs(x2 - x1)) {
    if (x1 > x2) {
      ENGINE_line_low(engine, x2, y2, x1, y1, c);
    } else {
      ENGINE_line_low(engine, x1, y1, x2, y2, c);
    }
  } else {
    if (y1 > y2) {
      ENGINE_line_high(engine, x2, y2, x1, y1, c);
    } else {
      ENGINE_line_high(engine, x1, y1, x2, y2, c);
    }

  }

}

internal void
ENGINE_circle_filled(ENGINE* engine, int64_t x0, int64_t y0, int64_t r, uint32_t c) {
  int64_t x = 0;
  int64_t y = r;
  int64_t d = round(M_PI - (2*r));

  while (x <= y) {
    ENGINE_line(engine, x0 - x, y0 + y, x0 + x, y0 + y, c);
    ENGINE_line(engine, x0 - y, y0 + x, x0 + y, y0 + x, c);
    ENGINE_line(engine, x0 + x, y0 - y, x0 - x, y0 - y, c);
    ENGINE_line(engine, x0 - y, y0 - x, x0 + y, y0 - x, c);

    if (d < 0) {
      d = d + (M_PI * x) + (M_PI * 2);
    } else {
      d = d + (M_PI * (x - y)) + (M_PI * 3);
      y--;
    }
    x++;
  }
}

internal void
ENGINE_circle(ENGINE* engine, int64_t x0, int64_t y0, int64_t r, uint32_t c) {
  int64_t x = 0;
  int64_t y = r;
  int64_t d = round(M_PI - (2*r));

  while (x <= y) {
    ENGINE_pset(engine, x0 + x, y0 + y, c);
    ENGINE_pset(engine, x0 + y, y0 + x, c);
    ENGINE_pset(engine, x0 - y, y0 + x, c);
    ENGINE_pset(engine, x0 - x, y0 + y, c);

    ENGINE_pset(engine, x0 - x, y0 - y, c);
    ENGINE_pset(engine, x0 - y, y0 - x, c);
    ENGINE_pset(engine, x0 + y, y0 - x, c);
    ENGINE_pset(engine, x0 + x, y0 - y, c);

    if (d < 0) {
      d = d + (M_PI * x) + (M_PI * 2);
    } else {
      d = d + (M_PI * (x - y)) + (M_PI * 3);
      y--;
    }
    x++;
  }
}

internal inline double
ellipse_getRegion(double x, double y, int32_t rx, int32_t ry) {
  double rxSquare = rx * rx;
  double rySquare = ry * ry;
  return (rySquare*x) / (rxSquare*y);
}

internal void
ENGINE_ellipsefill(ENGINE* engine, int64_t x0, int64_t y0, int64_t x1, int64_t y1, uint32_t c) {

  // Calculate radius
  int32_t rx = (x1 - x0) / 2; // Radius on x
  int32_t ry = (y1 - y0) / 2; // Radius on y
  uint32_t rxSquare = rx*rx;
  uint32_t rySquare = ry*ry;
  uint32_t rx2ry2 = rxSquare * rySquare;

  // calculate center co-ordinates
  int32_t xc = min(x0, x1) + rx;
  int32_t yc = min(y0, y1) + ry;

  // Start drawing at (0,ry)
  int32_t x = 0;
  int32_t y = ry;
  double d = 0;

  while (fabs(ellipse_getRegion(x, y, rx, ry)) < 1) {
    x++;
    double xSquare = x*x;
    // valuate decision paramter
    d = rySquare * xSquare + rxSquare * pow(y - 0.5, 2) - rx2ry2;

    if (d > 0) {
      y--;
    }
    ENGINE_line(engine, xc+x, yc+y, xc-x, yc+y, c);
    ENGINE_line(engine, xc-x, yc-y, xc+x, yc-y, c);
  }

  while (y > 0) {
    y--;
    double ySquare = y*y;
    // valuate decision paramter
    d = rxSquare * ySquare + rySquare * pow(x + 0.5, 2) - rx2ry2;

    if (d <= 0) {
      x++;
    }
    ENGINE_line(engine, xc+x, yc+y, xc-x, yc+y, c);
    ENGINE_line(engine, xc-x, yc-y, xc+x, yc-y, c);
  };
}

internal void
ENGINE_ellipse(ENGINE* engine, int64_t x0, int64_t y0, int64_t x1, int64_t y1, uint32_t c) {

  // Calcularte radius
  int32_t rx = llabs(x1 - x0) / 2; // Radius on x
  int32_t ry = llabs(y1 - y0) / 2; // Radius on y
  int32_t rxSquare = rx*rx;
  int32_t rySquare = ry*ry;
  int32_t rx2ry2 = rxSquare * rySquare;

  // calculate center co-ordinates
  int32_t xc = min(x0, x1) + rx;
  int32_t yc = min(y0, y1) + ry;

  // Start drawing at (0,ry)
  double x = 0;
  double y = ry;
  double d = 0;

  ENGINE_pset(engine, xc+x, yc+y, c);
  ENGINE_pset(engine, xc+x, yc-y, c);

  while (fabs(ellipse_getRegion(x, y, rx, ry)) < 1) {
    x++;
    double xSquare = x*x;
    // valuate decision paramter
    d = rySquare * xSquare + rxSquare * pow(y - 0.5, 2) - rx2ry2;

    if (d > 0) {
      y--;
    }
    ENGINE_pset(engine, xc+x, yc+y, c);
    ENGINE_pset(engine, xc-x, yc-y, c);
    ENGINE_pset(engine, xc-x, yc+y, c);
    ENGINE_pset(engine, xc+x, yc-y, c);
  }

  while (y > 0) {
    y--;
    double ySquare = y*y;
    // valuate decision paramter
    d = rxSquare * ySquare + rySquare * pow(x + 0.5, 2) - rx2ry2;

    if (d <= 0) {
      x++;
    }
    ENGINE_pset(engine, xc+x, yc+y, c);
    ENGINE_pset(engine, xc-x, yc-y, c);
    ENGINE_pset(engine, xc-x, yc+y, c);
    ENGINE_pset(engine, xc+x, yc-y, c);
  };
}

internal void
ENGINE_rect(ENGINE* engine, int64_t x, int64_t y, int64_t w, int64_t h, uint32_t c) {
  ENGINE_line(engine, x, y, x, y+h-1, c);
  ENGINE_line(engine, x, y, x+w-1, y, c);
  ENGINE_line(engine, x, y+h-1, x+w-1, y+h-1, c);
  ENGINE_line(engine, x+w-1, y, x+w-1, y+h-1, c);
}

internal void
ENGINE_rectfill(ENGINE* engine, int64_t x, int64_t y, int64_t w, int64_t h, uint32_t c) {
  int32_t width = engine->width;
  int32_t height = engine->height;
  int64_t x1 = mid(0, x, width);
  int64_t y1 = mid(0, y, height);
  int64_t x2 = mid(0, x + w, width);
  int64_t y2 = mid(0, y + h, height);

  for (int64_t j = y1; j < y2; j++) {
    for (int64_t i = x1; i < x2; i++) {
      ENGINE_pset(engine, i, j, c);
    }
  }
}

internal bool
ENGINE_getKeyState(ENGINE* engine, char* keyName) {
  SDL_Keycode keycode =  SDL_GetKeyFromName(keyName);
  SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);
  uint8_t* state = SDL_GetKeyboardState(NULL);
  return state[scancode];
}

internal float
ENGINE_getMouseX(ENGINE* engine) {
  SDL_Rect viewport = engine->viewport;

  int mouseX;
  int mouseY;
  int winX;
  int winY;
  SDL_GetMouseState(&mouseX, &mouseY);
  SDL_GetWindowSize(engine->window, &winX, &winY);
  return mouseX * max(((float)engine->width / (float)winX), (float)engine->height / (float)winY) - viewport.x;
}

internal float
ENGINE_getMouseY(ENGINE* engine) {
  SDL_Rect viewport = engine->viewport;

  int mouseX;
  int mouseY;
  int winX;
  int winY;
  SDL_GetMouseState(&mouseX, &mouseY);
  SDL_GetWindowSize(engine->window, &winX, &winY);
  return mouseY * max(((float)engine->width / (float)winX), (float)engine->height / (float)winY) - viewport.y;
}

internal bool
ENGINE_getMouseButton(int button) {
  return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button);
}

internal void
ENGINE_drawDebug(ENGINE* engine) {
  char buffer[20];
  ENGINE_DEBUG* debug = &engine->debug;
  // Choose alpha depending on how fast or slow you want old averages to decay.
  // 0.9 is usually a good choice.
  double framesThisSecond = 1000.0 / (debug->elapsed+1);
  double alpha = debug->alpha;
  debug->avgFps = alpha * debug->avgFps + (1.0 - alpha) * framesThisSecond;
  snprintf(buffer, sizeof(buffer), "%.01f fps", debug->avgFps);   // here 2 means binary
  int32_t width = engine->width;
  int32_t height = engine->height;
  int64_t startX = width - 4*8-2;
  int64_t startY = height - 8-2;

  ENGINE_rectfill(engine, startX, startY, 4*8+2, 10, 0x7F000000);
  ENGINE_print(engine, buffer, startX+1,startY+1, 0xFFFFFFFF);

  startX = width - 9*8 - 2;
  if (engine->vsyncEnabled) {
    ENGINE_print(engine, "VSync On", startX, startY - 8, 0xFFFFFFFF);
  } else {
    ENGINE_print(engine, "VSync Off", startX, startY - 8, 0xFFFFFFFF);
  }

  if (engine->lockstep) {
    ENGINE_print(engine, "Lockstep", startX, startY - 16, 0xFFFFFFFF);
  } else {
    ENGINE_print(engine, "Catchup", startX, startY - 16, 0xFFFFFFFF);
  }
}

internal bool
ENGINE_canvasResize(ENGINE* engine, uint32_t newWidth, uint32_t newHeight, uint32_t color) {
  if (engine->width == newWidth && engine->height == newHeight) {
    return true;
  }

  engine->width = newWidth;
  engine->height = newHeight;
  SDL_DestroyTexture(engine->texture);
  SDL_RenderSetLogicalSize(engine->renderer, newWidth, newHeight);

  engine->texture = SDL_CreateTexture(engine->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, newWidth, newHeight);
  if (engine->texture == NULL) {
    return false;
  }

  engine->pixels = realloc(engine->pixels, engine->width * engine->height * 4);
  if (engine->pixels == NULL) {
    return false;
  }
  ENGINE_rectfill(engine, 0, 0, engine->width, engine->height, color);

  return true;
}

internal void
ENGINE_takeScreenshot(ENGINE* engine) {
  size_t imageSize = engine->width * engine->height;
  uint8_t* destroyableImage = (uint8_t*)malloc(imageSize * 4 * sizeof(uint8_t));
  for (size_t i = 0; i < imageSize; i++) {
    uint32_t c = ((uint32_t*)engine->pixels)[i];
    uint8_t a = 0xFF;
    uint8_t r = (0x00FF0000 & c) >> 16;
    uint8_t g = (0x0000FF00 & c) >> 8;
    uint8_t b = (0x000000FF & c);
    ((uint32_t*)destroyableImage)[i] = a << 24 | b << 16 | g << 8 | r;
  }
  stbi_write_png("screenshot.png", engine->width, engine->height, 4, destroyableImage, engine->width * 4);
  free(destroyableImage);
}


internal void
ENGINE_reportError(ENGINE* engine) {
  if (engine->debug.errorBuf != NULL) {
    ENGINE_printLog(engine, engine->debug.errorBuf);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                             "DOME - Error",
                             engine->debug.errorBuf,
                             NULL);
  }
}
