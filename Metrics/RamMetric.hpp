#pragma once

#include "Metric.hpp"

// How much of the physical memory is used.
class RamMetric : public Metric {
public:
  void Update() override;
  D2D1_COLOR_F LineColor() const override;
};