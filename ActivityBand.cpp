#include "ActivityBand.hpp"
#include "Macros.h"

#include "Metrics/RamMetric.hpp"
#include "Metrics/TotalCpuUsageMetric.hpp"

extern long objectCounter;
extern const CLSID CLSID_ActivityBand;
extern HMODULE module;

static constexpr wchar_t WINDOW_CLASS[] = L"ActivityBand";
static constexpr UINT_PTR IDT_UPDATE = 1;

HRESULT ActivityBand::StaticInit() {
  WNDCLASSEX wc = {};
  ZeroMemory(&wc, sizeof(WNDCLASSEX));
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_NOCLOSE;
  wc.lpfnWndProc = InitWindowProc;
  wc.cbWndExtra = sizeof(ActivityBand *);
  wc.hInstance = ::module;
  wc.lpszClassName = WINDOW_CLASS;
  if (RegisterClassEx(&wc) == NULL) {
    return HRESULT_FROM_WIN32(GetLastError());
  }
  return S_OK;
}

void ActivityBand::StaticTearDown() {
  UnregisterClass(WINDOW_CLASS, ::module);
}

ActivityBand::ActivityBand() {
  InterlockedIncrement(&::objectCounter);
  m_metrics.push_back(new TotalCpuUsageMetric());
  m_metrics.push_back(new RamMetric());
  m_graphPainter.UpdateMetrics(m_metrics);
}

ActivityBand::~ActivityBand() {
  for (auto *metric : m_metrics) delete metric;
  SAFE_RELEASE(m_site);
  if (m_hwnd) DestroyWindow(m_hwnd);
  InterlockedDecrement(&::objectCounter);
}

/// <summary>
/// IUnknown::AddRef
/// Increments the reference count for an interface on an object.
/// </summary>
STDMETHODIMP_(ULONG) ActivityBand::AddRef() {
  return InterlockedIncrement(&m_refCount);
}

/// <summary>
/// IUnknown::QueryInterface
/// Retrieves pointers to the supported interfaces on an object.
/// </summary>
STDMETHODIMP ActivityBand::QueryInterface(REFIID riid, LPVOID *ppvObject) {
  if (ppvObject == nullptr) return E_POINTER;

  if (riid == IID_IUnknown) {
    *ppvObject = (IUnknown *)(IDeskBand2 *)this;
  }
  else if (riid == IID_IOleWindow) {
    *ppvObject = (IOleWindow *)this;
  }
  else if (riid == IID_IDockingWindow) {
    *ppvObject = (IDockingWindow *)this;
  }
  else if (riid == IID_IDeskBand) {
    *ppvObject = (IDeskBand *)this;
  }
  else if (riid == IID_IDeskBand2) {
    *ppvObject = (IDeskBand2 *)this;
  }
  else if (riid == IID_IPersist) {
    *ppvObject = (IPersist *)this;
  }
  else if (riid == IID_IPersistStream) {
    *ppvObject = (IPersistStream *)this;
  }
  else if (riid == IID_IObjectWithSite) {
    *ppvObject = (IObjectWithSite *)this;
  }
  else {
    *ppvObject = nullptr;
    return E_NOINTERFACE;
  }

  AddRef();
  return S_OK;
}

/// <summary>
/// IUnknown::Release
/// Decrements the reference count for an interface on an object.
/// </summary>
STDMETHODIMP_(ULONG) ActivityBand::Release() {
  ULONG refCount = InterlockedDecrement(&m_refCount);
  if (refCount == 0) {
    delete this;
  }
  return refCount;
}

/// <summary>
/// IOleWindow::GetWindow
/// Retrieves a handle to one of the windows participating in in-place activation (frame, document,
/// parent, or in-place object window).
/// </summary>
STDMETHODIMP ActivityBand::GetWindow(HWND *pHwnd) {
  if (pHwnd == nullptr) return E_POINTER;
  if (m_hwnd == nullptr) return E_FAIL;
  *pHwnd = m_hwnd;
  return S_OK;
}

/// <summary>
/// IOleWindow::ContextSensitiveHelp
/// Determines whether context-sensitive help mode should be entered during an in-place activation
/// session.
/// </summary>
STDMETHODIMP ActivityBand::ContextSensitiveHelp(BOOL /* fEnterMode */) {
  return E_NOTIMPL;
}

/// <summary>
/// IDockingWindow::ShowDW
/// Instructs the docking window object to show or hide itself.
/// </summary>
STDMETHODIMP ActivityBand::ShowDW(BOOL fShow) {
  if (m_hwnd) {
    ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);
  }
  return S_OK;
}

/// <summary>
/// IDockingWindow::CloseDW
/// Notifies the docking window object that it is about to be removed from the frame. The docking
/// window object should save any persistent information at this time.
/// </summary>
STDMETHODIMP ActivityBand::CloseDW(DWORD /* dwReserved */) {
  if (m_hwnd) {
    ShowWindow(m_hwnd, SW_HIDE);
    DestroyWindow(m_hwnd);
    m_hwnd = nullptr;
  }
  return S_OK;
}

/// <summary>
/// IDockingWindow::ResizeBorderDW
/// Notifies the docking window object that the frame's border space has changed. In response to
/// this method, the IDockingWindow implementation must call SetBorderSpaceDW, even if no border
/// space is required or a change is not necessary.
/// </summary>
STDMETHODIMP ActivityBand::ResizeBorderDW(LPCRECT prcBorder, LPUNKNOWN punkToolbarSite, BOOL /* fReserved */) {
  UNREFERENCED_PARAMETER(prcBorder);
  UNREFERENCED_PARAMETER(punkToolbarSite);

  return E_NOTIMPL;
}

/// <summary>
/// IDeskBand::GetBandInfo
/// </summary>
STDMETHODIMP ActivityBand::GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO *pdbi) {
  UNREFERENCED_PARAMETER(dwBandID);
  UNREFERENCED_PARAMETER(dwViewMode);

  if (pdbi == nullptr) return E_POINTER;

  if (CHECK_FLAG(pdbi->dwMask, DBIM_MINSIZE)) {
    pdbi->ptMinSize = { 100, 30 };
  }
  if (CHECK_FLAG(pdbi->dwMask, DBIM_MAXSIZE)) {
    pdbi->ptMaxSize.y = -1;
  }
  if (CHECK_FLAG(pdbi->dwMask, DBIM_INTEGRAL)) {
    pdbi->ptIntegral.y = 1;
  }
  if (CHECK_FLAG(pdbi->dwMask, DBIM_ACTUAL)) {
    pdbi->ptActual = { 100, 30 };
  }
  if (CHECK_FLAG(pdbi->dwMask, DBIM_TITLE)) {
    // Don't show a title by removing this flag.
    pdbi->dwMask &= ~DBIM_TITLE;
  }
  if (CHECK_FLAG(pdbi->dwMask, DBIM_MODEFLAGS)) {
    pdbi->dwModeFlags = DBIMF_NORMAL | DBIMF_VARIABLEHEIGHT;
  }
  if (CHECK_FLAG(pdbi->dwMask, DBIM_BKCOLOR)) {
    // Use the default background color by removing this flag.   
    pdbi->dwMask &= ~DBIM_BKCOLOR;
  }

  return S_OK;
}

/// <summary>
/// IDeskBand2::CanRenderComposited
/// Indicates the deskband's ability to be displayed as translucent.
/// </summary>
STDMETHODIMP ActivityBand::CanRenderComposited(LPBOOL pfCanRenderComposited) {
  if (pfCanRenderComposited == nullptr) return E_POINTER;

  *pfCanRenderComposited = TRUE;
  return S_OK;
}

/// <summary>
/// IDeskBand2::SetCompositionState
/// Sets the composition state.
/// </summary>
STDMETHODIMP ActivityBand::SetCompositionState(BOOL fCompositionEnabled) {
  m_compositionEnabled = fCompositionEnabled;
  if (m_hwnd) {
    InvalidateRect(m_hwnd, NULL, TRUE);
    UpdateWindow(m_hwnd);
  }
  return S_OK;
}

/// <summary>
/// IDeskBand2::GetCompositionState
/// Gets the composition state.
/// </summary>
STDMETHODIMP ActivityBand::GetCompositionState(LPBOOL pfCompositionEnabled) {
  if (pfCompositionEnabled == nullptr) return E_POINTER;

  *pfCompositionEnabled = m_compositionEnabled;
  return S_OK;
}

/// <summary>
/// IObjectWithSite::SetSite
/// Enables a container to pass an object a pointer to the interface for its site.
/// </summary>
STDMETHODIMP ActivityBand::SetSite(LPUNKNOWN pUnkSite) {
  if (m_site != nullptr) m_site->Release();

  // TODO:: if (m_hwnd) ...

  if (pUnkSite != nullptr) pUnkSite->AddRef();
  m_site = pUnkSite;

  if (pUnkSite == nullptr) return S_OK;


  IOleWindow *pow;
  HRESULT hr = pUnkSite->QueryInterface(IID_IOleWindow, reinterpret_cast<LPVOID*>(&pow));
  if (SUCCEEDED(hr)) {
    HWND parent;
    hr = pow->GetWindow(&parent);
    if (SUCCEEDED(hr)) {
      m_hwnd = CreateWindowExW(0, WINDOW_CLASS, nullptr,
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 0, 0, parent, nullptr, ::module, this);
      if (m_hwnd == nullptr) {
        hr = E_FAIL;
      }
      SetTimer(m_hwnd, IDT_UPDATE, 1000, nullptr);
      m_graphPainter.SetWindow(m_hwnd);
    }
    pow->Release();
  }

  return hr;
}

/// <summary>
/// IObjectWithSite::GetSite
/// Retrieves the latest site passed using SetSite.
/// </summary>
STDMETHODIMP ActivityBand::GetSite(REFIID riid, LPVOID *ppvSite) {
  if (ppvSite == nullptr) return E_POINTER;
  *ppvSite = nullptr;

  if (m_site == nullptr) return E_FAIL;
  return m_site->QueryInterface(riid, ppvSite);
}

/// <summary>
/// IPersist::GetClassID
/// Retrieves the class identifier (CLSID) of the object.
/// </summary>
STDMETHODIMP ActivityBand::GetClassID(LPCLSID pClassID) {
  if (pClassID == nullptr) return E_POINTER;

  *pClassID = CLSID_ActivityBand;
  return S_OK;
}

/// <summary>
/// IPersistStream::IsDirty
/// Determines whether an object has changed since it was last saved to its stream.
/// </summary>
STDMETHODIMP ActivityBand::IsDirty() {
  return S_FALSE;
}

/// <summary>
/// IPersistStream::Load
/// Initializes an object from the stream where it was saved previously.
/// </summary>
STDMETHODIMP ActivityBand::Load(IStream *pStm) {
  UNREFERENCED_PARAMETER(pStm);

  return S_OK;
}

/// <summary>
/// IPersistStream::Save
/// Saves an object to the specified stream.
/// </summary>
STDMETHODIMP ActivityBand::Save(IStream *pStm, BOOL fClearDirty) {
  UNREFERENCED_PARAMETER(pStm);
  UNREFERENCED_PARAMETER(fClearDirty);

  return S_OK;
}

/// <summary>
/// IPersistStream::GetSizeMax
/// Retrieves the size of the stream needed to save the object.
/// </summary>
STDMETHODIMP ActivityBand::GetSizeMax(PULARGE_INTEGER pcbSize) {
  *pcbSize = { 0 };
  return S_OK;
}

LRESULT ActivityBand::HandleMessage(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_PAINT:
    m_graphPainter.Paint();
    return 0;

  case WM_SIZE:
    m_graphPainter.UpdateWindowSize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam)));
    break;

  case WM_ERASEBKGND:
    if (m_compositionEnabled) {
      return 1;
    }
    break;

  case WM_TIMER:
    switch (wParam) {
    case IDT_UPDATE:
      for (auto *metric : m_metrics) {
        metric->Update();
      }
      InvalidateRect(window, nullptr, false);
      UpdateWindow(window);
      break;
    }
    break;
  }

  return DefWindowProc(window, msg, wParam, lParam);
}

LRESULT WINAPI ActivityBand::ExternWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
  return ((ActivityBand *)GetWindowLongPtr(window, 0))->HandleMessage(window, message, wParam, lParam);
}

LRESULT WINAPI ActivityBand::InitWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WM_CREATE) {
    SetWindowLongPtr(window, 0, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
    SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)&ActivityBand::ExternWindowProc);
    return ExternWindowProc(window, message, wParam, lParam);
  }
  return DefWindowProc(window, message, wParam, lParam);
}