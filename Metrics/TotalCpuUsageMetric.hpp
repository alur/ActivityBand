#pragma once

#include "Metric.hpp"

// Combined load on all logical cores.
class TotalCpuUsageMetric : public Metric {
public:
  explicit TotalCpuUsageMetric();

public:
  void Update() override;
  D2D1_COLOR_F LineColor() const override;

private:
  static void GetTimes(uint64_t *, uint64_t *, uint64_t *);

private:
  uint64_t m_lastIdleTime, m_lastUserTime, m_lastKernelTime;
};