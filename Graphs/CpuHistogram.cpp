#include "CpuHistogram.hpp"

#include "../Macros.h"

#include <algorithm>

CpuHistogram::CpuHistogram(const MetricStore &metricStore) : m_metricStore(metricStore) {}

void CpuHistogram::Paint(const D2D1_RECT_F &graphPosition) const {
  if (m_metricStore.GetSnapshots().empty()) return;

  PaintLine(graphPosition, [](const MetricSnapshot &snapshot) {
    return *std::max_element(snapshot.cpu_core_usage.begin(), snapshot.cpu_core_usage.end());
  }, m_topCoreBrush);
  PaintLine(graphPosition, [](const MetricSnapshot &snapshot) {
    return snapshot.cpu_usage;
  }, m_allCoreBrush);
  PaintLine(graphPosition, [](const MetricSnapshot &snapshot) {
    return 1.0f - float(double(snapshot.memory.ullAvailPhys) / double(snapshot.memory.ullTotalPhys));
  }, m_memoryBrush);
}

void CpuHistogram::PaintLine(const D2D1_RECT_F &graphPosition, std::function<float(const MetricSnapshot &)> metric, ID2D1Brush *brush) const {
  D2D1_SIZE_F size = D2D1::SizeF(
    graphPosition.right - graphPosition.left,
    graphPosition.bottom - graphPosition.top);

  const auto &points = m_metricStore.GetSnapshots();
  for (size_t i = 0; i < points.size() - 1; ++i) {
    m_renderTarget->DrawLine(
      D2D1::Point2(
        graphPosition.right - size.width * (i / float(30)),
        graphPosition.bottom - size.height * metric(points[i])),
      D2D1::Point2(
        graphPosition.right - size.width * ((i + 1) / float(30)),
        graphPosition.bottom - size.height * metric(points[i + 1])),
      brush,
      2.0f
    ); 
  }
}

HRESULT CpuHistogram::CreateDeviceResources(ID2D1RenderTarget *renderTarget) {
  m_renderTarget = renderTarget;
  renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue, 0.7f), &m_allCoreBrush);
  renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 0.7f), &m_memoryBrush);
  return renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue, 0.7f), &m_topCoreBrush);
}

void CpuHistogram::DiscardDeviceResources() {
  m_renderTarget = nullptr;
  SAFE_RELEASE(m_allCoreBrush);
  SAFE_RELEASE(m_memoryBrush);
  SAFE_RELEASE(m_topCoreBrush);
}