#include "RamMetric.hpp"

#include "../Windows.h"

void RamMetric::Update() {
  MEMORYSTATUSEX buffer;
  buffer.dwLength = sizeof(buffer);
  GlobalMemoryStatusEx(&buffer);
  AddSample(1.0f - float(double(buffer.ullAvailPhys) / double(buffer.ullTotalPhys)));
}

D2D1_COLOR_F RamMetric::LineColor() const {
  return D2D1::ColorF(D2D1::ColorF::DarkRed);
}