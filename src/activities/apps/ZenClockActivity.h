#pragma once

#include <memory>

#include "activities/Activity.h"
#include "activities/apps/standby/ZenHomeFace.h"

// 全屏禅意时钟 Activity。复用 ZenHomeFace 的渲染逻辑，
// 但在独立 Activity 里运行，可以从 Apps 菜单进入。
class ZenClockActivity final : public Activity {
 public:
  ZenClockActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ZenClock", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  std::unique_ptr<ZenHomeFace> face_;
  GfxRenderer::Orientation savedOrientation_ = GfxRenderer::Orientation::Portrait;
};
