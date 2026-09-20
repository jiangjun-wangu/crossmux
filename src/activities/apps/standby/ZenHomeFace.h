#pragma once

#include "StandbyFace.h"

// "汉字禅意时钟" — 竖屏专用 standby face。
// 把当前时间拆成汉字逐位显示：
//   小时：「一」「時」...(≤10 加時) / 「十」「一」...(≥11 不加時)
//   分钟：「二」「十」...(逐位，整点显示「時」)
// 中间夹一行小字：时段 + 星期 + 日期 + 数字时间。
class ZenHomeFace final : public StandbyFace {
 public:
  void onEnter() override;
  void onExit() override;
  TickResult tick() override;
  void render(GfxRenderer& renderer, const Rect& viewport) override;
  StrId titleId() const override;
  uint32_t secondsUntilNextWake() const override;
  bool wantsGrayscale() const override { return false; }

 private:
  uint32_t startMs_ = 0;
  int32_t lastMin_ = -1;
};
