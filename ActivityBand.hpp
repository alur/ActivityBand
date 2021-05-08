#pragma once

#include "GraphPainter.hpp"
#include "Metrics/Metric.hpp"

#include "Windows.h"

#include <shobjidl.h>
#include <d2d1.h>

#include <vector>

class ActivityBand : public IDeskBand2, IObjectWithSite, IPersistStream {
public:
  explicit ActivityBand();

  static HRESULT StaticInit();
  static void StaticTearDown();

private:
  virtual ~ActivityBand();

  static LRESULT WINAPI ExternWindowProc(HWND, UINT, WPARAM, LPARAM);
  static LRESULT WINAPI InitWindowProc(HWND, UINT, WPARAM, LPARAM);

  LRESULT HandleMessage(HWND, UINT, WPARAM, LPARAM);

public:
  // IUnknown
  STDMETHOD_(ULONG, AddRef) (void) override;
  STDMETHOD(QueryInterface) (REFIID, LPVOID*) override;
  STDMETHOD_(ULONG, Release) (void) override;

  // IOleWindow
  STDMETHOD(GetWindow) (HWND*) override;
  STDMETHOD(ContextSensitiveHelp) (BOOL) override;

  // IDockingWindow
  STDMETHOD(ShowDW) (BOOL) override;
  STDMETHOD(CloseDW) (DWORD) override;
  STDMETHOD(ResizeBorderDW) (LPCRECT, LPUNKNOWN, BOOL) override;

  // IDeskBand
  STDMETHOD(GetBandInfo) (DWORD, DWORD, DESKBANDINFO*) override;

  // IDeskBand2
  STDMETHOD(CanRenderComposited) (LPBOOL) override;
  STDMETHOD(SetCompositionState) (BOOL) override;
  STDMETHOD(GetCompositionState) (LPBOOL) override;

  // IObjectWithSite
  STDMETHOD(SetSite) (LPUNKNOWN) override;
  STDMETHOD(GetSite) (REFIID riid, LPVOID*) override;

  // IPersist 
  STDMETHOD(GetClassID) (LPCLSID) override;

  // IPersistStream
  STDMETHOD(IsDirty) (void) override;
  STDMETHOD(Load) (IStream*) override;
  STDMETHOD(Save) (IStream*, BOOL) override;
  STDMETHOD(GetSizeMax) (PULARGE_INTEGER) override;

private:
  HWND m_hwnd = nullptr;
  ULONG m_refCount = 1;
  LPUNKNOWN m_site = nullptr;
  BOOL m_compositionEnabled = TRUE;
  GraphPainter m_graphPainter;

  std::vector<Metric *> m_metrics;
};