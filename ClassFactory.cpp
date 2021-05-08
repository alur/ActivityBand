#include "ActivityBand.hpp"
#include "ClassFactory.hpp"

extern long objectCounter;
extern const CLSID CLSID_ActivityBand;

ClassFactory::ClassFactory() {
  InterlockedIncrement(&::objectCounter);
}

ClassFactory::~ClassFactory() {
  InterlockedDecrement(&::objectCounter);
}

/// <summary>
/// IUnknown::AddRef
/// Increments the reference count for an interface on an object.
/// </summary>
STDMETHODIMP_(ULONG) ClassFactory::AddRef() {
  return InterlockedIncrement(&m_refCount);
}

/// <summary>
/// IUnknown::QueryInterface
/// Retrieves pointers to the supported interfaces on an object.
/// </summary>
STDMETHODIMP ClassFactory::QueryInterface(REFIID riid, LPVOID *ppvObject) {
  if (ppvObject == nullptr) return E_POINTER;

  if (riid == IID_IUnknown) {
    *ppvObject = (IUnknown *)this;
  }
  else if (riid == IID_IClassFactory) {
    *ppvObject = (IClassFactory *)this;
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
STDMETHODIMP_(ULONG) ClassFactory::Release() {
  ULONG refCount = InterlockedDecrement(&m_refCount);
  if (refCount == 0) {
    delete this;
  }
  return refCount;
}

/// <summary>
/// IClassFactory::CreateInstance
/// Creates an uninitialized object.
/// </summary>
STDMETHODIMP ClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObject) {
  if (pUnkOuter != nullptr) return CLASS_E_NOAGGREGATION;
  if (ppvObject == nullptr) return E_POINTER;

  if (riid == IID_IDeskBand) {
    *ppvObject = (IDeskBand *)(new ActivityBand());
  }
  else if (riid == IID_IDeskBand2) {
    *ppvObject = (IDeskBand2 *)(new ActivityBand());
  }
  else {
    *ppvObject = nullptr;
    return E_NOINTERFACE;
  }

  if (*ppvObject == nullptr) {
    return E_OUTOFMEMORY;
  }

  return S_OK;
}

/// <summary>
/// IClassFactory::LockServer
/// Locks an object application open in memory. This enables instances to be created more quickly.
/// </summary>
STDMETHODIMP ClassFactory::LockServer(BOOL fLock) {
  fLock ? InterlockedIncrement(&::objectCounter) : InterlockedDecrement(&::objectCounter);
  return S_OK;
}