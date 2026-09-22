#include "ZenHomeFace.h"

#include <Arduino.h>
#include <GfxRenderer.h>
#include <Logging.h>
#include <time.h>

#include <cstdio>
#include <cstring>

#include "StandbyTime.h"
#include "fontIds.h"
#include "util/TimeUtils.h"

namespace {

// CJK 数字 0-9
const char* kCjkDigits[10] = {"零", "一", "二", "三", "四", "五", "六", "七", "八", "九"};

constexpr const char* kShi = "时";

// 小时部分：返回 1-2 个汉字 + 是否追加「時」
// 规则：1-10 加時；11-24 用两位汉字，不加時。
void formatHour(unsigned hh, const char*& d1, const char*& d2, bool& showShi) {
  d1 = nullptr;
  d2 = nullptr;
  showShi = false;
  if (hh == 0) {
    d1 = "零";
    showShi = true;
  } else if (hh <= 10) {
    d1 = kCjkDigits[hh];
    showShi = true;
  } else if (hh <= 19) {
    d1 = "十";
    d2 = kCjkDigits[hh - 10];
  } else if (hh == 20) {
    d1 = "二";
    d2 = "十";
  } else if (hh <= 23) {
    d1 = "二";
    d2 = kCjkDigits[hh - 20];
  } else {
    d1 = "二";
    d2 = "四";
  }
}

// 分钟部分：返回 1-2 个汉字
// 规则：
//   0      → 「時」（整点占位）
//   1-9    → 「零」「X」
//   10     → 「一」「十」
//   11-19  → 「十」「X」
//   20     → 「二」「十」  21-29 → 「二」「X」
//   30     → 「三」「十」  31-39 → 「三」「X」
//   40     → 「四」「十」  41-49 → 「四」「X」
//   50     → 「五」「十」  51-59 → 「五」「X」
void formatMinute(unsigned mm, const char*& d1, const char*& d2) {
  d1 = nullptr;
  d2 = nullptr;
  if (mm == 0) {
    d1 = "时";
  } else if (mm <= 9) {
    d1 = "零";
    d2 = kCjkDigits[mm];
  } else if (mm == 10) {
    d1 = "一";
    d2 = "十";
  } else if (mm <= 19) {
    d1 = "十";
    d2 = kCjkDigits[mm - 10];
  } else if (mm == 20) {
    d1 = "二";
    d2 = "十";
  } else if (mm <= 29) {
    d1 = "二";
    d2 = kCjkDigits[mm - 20];
  } else if (mm == 30) {
    d1 = "三";
    d2 = "十";
  } else if (mm <= 39) {
    d1 = "三";
    d2 = kCjkDigits[mm - 30];
  } else if (mm == 40) {
    d1 = "四";
    d2 = "十";
  } else if (mm <= 49) {
    d1 = "四";
    d2 = kCjkDigits[mm - 40];
  } else if (mm == 50) {
    d1 = "五";
    d2 = "十";
  } else {
    d1 = "五";
    d2 = kCjkDigits[mm - 50];
  }
}

const char* periodName(unsigned hh) {
  if (hh < 6) return "凌晨";
  if (hh < 12) return "上午";
  if (hh < 18) return "下午";
  return "晚上";
}

const char* weekdayName(int wd) {
  static const char* kDays[7] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
  return kDays[(wd % 7 + 7) % 7];
}

// ===== 字体 =====
// 大字：内置 72pt 思源黑体（12 汉字：零一二三四五六七八九十时）
// 小字：内置 UI 12pt
constexpr int kSmallFontId = SANS_12_FONT_ID;
// 与 main.cpp 的 ZEN_72_FONT_ID 保持一致
constexpr int kBigFontId = 0x434A4B48;

}  // namespace

void ZenHomeFace::onEnter() {
  startMs_ = millis();
  lastMin_ = -1;
  // 字体加载推迟到 render()（onEnter 拿不到 renderer）
}

void ZenHomeFace::onExit() {}

StandbyFace::TickResult ZenHomeFace::tick() {
  const uint32_t nowMin = standby_time::getMinuteTick(startMs_);
  if (static_cast<int32_t>(nowMin) == lastMin_) return TickResult::None;
  lastMin_ = static_cast<int32_t>(nowMin);
  return TickResult::Redraw;
}

void ZenHomeFace::render(GfxRenderer& renderer, const Rect& viewport) {
  // ---- 1. 取当前时间 ----
  unsigned hh = 0, mm = 0;
  standby_time::getNowHHMM(startMs_, hh, mm);

  // ---- 2. 取日期 ----
  int year = 0, month = 0, day = 0, wd = 0;
  const time_t ts = TimeUtils::getCurrentValidTimestamp();
  std::tm local = {};
  if (ts && TimeUtils::getLocalDateTime(ts, local)) {
    year = local.tm_year + 1900;
    month = local.tm_mon + 1;
    day = local.tm_mday;
    wd = local.tm_wday;
  }

  // ---- 3. 拆汉字 ----
  const char* h1 = nullptr;
  const char* h2 = nullptr;
  bool showShi = false;
  formatHour(hh, h1, h2, showShi);

  const char* m1 = nullptr;
  const char* m2 = nullptr;
  formatMinute(mm, m1, m2);

  // ---- 4. 计算布局 ----
  // 行组成：小时(1-2) + 时(0-1) + 中间信息(1) + 分钟(1-2)
  // 总行数最少 3 行，最多 6 行
  const int hourLines = (h2 ? 2 : 1) + (showShi ? 1 : 0);
  const int minLines = (m2 ? 2 : 1);
  const int bigLineCount = hourLines + minLines;   // 大字行数
  const int totalBigLines = bigLineCount + 1;      // +1 给中间小字留半个大字行

  // 大字行高：铺满 viewport 高度减去小字余量
  // 小字块固定占屏幕高度的 11%，两行之间留出足够呼吸感，
  // 不随大字行数变化被压缩。
  const int smallBlockH = viewport.height * 11 / 100;
  const int smallLineH = smallBlockH / 2;  // 小字块拆成两行（星期/时段 + 日期/时分）
  const int bigLineH = (viewport.height - smallBlockH) / totalBigLines;

  // 大字的视觉中心高度（基线需要往下偏移）
  const int bigAscender = renderer.getFontAscenderSize(kBigFontId);
  const int bigLineVisualY = (bigLineH - bigAscender) / 2;  // 让 glyph 视觉上垂直居中

  const int centerX = viewport.x + viewport.width / 2;
  // 小字块严格垂直居中在屏幕中心，拆成两行：
  //   第 1 行：星期 + 时段（如「周日凌晨」）
  //   第 2 行：日期 + 时分（如「9月20日 1:20」）
  // 小时区向上排、分钟区向下排，上下留白天然对称。
  const int midY = viewport.y + viewport.height / 2;
  const int smallTop = midY - smallBlockH / 2;
  // 72pt 大字的 ascender 远大于 descender，汉字字形像素高度约 ascender 的 70%，
  // 使字形视觉重心偏下，小时最后一行会贴近小字块。
  // 上提 bigAscender/3 让上下视觉距离对称。
  // 再收窄 1/3：小时区下移 kGapAdjust，分钟区上移同量（保持对称）。
  const int kGapAdjust = bigAscender / 9;
  const int hourOffset = -bigAscender / 3 + kGapAdjust;
  int y = smallTop - hourLines * bigLineH + hourOffset;

  // 水平居中画一个字符串
  auto drawH = [&](int fontId, int yy, const char* text, bool black) {
    if (!text) return;
    const int w = renderer.getTextWidth(fontId, text);
    renderer.drawText(fontId, centerX - w / 2, yy, text, black);
  };

  // 竖直居中画大字（视觉居中，非基线居中）
  auto drawBig = [&](int yy, const char* text) {
    if (!text) return;
    drawH(kBigFontId, yy + bigLineVisualY, text, true);
  };

  // ---- 5. 小时 ----
  drawBig(y, h1);
  y += bigLineH;
  if (h2) {
    drawBig(y, h2);
    y += bigLineH;
  }
  if (showShi) {
    drawBig(y, kShi);
    y += bigLineH;
  }

  // ---- 6. 中间信息（两行）----
  {
    const int smallVisualY = (smallLineH - renderer.getFontAscenderSize(kSmallFontId)) / 2;
    // 第 1 行：星期 + 时段
    char line1[32];
    std::snprintf(line1, sizeof(line1), "%s%s", weekdayName(wd), periodName(hh));
    drawH(kSmallFontId, smallTop + smallVisualY, line1, true);
    // 第 2 行：日期 + 时分
    char line2[48];
    std::snprintf(line2, sizeof(line2), "%d月%d日  %u:%02u", month, day, hh, mm);
    drawH(kSmallFontId, smallTop + smallLineH + smallVisualY, line2, true);
  }
  y = smallTop + smallBlockH - kGapAdjust;

  // ---- 7. 分钟 ----
  drawBig(y, m1);
  y += bigLineH;
  if (m2) {
    drawBig(y, m2);
  }
}

StrId ZenHomeFace::titleId() const { return StrId::STR_ZEN_CLOCK_TITLE; }

uint32_t ZenHomeFace::secondsUntilNextWake() const {
  if (standby_time::isSynced()) {
    const time_t now = time(nullptr);
    const uint32_t sec = static_cast<uint32_t>(now % 60);
    return (sec == 0) ? 60 : (60 - sec);
  }
  const uint32_t sec = static_cast<uint32_t>(((millis() - startMs_) / 1000u) % 60u);
  return (sec == 0) ? 60 : (60 - sec);
}
