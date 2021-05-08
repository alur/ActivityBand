#include "TopCoreUsageMetric.hpp"

void TopCoreUsageMetric::Update() {}

D2D1_COLOR_F TopCoreUsageMetric::LineColor() const {
  return D2D1::ColorF(D2D1::ColorF::IndianRed);
}