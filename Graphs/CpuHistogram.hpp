#pragma once

#include "../Windows.h"
#include "../MetricStore.hpp"
#include <d2d1.h>

#include <functional>

class CpuHistogram {
public:
  explicit CpuHistogram(const MetricStore &);
  HRESULT SetPosition(const D2D1_RECT_F &);

  void Paint() const;

  HRESULT CreateDeviceResources(ID2D1RenderTarget *);
  void DiscardDeviceResources();

private:
  void PaintLine(ID2D1Brush *brush, std::function<float(const MetricSnapshot&)>) const;
  void FillBetweenLines(ID2D1Brush *brush, std::function<float(const MetricSnapshot &)>,
    std::function<float(const MetricSnapshot &)>) const;

  D2D1_POINT_2F PointPosition(size_t, float) const;
  HRESULT MakeFillGeometry(std::function<float(const MetricSnapshot &)>,
    std::function<float(const MetricSnapshot &)>, ID2D1PathGeometry **) const;

private:
  const MetricStore &m_metricStore;

  D2D1_RECT_F m_borderPosition;
  D2D1_RECT_F m_position;
  D2D1_SIZE_F m_size;

  ID2D1RenderTarget *m_renderTarget = nullptr;
  ID2D1SolidColorBrush *m_allCoreBrush = nullptr;
  ID2D1SolidColorBrush *m_memoryBrush = nullptr;
  ID2D1SolidColorBrush *m_topCoreBrush = nullptr;
  ID2D1SolidColorBrush *m_borderBrush = nullptr;
};