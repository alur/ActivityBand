#include "TotalCpuUsageMetric.hpp"

#include "../Windows.h"

TotalCpuUsageMetric::TotalCpuUsageMetric() {
  GetTimes(&m_lastIdleTime, &m_lastKernelTime, &m_lastUserTime);
}

void TotalCpuUsageMetric::Update() {
  uint64_t idleTime, kernelTime, userTime;
  GetTimes(&idleTime, &kernelTime, &userTime);

  double idleDelta = double(idleTime) - double(m_lastIdleTime);
  double systemDelta = double(kernelTime) + double(userTime)
    - double(m_lastUserTime) - double(m_lastKernelTime);

  AddSample(1.0f - float(idleDelta / systemDelta));

  m_lastIdleTime = idleTime;
  m_lastKernelTime = kernelTime;
  m_lastUserTime = userTime;
}

void TotalCpuUsageMetric::GetTimes(uint64_t *idle, uint64_t *kernel, uint64_t *user) {
  FILETIME idleTime, kernelTime, userTime;
  // TODO::Propagate failure up.
  if (GetSystemTimes(&idleTime, &kernelTime, &userTime) != FALSE) {
    *idle = ULARGE_INTEGER{ idleTime.dwLowDateTime, idleTime.dwHighDateTime }.QuadPart;
    *kernel = ULARGE_INTEGER{ kernelTime.dwLowDateTime, kernelTime.dwHighDateTime }.QuadPart;
    *user = ULARGE_INTEGER{ userTime.dwLowDateTime, userTime.dwHighDateTime }.QuadPart;
  }
}

D2D1_COLOR_F TotalCpuUsageMetric::LineColor() const {
  return D2D1::ColorF(D2D1::ColorF::Blue);
}