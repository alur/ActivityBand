#include "CpuHistogram.hpp"

#include "../Macros.h"

#include <algorithm>

CpuHistogram::CpuHistogram(const MetricStore &metricStore) : m_metricStore(metricStore) {}

HRESULT CpuHistogram::SetPosition(const D2D1_RECT_F &position) {
  // Reserve space for the border
  m_position = D2D1::RectF(
    position.left + 0.5f, position.top + 0.5f,
    position.right - 0.5f, position.bottom - 0.5f);
  m_size = D2D1::SizeF(
    m_position.right - m_position.left,
    m_position.bottom - m_position.top);
  m_borderPosition = position;

  return S_OK;
}

void CpuHistogram::Paint() const {
  if (m_metricStore.GetSnapshots().empty()) return;

  m_renderTarget->DrawRectangle(m_borderPosition, m_borderBrush);

  PaintLine(m_topCoreBrush, [](const MetricSnapshot &snapshot) {
    return *std::max_element(snapshot.cpu_core_usage.begin(), snapshot.cpu_core_usage.end());
    });
  PaintLine(m_allCoreBrush, [](const MetricSnapshot &snapshot) {
    return snapshot.cpu_usage;
  });

  FillBetweenLines(m_allCoreBrush,
    [](const MetricSnapshot &snapshot) {
      return snapshot.cpu_usage;
    },
    [](const MetricSnapshot &) {
      return 0.0f;
    });
  FillBetweenLines(m_topCoreBrush,
    [](const MetricSnapshot &snapshot) {
      return *std::max_element(snapshot.cpu_core_usage.begin(), snapshot.cpu_core_usage.end());
    },
    [](const MetricSnapshot &snapshot) {
      return snapshot.cpu_usage;
    });

  PaintLine(m_memoryBrush, [](const MetricSnapshot &snapshot) {
    return 1.0f - float(double(snapshot.memory.ullAvailPhys) / double(snapshot.memory.ullTotalPhys));
  });
}

void CpuHistogram::PaintLine(ID2D1Brush *brush, std::function<float(const MetricSnapshot &)> metric) const {
  const auto &points = m_metricStore.GetSnapshots();
  for (size_t i = 0; i < points.size() - 1; ++i) {
    m_renderTarget->DrawLine(
      PointPosition(i, metric(points[i])),
      PointPosition(i + 1, metric(points[i + 1])),
      brush,
      2.0f);
  }
}

void CpuHistogram::FillBetweenLines(ID2D1Brush *brush, std::function<float(const MetricSnapshot &)> metric1,
  std::function<float(const MetricSnapshot &)> metric2) const {
  // TODO::Create the geometry outside the painting process.
  ID2D1PathGeometry *geometry;
  if (SUCCEEDED(MakeFillGeometry(metric1, metric2, &geometry))) {
    m_renderTarget->FillGeometry(geometry, brush);
    geometry->Release();
  }
}

D2D1_POINT_2F CpuHistogram::PointPosition(size_t index, float metric) const {
  return D2D1::Point2(
    m_position.right - m_size.width * (index / (float(m_metricStore.GetMaxSnapshots()) - 1.0f)),
    m_position.bottom - m_size.height * metric
  );
}

HRESULT CpuHistogram::MakeFillGeometry(std::function<float(const MetricSnapshot &)> metric1,
  std::function<float(const MetricSnapshot &)> metric2, ID2D1PathGeometry **geometry) const {

  ID2D1Factory *factory;
  m_renderTarget->GetFactory(&factory);

  HRESULT hr = factory->CreatePathGeometry(geometry);
  if (SUCCEEDED(hr)) {
    ID2D1GeometrySink *sink;
    hr = (*geometry)->Open(&sink);
    if (SUCCEEDED(hr)) {
      const auto &points = m_metricStore.GetSnapshots();
      sink->BeginFigure(PointPosition(0, metric1(points[0])), D2D1_FIGURE_BEGIN_FILLED);
      for (size_t i = 0; i < points.size(); ++i) {
        sink->AddLine(PointPosition(i, metric2(points[i])));
      }
      for (size_t i = points.size() - 1; i > 0; --i) {
        sink->AddLine(PointPosition(i, metric1(points[i])));
      }
      sink->EndFigure(D2D1_FIGURE_END_CLOSED);
      hr = sink->Close();
      sink->Release();
    }
  }

  return hr;
}

HRESULT CpuHistogram::CreateDeviceResources(ID2D1RenderTarget *renderTarget) {
  m_renderTarget = renderTarget;

  HRESULT hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue, 0.7f), &m_allCoreBrush);
  if (SUCCEEDED(hr)) {
    hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 0.7f), &m_memoryBrush);
  }
  if (SUCCEEDED(hr)) {
    hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue, 0.7f), &m_topCoreBrush);
  }
  if (SUCCEEDED(hr)) {
    hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::WhiteSmoke, 0.7f), &m_borderBrush);
  }

  return hr;
}

void CpuHistogram::DiscardDeviceResources() {
  m_renderTarget = nullptr;
  SAFE_RELEASE(m_allCoreBrush);
  SAFE_RELEASE(m_memoryBrush);
  SAFE_RELEASE(m_topCoreBrush);
  SAFE_RELEASE(m_borderBrush);
}