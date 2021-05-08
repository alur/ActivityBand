#pragma once

#include "Windows.h"
#include <Unknwn.h>

class ClassFactory : public IClassFactory {
public:
  explicit ClassFactory();

private:
  virtual ~ClassFactory();

public:
  // IUnknown
  STDMETHOD_(ULONG, AddRef)() override;
  STDMETHOD(QueryInterface) (REFIID, LPVOID*) override;
  STDMETHOD_(ULONG, Release)() override;

  // IClassFactory
  STDMETHOD(CreateInstance) (LPUNKNOWN, REFIID, LPVOID*) override;
  STDMETHOD(LockServer) (BOOL) override;

private:
  ULONG m_refCount = 1;
};