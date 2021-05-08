#pragma once

#include "MetricPainter.hpp"

#include "Windows.h"
#include <d2d1.h>

#include <vector>

class GraphPainter { 
public:
  ~GraphPainter();

public:
  void Paint();
  void SetWindow(HWND);
  void UpdateWindowSize(const D2D1_SIZE_U &);

  void UpdateMetrics(const std::vector<Metric*> &);

  HRESULT ReCreateDeviceResources();
  void DiscardDeviceResources();

private:
  void PaintGrid() const;

  bool GetUpdateRect(D2D1_RECT_F &) const;

private:
  HWND m_hwnd = nullptr;
  D2D1_SIZE_U m_windowSize;
  D2D1_RECT_F m_graphPosition; // Within the window.

  std::vector<MetricPainter> m_metricPainters;
  ID2D1HwndRenderTarget *m_renderTarget = nullptr;
  ID2D1SolidColorBrush *m_backgroundBrush = nullptr;
  ID2D1SolidColorBrush *m_borderBrush = nullptr;
  ID2D1SolidColorBrush *m_gridBrush = nullptr;
};