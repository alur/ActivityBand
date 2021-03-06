#include "GraphManager.hpp"
#include "Macros.h"

GraphManager::GraphManager(const MetricStore &store) : m_metricStore(store) {
  m_graphs.push_back(new CpuHistogram(store));
}

GraphManager::~GraphManager() {
  DiscardDeviceResources();
  for (auto graph : m_graphs) delete graph;
}

void GraphManager::Paint() {
  D2D1_RECT_F updateRect;

  if (!GetUpdateRect(updateRect)) return;
  if (ReCreateDeviceResources() != S_OK) return;

  m_renderTarget->BeginDraw();
  m_renderTarget->PushAxisAlignedClip(&updateRect, D2D1_ANTIALIAS_MODE_ALIASED);
  m_renderTarget->Clear();

  for (const auto graph : m_graphs) {
    graph->Paint();
  }

  m_renderTarget->PopAxisAlignedClip();

  // If EndDraw fails we need to recreate all device-dependent resources
  if (m_renderTarget->EndDraw() == D2DERR_RECREATE_TARGET) {
    DiscardDeviceResources();
  }
  else {
    ValidateRect(m_hwnd, nullptr);
  }
}

void GraphManager::SetWindow(HWND hwnd) {
  m_hwnd = hwnd;

  RECT rect;
  if (GetWindowRect(m_hwnd, &rect)) {
    UpdateWindowSize(D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top));
  }
}

void GraphManager::UpdateWindowSize(const D2D1_SIZE_U &size) {
  m_windowSize = size;
  m_graphPosition = D2D1::RectF(0.0f, 5.0f, (FLOAT)size.width, (FLOAT)size.height - 5.0f);
  if (m_renderTarget) {
    m_renderTarget->Resize(size);
  }
  for (const auto graph : m_graphs) {
    graph->SetPosition(m_graphPosition);
  }
}

HRESULT GraphManager::ReCreateDeviceResources() {
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

  for (const auto graph : m_graphs) {
    if (SUCCEEDED(hr)) {
      hr = graph->CreateDeviceResources(m_renderTarget);
    }
  }

  if (FAILED(hr)) DiscardDeviceResources();

  return hr;
}

void GraphManager::DiscardDeviceResources() {
  for (const auto graph : m_graphs) {
    graph->DiscardDeviceResources();
  }
  SAFE_RELEASE(m_renderTarget);
}

bool GraphManager::GetUpdateRect(D2D1_RECT_F &rect) const {
  RECT updateRect;
  if (::GetUpdateRect(m_hwnd, &updateRect, FALSE) == FALSE) return false;
  rect = D2D1::RectF(
    (FLOAT)updateRect.left, (FLOAT)updateRect.top,
    (FLOAT)updateRect.right, (FLOAT)updateRect.bottom);
  return true;
}