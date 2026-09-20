#include "ZenClockActivity.h"

#include <GfxRenderer.h>
#include <Logging.h>
#include <Memory.h>

#include "MappedInputManager.h"

void ZenClockActivity::onEnter() {
  Activity::onEnter();
  savedOrientation_ = renderer.getOrientation();
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);
  face_ = makeUniqueNoThrow<ZenHomeFace>();
  if (face_) face_->onEnter();
  requestUpdate();
}

void ZenClockActivity::onExit() {
  if (face_) face_->onExit();
  face_.reset();
  renderer.setOrientation(savedOrientation_);
  Activity::onExit();
}

void ZenClockActivity::loop() {
  // 拨轮中键 / 返回键 / PWR 键 → 退出
  if (mappedInput.wasReleased(MappedInputManager::Button::Back) ||
      mappedInput.wasReleased(MappedInputManager::Button::Confirm) ||
      mappedInput.wasReleased(MappedInputManager::Button::Power)) {
    finish();
    return;
  }
  // 由 ZenHomeFace 判断是否需要重绘
  if (face_ && face_->tick() != StandbyFace::TickResult::None) {
    requestUpdate();
  }
}

void ZenClockActivity::render(RenderLock&&) {
  if (!face_) return;
  renderer.clearScreen(0xFF);
  const Rect viewport{0, 0, renderer.getScreenWidth(), renderer.getScreenHeight()};
  face_->render(renderer, viewport);
  renderer.displayBuffer();
}
