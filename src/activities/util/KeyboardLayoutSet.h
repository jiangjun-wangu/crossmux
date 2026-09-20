#pragma once

#include <FreeInkUI.h>
#include <I18n.h>

#include <cstdint>

namespace keyboard_layouts {

struct LayoutInfo {
  freeink::ui::KeyboardLayoutId id;
  Language language;
};

// Table position is the persisted bit assignment. Keep existing rows in place
// and append new layouts so SDK enum changes cannot reinterpret saved masks.
//
// CrossMux 精简：只保留英文 QWERTY。其它语言的键位数据仍编译进固件
// （子模块 freeink-sdk），此处只决定哪些布局在设置 UI 中可选。
// 恢复方法：从 git 或备份还原本文件。
inline constexpr LayoutInfo ALL[] = {
    {freeink::ui::KeyboardLayoutId::QwertyEn, Language::EN},
};
inline constexpr uint8_t COUNT = sizeof(ALL) / sizeof(ALL[0]);
static_assert(COUNT <= 16, "keyboard layout mask is uint16_t");

inline constexpr uint16_t bitAt(const uint8_t i) { return static_cast<uint16_t>(1u << i); }
// Symbol layers carry no Latin letters, so credentials and URLs require at
// least one Latin layout enabled. In this build only QwertyEn survives, so
// LATIN_BITS is just its bit.
inline constexpr uint16_t LATIN_BITS = bitAt(0);

uint16_t enabled();
freeink::ui::KeyboardLayoutId startingLayout();
freeink::ui::KeyboardLayoutId next(freeink::ui::KeyboardLayoutId current);

}  // namespace keyboard_layouts
