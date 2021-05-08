#pragma once

#include "Metrics/Metric.hpp"

class MetricPainter {
public:
  explicit MetricPainter(const Metric *);

  void Paint(const D2D1_RECT_F &) const;

  HRESULT CreateDeviceResources(ID2D1RenderTarget *);
  void DiscardDeviceResources();

private:
  const Metric *m_metric;
  ID2D1RenderTarget *m_renderTarget = nullptr;
  ID2D1SolidColorBrush *m_brush = nullptr;
};