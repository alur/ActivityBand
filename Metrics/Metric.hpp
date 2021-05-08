#pragma once

#include <deque>
#include "../Windows.h"
#include <d2d1.h>

class Metric {
public:
  virtual ~Metric();

  virtual void Update() = 0;
  virtual D2D1_COLOR_F LineColor() const = 0;

  // Returns the historical data of this metric as a list of floats in the range [0, 1].
  const std::deque<float> &GetSamples() const;

  // The amount of historical entries stored in samples.
  uint32_t GetWindowSize() const;

protected:
  void AddSample(float);

private:
  std::deque<float> m_samples;
};