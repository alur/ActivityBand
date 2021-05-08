#pragma once

#include "Metric.hpp"

// Load on the most heavily loaded logical core. More indicative whether a single-threaded app is
// using a lot of CPU in systems with lots of cores.
class TopCoreUsageMetric : public Metric {
public:
  void Update() override;
  D2D1_COLOR_F LineColor() const override;
};