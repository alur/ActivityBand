#pragma once

#include "Windows.h"

#include <deque>
#include <Pdh.h>
#include <vector>

struct MetricSnapshot {
  float cpu_usage;
  MEMORYSTATUSEX memory;
  std::vector<float> cpu_core_usage; // CPU usage of each individual core.
};

class MetricStore {
public:
  MetricStore();
  ~MetricStore();

  HRESULT TakeSnapshot();
  const std::deque<MetricSnapshot> &GetSnapshots() const;

private:
  HRESULT GetTotalCpuMetric(MetricSnapshot &);
  static void GetCpuTimes(uint64_t *, uint64_t *, uint64_t *);

private:
  std::deque<MetricSnapshot> m_snapshots;

  uint64_t m_lastIdleTime, m_lastUserTime, m_lastKernelTime;

  PDH_HQUERY m_query;
  // "% Processor Time" counters for each core.
  std::vector<PDH_HQUERY> m_processorTimeCounters;
};