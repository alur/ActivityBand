#include "Registration.h"

#include <shellapi.h>
#include <ShlObj.h>
#include <strsafe.h>

extern HMODULE module;
extern const CLSID CLSID_ActivityBand;

/// <summary>
/// Registers this DLL, called by regsvr32.
/// </summary>
STDAPI DllRegisterServer() {
  if (!IsElevated()) return RunElevated(L"DllRegisterServer");

  DeleteRegKeys(); // Get rid of any old settings.

  HRESULT hr = RegisterDLL();
  if (SUCCEEDED(hr)) hr = RegisterComCat();

  // Cleanup on failure.
  if (FAILED(hr)) DeleteRegKeys();

  return hr;
}

/// <summary>
/// Unregisters this DLL, called by regsvr32 /u.
/// </summary>
STDAPI DllUnregisterServer() {
  if (!IsElevated()) return RunElevated(L"DllUnregisterServer");

  HRESULT hr = UnRegisterComCat();
  HRESULT hr2 = DeleteRegKeys();

  return FAILED(hr2) ? hr2 : hr;
}

/// <summary>
/// Checks if the current process is elevated.
/// </summary>
bool IsElevated() {
  // Allocate and initialize a SID of the administrators group.
  SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
  PSID pAdministratorsGroup = nullptr;
  if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
      0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
    return false;
  }

  // Determine whether the SID of administrators group is enabled in the primary access token of the process.
  BOOL fIsRunAsAdmin = FALSE;
  CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin);
  FreeSid(pAdministratorsGroup);

  return fIsRunAsAdmin != FALSE;
}

/// <summary>
/// Attempts to run the specified function in elevated mode.
/// We do that by calling rundll32 with runas.
/// </summary>
HRESULT RunElevated(LPCWSTR func) {
  WCHAR dllPath[MAX_PATH], commandLine[1024], workingDirectory[MAX_PATH];
  HRESULT hr = E_UNEXPECTED;
  HRESULT *pResult = (HRESULT *)malloc(sizeof(pResult));

  GetModuleFileNameW(::module, dllPath, MAX_PATH);
  StringCchPrintfW(commandLine, 1024, L"%s,DllElevatedEntry %s %p %x", dllPath, func, pResult, GetCurrentProcessId());
  GetCurrentDirectoryW(MAX_PATH, workingDirectory);

  SHELLEXECUTEINFOW info;
  info.cbSize = sizeof(SHELLEXECUTEINFOW);
  info.fMask = SEE_MASK_NOCLOSEPROCESS;
  info.lpDirectory = workingDirectory;
  info.lpFile = L"rundll32";
  info.lpParameters = commandLine;
  info.lpVerb = L"runas";
  info.nShow = SW_SHOW;

  ShellExecuteExW(&info);

  if (info.hProcess != NULL) {
    WaitForSingleObject(info.hProcess, INFINITE);
    CloseHandle(info.hProcess);

    if (pResult != nullptr) {
      hr = *pResult;
    }
  }

  free(pResult);

  return hr;
}

/// <summary>
/// Called by rundll32 when we are "elevating" the process.
/// </summary>
void CALLBACK DllElevatedEntry(HWND, HINSTANCE, LPSTR cmdLine, int /* cmdShow */) {
  char *context, *token, *endPtr;
  char func[64];

  // Get the function to execute
  token = strtok_s(cmdLine, " ", &context);
  StringCchCopyA(func, 64, token);

  // Get the address to store the result in
  token = strtok_s(nullptr, " ", &context);
  HRESULT *pResult = (HRESULT *)strtoull(token, &endPtr, 16);

  // Get the process ID
  token = strtok_s(nullptr, " ", &context);
  DWORD processID = (DWORD)strtoul(token, &endPtr, 16);

  // Execute the function
  HRESULT hr;
  if (strcmp(func, "DllRegisterServer") == 0) {
    hr = DllRegisterServer();
  } else if (strcmp(func, "DllUnregisterServer") == 0) {
    hr = DllUnregisterServer();
  } else {
    hr = E_UNEXPECTED;
  }

  // Store the result in the callers memory
  HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, false, processID);
  if (hProcess != NULL) {
    WriteProcessMemory(hProcess, pResult, &hr, sizeof(HRESULT), nullptr);
    CloseHandle(hProcess);
  }
}

/// <summary>
/// Retrives the path to the registry keys used.
/// </summary>
HRESULT GetKeyPath(LPWSTR path, UINT cchPath) {
  LPWSTR clsidString;
  HRESULT hr = StringFromCLSID(CLSID_ActivityBand, &clsidString);

  if (SUCCEEDED(hr)) {
    hr = StringCchPrintfW(path, cchPath, L"CLSID\\%s", clsidString);
    CoTaskMemFree(clsidString);
  }

  return hr;
}

/// <summary>
/// Deletes all registration-related registry keys.
/// </summary>
HRESULT DeleteRegKeys() {
  // Drop the CLSID key
  WCHAR key[MAX_PATH];
  HRESULT hr = GetKeyPath(key, MAX_PATH);
  if (SUCCEEDED(hr)) {
    LSTATUS ls = RegDeleteTreeW(HKEY_CLASSES_ROOT, key);
    hr = ls == ERROR_SUCCESS || ls == ERROR_FILE_NOT_FOUND ? S_OK : HRESULT_FROM_WIN32(ls);
  }

  return hr;
}

/// <summary>
/// Registers this DLL.
/// </summary>
HRESULT RegisterDLL() {
  HKEY baseKey, subKey;
  WCHAR dllPath[MAX_PATH], key[MAX_PATH];

  // Get the path to this DLL.
  GetModuleFileName(::module, dllPath, MAX_PATH);

  // Configure the top-level key.
  GetKeyPath(key, MAX_PATH);
  HRESULT hr = HRESULT_FROM_WIN32(RegCreateKey(HKEY_CLASSES_ROOT, key, &baseKey));
  if (SUCCEEDED(hr)) {
    RegSetValue(baseKey, nullptr, REG_SZ, L"ActivityBand", 0);
    RegSetValue(baseKey, L"DefaultIcon", REG_SZ, L"%SystemRoot%\\system32\\imageres.dll,-1023", 0);

    // Configure the InprocServer32 subkey.
    hr = HRESULT_FROM_WIN32(RegCreateKey(baseKey, L"InprocServer32", &subKey));
    if (SUCCEEDED(hr)) {
      RegSetValue(subKey, nullptr, REG_SZ, dllPath, 0);
      RegSetValue(subKey, L"ThreadingModel", REG_SZ, L"Apartment", 0);
      RegCloseKey(subKey);
    }

    // Close the top-level key.
    RegCloseKey(baseKey);
  }

  return S_OK;
}

HRESULT RegisterComCat() {
  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (SUCCEEDED(hr)) {
    ICatRegister *pcr;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcr));
    if (SUCCEEDED(hr)) {
      CATID catid = CATID_DeskBand;
      hr = pcr->RegisterClassImplCategories(CLSID_ActivityBand, 1, &catid);
      pcr->Release();
    }
    CoUninitialize();
  }
  return hr;
}

HRESULT UnRegisterComCat() {
  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (SUCCEEDED(hr)) {
    ICatRegister *pcr;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcr));
    if (SUCCEEDED(hr)) {
      CATID catid = CATID_DeskBand;
      hr = pcr->UnRegisterClassImplCategories(CLSID_ActivityBand, 1, &catid);
      pcr->Release();
    }
    CoUninitialize();
  }
  return hr;
}