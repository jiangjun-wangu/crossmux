#include "FullScreenMessageActivity.h"

#include <Arduino.h>
#include <GfxRenderer.h>
#include <Logging.h>

#include "MappedInputManager.h"
#include "fontIds.h"

void FullScreenMessageActivity::onEnter() {
  Activity::onEnter();

  const auto height = renderer.getLineHeight(UI_10_FONT_ID);
  const auto top = (renderer.getScreenHeight() - height) / 2;

  renderer.clearScreen();
  renderer.drawCenteredText(UI_10_FONT_ID, top, text.c_str(), true, style);
  renderer.displayBuffer(refreshMode);
}

void FullScreenMessageActivity::loop() {
  // Any button press reboots, so the user can insert the SD card and retry
  // instead of being stuck on the error screen.
  using B = MappedInputManager::Button;
  constexpr B kButtons[] = {B::Back,   B::Confirm, B::Left,  B::Right,
                            B::Up,     B::Down,    B::Power, B::PageBack,
                            B::PageForward};
  for (const B b : kButtons) {
    if (mappedInput.wasPressed(b)) {
      LOG_INF("FSM", "Reboot requested from full-screen message");
      ESP.restart();
    }
  }
}
