#include "MetricStore.hpp"

#include <pdh.h>
#include <pdhmsg.h>
#include <strsafe.h>
#include <thread>

MetricStore::MetricStore() {
  GetCpuTimes(&m_lastIdleTime, &m_lastKernelTime, &m_lastUserTime);
  PdhOpenQuery(nullptr, 1, &m_query);

  uint32_t coreCount = std::thread::hardware_concurrency();
  m_processorTimeCounters.resize(coreCount);
  WCHAR counterPath[255];
  for (uint32_t i = 0; i < coreCount; ++i) {
    StringCchPrintf(counterPath, 255, L"\\Processor(%d)\\%% Processor Time", i);
    PdhAddCounter(m_query, counterPath, i, &m_processorTimeCounters[i]);
  }
}

MetricStore::~MetricStore() {
  PdhCloseQuery(m_query);
}

HRESULT MetricStore::TakeSnapshot() {
  MetricSnapshot snapshot;

  snapshot.memory.dwLength = sizeof(snapshot.memory);
  GlobalMemoryStatusEx(&snapshot.memory); // TODO::Check return

  GetTotalCpuMetric(snapshot);

  if (PdhCollectQueryData(m_query) == ERROR_SUCCESS) {
    PDH_FMT_COUNTERVALUE value;
    for (const auto counter : m_processorTimeCounters) {
      PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &value);
      //PdhGetRawCounterValue(counter, nullptr, &value);
      snapshot.cpu_core_usage.push_back(float(value.doubleValue / 100.0));
    }
  }

  m_snapshots.push_front(snapshot);
  if (m_snapshots.size() > 30) m_snapshots.pop_back();

  return S_OK;
}

const std::deque<MetricSnapshot> &MetricStore::GetSnapshots() const {
  return m_snapshots;
}

HRESULT MetricStore::GetTotalCpuMetric(MetricSnapshot &snapshot) {
  uint64_t idleTime, kernelTime, userTime;
  GetCpuTimes(&idleTime, &kernelTime, &userTime);

  double idleDelta = double(idleTime) - double(m_lastIdleTime);
  double systemDelta = double(kernelTime) + double(userTime)
    - double(m_lastUserTime) - double(m_lastKernelTime);

  snapshot.cpu_usage = 1.0f - float(idleDelta / systemDelta);

  m_lastIdleTime = idleTime;
  m_lastKernelTime = kernelTime;
  m_lastUserTime = userTime;

  return S_OK;
}

void MetricStore::GetCpuTimes(uint64_t *idle, uint64_t *kernel, uint64_t *user) {
  FILETIME idleTime, kernelTime, userTime;
  // TODO::Propagate failure up.
  if (GetSystemTimes(&idleTime, &kernelTime, &userTime) != FALSE) {
    *idle = ULARGE_INTEGER{ idleTime.dwLowDateTime, idleTime.dwHighDateTime }.QuadPart;
    *kernel = ULARGE_INTEGER{ kernelTime.dwLowDateTime, kernelTime.dwHighDateTime }.QuadPart;
    *user = ULARGE_INTEGER{ userTime.dwLowDateTime, userTime.dwHighDateTime }.QuadPart;
  }
}