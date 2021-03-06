internal void
CANVAS_print(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, STRING, "text");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "y");
  ASSERT_SLOT_TYPE(vm, 4, NUM, "color");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  char* text = (char*)wrenGetSlotString(vm, 1);
  int64_t x = round(wrenGetSlotDouble(vm, 2));
  int64_t y = round(wrenGetSlotDouble(vm, 3));
  uint32_t c = round(wrenGetSlotDouble(vm, 4));

  ENGINE_print(engine, text, x, y, c);
}

internal void
CANVAS_pset(WrenVM* vm)
{
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "color");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int64_t x = round(wrenGetSlotDouble(vm, 1));
  int64_t y = round(wrenGetSlotDouble(vm, 2));
  uint32_t c = round(wrenGetSlotDouble(vm, 3));
  ENGINE_pset(engine, x,y,c);
}

internal void
CANVAS_circle_filled(WrenVM* vm)
{
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "radius");
  ASSERT_SLOT_TYPE(vm, 4, NUM, "color");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int64_t x = round(wrenGetSlotDouble(vm, 1));
  int64_t y = round(wrenGetSlotDouble(vm, 2));
  int64_t r = round(wrenGetSlotDouble(vm, 3));
  uint32_t c = round(wrenGetSlotDouble(vm, 4));
  ENGINE_circle_filled(engine, x, y, r, c);
}

internal void
CANVAS_circle(WrenVM* vm)
{
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "radius");
  ASSERT_SLOT_TYPE(vm, 4, NUM, "color");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int64_t x = round(wrenGetSlotDouble(vm, 1));
  int64_t y = round(wrenGetSlotDouble(vm, 2));
  int64_t r = round(wrenGetSlotDouble(vm, 3));
  uint32_t c = round(wrenGetSlotDouble(vm, 4));
  ENGINE_circle(engine, x, y, r, c);
}
internal void
CANVAS_line(WrenVM* vm)
{
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x1");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y1");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "x2");
  ASSERT_SLOT_TYPE(vm, 4, NUM, "y2");
  ASSERT_SLOT_TYPE(vm, 5, NUM, "color");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int64_t x1 = round(wrenGetSlotDouble(vm, 1));
  int64_t y1 = round(wrenGetSlotDouble(vm, 2));
  int64_t x2 = round(wrenGetSlotDouble(vm, 3));
  int64_t y2 = round(wrenGetSlotDouble(vm, 4));
  uint32_t c = round(wrenGetSlotDouble(vm, 5));
  ENGINE_line(engine, x1, y1, x2, y2, c);
}

internal void
CANVAS_ellipse(WrenVM* vm)
{
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x1");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y1");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "x2");
  ASSERT_SLOT_TYPE(vm, 4, NUM, "y2");
  ASSERT_SLOT_TYPE(vm, 5, NUM, "color");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int64_t x1 = round(wrenGetSlotDouble(vm, 1));
  int64_t y1 = round(wrenGetSlotDouble(vm, 2));
  int64_t x2 = round(wrenGetSlotDouble(vm, 3));
  int64_t y2 = round(wrenGetSlotDouble(vm, 4));
  uint32_t c = round(wrenGetSlotDouble(vm, 5));
  ENGINE_ellipse(engine, x1, y1, x2, y2, c);
}

internal void
CANVAS_ellipsefill(WrenVM* vm)
{
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x1");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y1");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "x2");
  ASSERT_SLOT_TYPE(vm, 4, NUM, "y2");
  ASSERT_SLOT_TYPE(vm, 5, NUM, "color");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int64_t x1 = round(wrenGetSlotDouble(vm, 1));
  int64_t y1 = round(wrenGetSlotDouble(vm, 2));
  int64_t x2 = round(wrenGetSlotDouble(vm, 3));
  int64_t y2 = round(wrenGetSlotDouble(vm, 4));
  uint32_t c = round(wrenGetSlotDouble(vm, 5));
  ENGINE_ellipsefill(engine, x1, y1, x2, y2, c);
}

internal void
CANVAS_rect(WrenVM* vm)
{
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x1");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y1");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "x2");
  ASSERT_SLOT_TYPE(vm, 4, NUM, "y2");
  ASSERT_SLOT_TYPE(vm, 5, NUM, "color");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int64_t x = round(wrenGetSlotDouble(vm, 1));
  int64_t y = round(wrenGetSlotDouble(vm, 2));
  int64_t w = round(wrenGetSlotDouble(vm, 3));
  int64_t h = round(wrenGetSlotDouble(vm, 4));
  uint32_t c = round(wrenGetSlotDouble(vm, 5));
  ENGINE_rect(engine, x, y, w, h, c);
}

internal void
CANVAS_rectfill(WrenVM* vm)
{
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x1");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y1");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "x2");
  ASSERT_SLOT_TYPE(vm, 4, NUM, "y2");
  ASSERT_SLOT_TYPE(vm, 5, NUM, "color");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  int64_t x = round(wrenGetSlotDouble(vm, 1));
  int64_t y = round(wrenGetSlotDouble(vm, 2));
  int64_t w = round(wrenGetSlotDouble(vm, 3));
  int64_t h = round(wrenGetSlotDouble(vm, 4));
  uint32_t c = round(wrenGetSlotDouble(vm, 5));
  ENGINE_rectfill(engine, x, y, w, h, c);
}

internal void
CANVAS_getWidth(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  wrenSetSlotDouble(vm, 0, engine->width);
}
internal void
CANVAS_getHeight(WrenVM* vm) {
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  wrenSetSlotDouble(vm, 0, engine->height);
}

internal void
CANVAS_resize(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, NUM, "width");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "height");
  ASSERT_SLOT_TYPE(vm, 3, NUM, "color");
  ENGINE* engine = (ENGINE*)wrenGetUserData(vm);
  uint32_t width = wrenGetSlotDouble(vm, 1);
  uint32_t height = wrenGetSlotDouble(vm, 2);
  uint32_t color = wrenGetSlotDouble(vm, 3);
  bool success = ENGINE_canvasResize(engine, width, height, color);
  if (success == false) {
    VM_ABORT(vm, SDL_GetError());
    return;
  }
}

