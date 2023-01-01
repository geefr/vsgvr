#pragma once

#include <cstdint>
#include <chrono>

class SampleImGUIComponent
{
public:
  SampleImGUIComponent();
  bool operator()();

  float someFloatVal = 0.0f;
  uint32_t buttonPressed = 0;
private:
  bool _windowOpen = true;

  uint32_t _frameI = 0;
};
