#pragma once

#include "../Windows.h"
#include "../MetricStore.hpp"
#include <d2d1.h>

#include <functional>

class CpuHistogram {
public:
  explicit CpuHistogram(const MetricStore &);

  void Paint(const D2D1_RECT_F &) const;

  HRESULT CreateDeviceResources(ID2D1RenderTarget *);
  void DiscardDeviceResources();

private:
  void PaintLine(const D2D1_RECT_F &, std::function<float(const MetricSnapshot&)>, ID2D1Brush *) const;

private:
  const MetricStore &m_metricStore;
  ID2D1RenderTarget *m_renderTarget = nullptr;
  ID2D1SolidColorBrush *m_allCoreBrush = nullptr;
  ID2D1SolidColorBrush *m_memoryBrush = nullptr;
  ID2D1SolidColorBrush *m_topCoreBrush = nullptr;
};