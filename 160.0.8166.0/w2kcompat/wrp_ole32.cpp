#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include "W2kCompatCommon.h"

// ============================================================================
// Exports
// ============================================================================

typedef HRESULT (WINAPI *PFN_CoGetContextToken)(ULONG_PTR* pToken);
typedef HRESULT (WINAPI *PFN_CoGetObjectContext)(REFIID riid, LPVOID* ppv);

extern "C" HRESULT WINAPI LibCoGetContextToken(ULONG_PTR* pToken) {
    COMPAT_PROC(pfn, PFN_CoGetContextToken, L"ole32.dll", "CoGetContextToken");
    if (pfn) return pfn(pToken);

    if (!pToken) return E_POINTER;
    *pToken = 0;

    COMPAT_PROC(pfnObjCtx, PFN_CoGetObjectContext, L"ole32.dll", "CoGetObjectContext");
    if (!pfnObjCtx) return CO_E_NOTINITIALIZED;

    IUnknown* context = nullptr;
    HRESULT hr = pfnObjCtx(IID_IUnknown, reinterpret_cast<void**>(&context));
    if (FAILED(hr)) return hr;
    if (!context) return CO_E_NOTINITIALIZED;

    *pToken = reinterpret_cast<ULONG_PTR>(context);
    context->Release();
    return S_OK;
}
