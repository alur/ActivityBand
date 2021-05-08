#include "MetricPainter.hpp"

#include "Macros.h"

MetricPainter::MetricPainter(const Metric *metric) : m_metric(metric) {}

void MetricPainter::Paint(const D2D1_RECT_F &graphPosition) const {
  D2D1_SIZE_F size = D2D1::SizeF(
    graphPosition.right - graphPosition.left,
    graphPosition.bottom - graphPosition.top);

  const auto &points = m_metric->GetSamples();
  if (points.empty()) return;
  for (size_t i = 0; i < points.size() - 1; ++i) {
    m_renderTarget->DrawLine(
      D2D1::Point2(
        graphPosition.right - size.width * (i / float(m_metric->GetWindowSize())),
        graphPosition.bottom - size.height * points[i]),
      D2D1::Point2(
        graphPosition.right - size.width * ((i + 1) / float(m_metric->GetWindowSize())),
        graphPosition.bottom - size.height * points[i + 1]),
      m_brush,
      2.0f
    );
  }
}

HRESULT MetricPainter::CreateDeviceResources(ID2D1RenderTarget *renderTarget) {
  m_renderTarget = renderTarget;
  return renderTarget->CreateSolidColorBrush(m_metric->LineColor(), &m_brush);
}

void MetricPainter::DiscardDeviceResources() {
  m_renderTarget = nullptr;
  SAFE_RELEASE(m_brush);
}