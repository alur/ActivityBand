#include "Metric.hpp"

Metric::~Metric() {}

const std::deque<float> &Metric::GetSamples() const {
  return m_samples;
}

uint32_t Metric::GetWindowSize() const {
  return 30;
}

void Metric::AddSample(float point) {
  m_samples.push_front(point);
  if (m_samples.size() > GetWindowSize()) m_samples.pop_back();
}