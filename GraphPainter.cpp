#include "GraphPainter.hpp"
#include "Macros.h"

GraphPainter::~GraphPainter() {
  DiscardDeviceResources();
}

void GraphPainter::Paint() {
  D2D1_RECT_F updateRect;

  if (!GetUpdateRect(updateRect)) return;
  if (ReCreateDeviceResources() != S_OK) return;

  m_renderTarget->BeginDraw();
  m_renderTarget->PushAxisAlignedClip(&updateRect, D2D1_ANTIALIAS_MODE_ALIASED);
  m_renderTarget->Clear();

  // m_renderTarget->FillRectangle(m_graphPosition, m_backgroundBrush);
  // PaintGrid();
  m_renderTarget->DrawRectangle(m_graphPosition, m_backgroundBrush);

  for (const auto &painter : m_metricPainters) {
    painter.Paint(m_graphPosition);
  }

  m_renderTarget->PopAxisAlignedClip();

  // If EndDraw fails we need to recreate all device-dependent resources
  if (m_renderTarget->EndDraw() == D2DERR_RECREATE_TARGET) {
    DiscardDeviceResources();
  } else {
    ValidateRect(m_hwnd, nullptr);
  }
}

void GraphPainter::PaintGrid() const {
  D2D1_SIZE_F size = D2D1::SizeF(
    m_graphPosition.right - m_graphPosition.left,
    m_graphPosition.bottom - m_graphPosition.top);
  D2D1_SIZE_F cellSize = D2D1::SizeF(size.width / 6.0f, size.height / 6.0f);

  for (int i = 1; i < 6; i++) {
    float location = m_graphPosition.left + cellSize.width * i;
    m_renderTarget->DrawLine(
      D2D1::Point2(location, m_graphPosition.top),
      D2D1::Point2(location, m_graphPosition.bottom),
      m_gridBrush
    );
  }

  for (int i = 1; i < 6; i++) {
    float location = m_graphPosition.top + cellSize.height * i;
    m_renderTarget->DrawLine(
      D2D1::Point2(m_graphPosition.left, location),
      D2D1::Point2(m_graphPosition.right, location),
      m_gridBrush
    );
  }
}

void GraphPainter::SetWindow(HWND hwnd) {
  m_hwnd = hwnd;

  RECT rect;
  if (GetWindowRect(m_hwnd, &rect)) {
    UpdateWindowSize(D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top));
  }
}

void GraphPainter::UpdateWindowSize(const D2D1_SIZE_U &size) {
  m_windowSize = size;
  m_graphPosition = D2D1::RectF(0.0f, 5.0f, (FLOAT)size.width, (FLOAT)size.height - 5.0f);
  if (m_renderTarget) {
    m_renderTarget->Resize(size);
  }
}

void GraphPainter::UpdateMetrics(const std::vector<Metric *> &metrics) {
  m_metricPainters.clear();
  for (const auto &metric : metrics) {
    m_metricPainters.emplace_back(metric);
  }
}

HRESULT GraphPainter::ReCreateDeviceResources() {
  if (m_renderTarget) return S_OK;

  ID2D1Factory *pD2DFactory;
  D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};
#if defined(_DEBUG)
  d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif  
  HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory),
    &d2dFactoryOptions, reinterpret_cast<LPVOID *>(&pD2DFactory));

  if (SUCCEEDED(hr)) {
    hr = pD2DFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 96.0f, 96.0f
      ),
      D2D1::HwndRenderTargetProperties(m_hwnd, m_windowSize),
      &m_renderTarget
    );

    pD2DFactory->Release();
  }

  if (SUCCEEDED(hr)) {
    hr = m_renderTarget->CreateSolidColorBrush(
      D2D1::ColorF(D2D1::ColorF::WhiteSmoke, 0.7f), &m_backgroundBrush);
  }
  if (SUCCEEDED(hr)) {
    hr = m_renderTarget->CreateSolidColorBrush(
      D2D1::ColorF(D2D1::ColorF::CornflowerBlue, 0.7f), &m_gridBrush);
  }
  if (SUCCEEDED(hr)) {
    hr = m_renderTarget->CreateSolidColorBrush(
      D2D1::ColorF(D2D1::ColorF::CornflowerBlue, 0.7f), &m_borderBrush);
  }

  for (auto &painter : m_metricPainters) {
    if (SUCCEEDED(hr)) {
      hr = painter.CreateDeviceResources(m_renderTarget);
    }
  }

  if (FAILED(hr)) DiscardDeviceResources();

  return hr;
}

void GraphPainter::DiscardDeviceResources() {
  SAFE_RELEASE(m_borderBrush);
  SAFE_RELEASE(m_backgroundBrush);
  SAFE_RELEASE(m_gridBrush);
  SAFE_RELEASE(m_renderTarget);
  for (auto &painter : m_metricPainters) {
    painter.DiscardDeviceResources();
  }
}

bool GraphPainter::GetUpdateRect(D2D1_RECT_F &rect) const {
  RECT updateRect;
  if (::GetUpdateRect(m_hwnd, &updateRect, FALSE) == FALSE) return false;
  rect = D2D1::RectF(
    (FLOAT)updateRect.left, (FLOAT)updateRect.top,
    (FLOAT)updateRect.right, (FLOAT)updateRect.bottom);
  return true;
}