#include "PageTurnAnimator.h"

#include <GfxRenderer.h>
#include <Logging.h>

PageTurnAnimator pageTurnAnimator;

uint8_t PageTurnAnimator::stripsForSpeed(Speed speed) {
  switch (speed) {
    case VERY_FAST: return 1;   // 不分条，整屏一次刷
    case FAST:      return 2;
    case NORMAL:    return 4;
    case SLOW:      return 6;
    case VERY_SLOW: return MAX_PARTIAL_BEFORE_FULL;  // 8 条（安全上限）
  }
  return 4;
}

PageTurnAnimator::Result PageTurnAnimator::animate(GfxRenderer& renderer, Mode mode, Speed speed,
                                                    const std::function<bool()>& cancelCheck) {
  if (mode == OFF) return NOT_RUN;

  const int screenW = renderer.getScreenWidth();
  const int screenH = renderer.getScreenHeight();
  if (screenW <= 0 || screenH <= 0) return NOT_RUN;

  const uint8_t strips = stripsForSpeed(speed);
  const uint32_t startMs = millis();

  LOG_DBG("ANIM", "start mode=%u speed=%u strips=%u screen=%dx%d",
          static_cast<unsigned>(mode), static_cast<unsigned>(speed),
          static_cast<unsigned>(strips), screenW, screenH);

  auto refreshStrip = [&](uint8_t idx) -> bool {
    const int stripH = screenH / strips;
    if (stripH <= 0) return false;
    const int y = idx * stripH;
    const int h = (idx == strips - 1) ? (screenH - y) : stripH;
    // GfxRenderer::displayWindow 负责逻辑->物理旋转 + 8 像素对齐 + 边界裁剪
    renderer.displayWindow(0, y, screenW, h);
#ifdef SIMULATOR
    delay(300);  // 仅模拟器：人为延时让动画肉眼可见；真机由墨水屏波形自然耗时
#endif
    if (partialsSinceFull_ < 255) partialsSinceFull_++;
    return true;
  };

  // 条带顺序：SCROLL 是 0..N-1；BLINDS 是 0,2,4,... 再 1,3,5,...
  auto orderAt = [&](uint8_t step) -> uint8_t {
    if (mode == SCROLL) return step;
    const uint8_t half = static_cast<uint8_t>((strips + 1) / 2);
    if (step < half) return static_cast<uint8_t>(step * 2);
    return static_cast<uint8_t>((step - half) * 2 + 1);
  };

  for (uint8_t i = 0; i < strips; i++) {
    if (millis() - startMs > MAX_ANIMATION_TOTAL_MS) {
      LOG_INF("ANIM", "timeout at %u/%u", static_cast<unsigned>(i),
               static_cast<unsigned>(strips));
      return CANCELLED;
    }
    if (cancelCheck && (millis() - startMs) >= CANCEL_GRACE_MS && cancelCheck()) {
      LOG_DBG("ANIM", "cancel at %u/%u", static_cast<unsigned>(i),
              static_cast<unsigned>(strips));
      return CANCELLED;
    }
    const uint8_t idx = orderAt(i);
    if (idx >= strips) continue;
    if (!refreshStrip(idx)) break;
  }

  LOG_DBG("ANIM", "done in %lu ms, partials=%u",
          static_cast<unsigned long>(millis() - startMs),
          static_cast<unsigned>(partialsSinceFull_));
  return COMPLETED;
}
