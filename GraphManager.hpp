#pragma once

#include "Graphs/CpuHistogram.hpp"

#include "Windows.h"
#include <d2d1.h>

#include <vector>

class GraphManager {
public:
  explicit GraphManager(const MetricStore &);
  ~GraphManager();

public:
  void Paint();
  void SetWindow(HWND);
  void UpdateWindowSize(const D2D1_SIZE_U &);

  HRESULT ReCreateDeviceResources();
  void DiscardDeviceResources();

private:
  bool GetUpdateRect(D2D1_RECT_F &) const;

private:
  HWND m_hwnd = nullptr;
  D2D1_SIZE_U m_windowSize;
  D2D1_RECT_F m_graphPosition; // Within the window.

  ID2D1HwndRenderTarget *m_renderTarget = nullptr;

  const MetricStore &m_metricStore;
  std::vector<CpuHistogram *> m_graphs;
};