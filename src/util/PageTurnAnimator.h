#pragma once

#include <cstdint>
#include <functional>

class GfxRenderer;

// 翻页动画引擎（纯软件实现）。
//
// 职责：在“新页已渲染到 framebuffer、面板仍显示旧页”时，按条带逐步把新页
// 内容刷到面板上。仅调用 GfxRenderer::displayWindow()，不管理 framebuffer、
// 不修改 LUT / 波形、不直接操作控制器命令。
//
// 硬件安全约束（不可绕过，见 CLAUDE.md）：
//   - 单次动画连续局刷不超过 MAX_PARTIAL_BEFORE_FULL 条带
//   - 单次动画总时长不超过 MAX_ANIMATION_TOTAL_MS
//   - 超出约束立即停止，由调用方决定是否做全刷恢复
class PageTurnAnimator {
 public:
  enum Mode : uint8_t {
    OFF = 0,
    SCROLL = 1,   // 条带自上而下逐个揭开新页
    BLINDS = 2,   // 偶数条带先揭开，再揭开奇数条带
    MODE_COUNT
  };

  enum Speed : uint8_t {
    VERY_SLOW = 0,
    SLOW = 1,
    NORMAL = 2,
    FAST = 3,
    VERY_FAST = 4,
    SPEED_COUNT
  };

  // 硬件安全约束
  static constexpr uint8_t  MAX_PARTIAL_BEFORE_FULL = 8;
  static constexpr uint32_t MAX_ANIMATION_TOTAL_MS  = 15000;
  // 动画开始后的宽限期：期间不响应取消（避免翻页键的 press-edge 误取消）。
  static constexpr uint32_t CANCEL_GRACE_MS         = 300;

  enum Result : uint8_t {
    NOT_RUN = 0,    // mode==OFF / 参数无效；调用方走正常刷新
    COMPLETED = 1,  // 全部条带完成；调用方不要再 displayBuffer
    CANCELLED = 2   // 用户中断或超时；调用方应做一次全刷清残影
  };

  // 运行一次翻页动画。
  // cancelCheck：非空时每次条带前调用；返回 true 表示用户中断。
  Result animate(GfxRenderer& renderer, Mode mode, Speed speed,
                 const std::function<bool()>& cancelCheck = {});

  // 累计局刷计数（用于跨多次动画的残影控制）。
  uint8_t consecutivePartials() const { return partialsSinceFull_; }
  void noteFullRefresh() { partialsSinceFull_ = 0; }

  // 速度 → 条带数（越多越平滑，越慢）。
  static uint8_t stripsForSpeed(Speed speed);

 private:
  uint8_t partialsSinceFull_ = 0;
};

extern PageTurnAnimator pageTurnAnimator;
