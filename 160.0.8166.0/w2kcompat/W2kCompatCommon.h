#pragma once

#include <windows.h>

#if defined(__clang__)
#  if __has_warning("-Wcast-function-type-mismatch")
#    pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#  endif
#endif

#ifndef COMPAT_ALLOW_APPLOCAL_FALLBACK
#define COMPAT_ALLOW_APPLOCAL_FALLBACK 1
#endif

// ============================================================================
// Lazy export resolution
// ============================================================================

struct CompatProcSlot {
    PVOID volatile value;
};

inline PVOID CompatProcMissing() {
    return reinterpret_cast<PVOID>(~static_cast<ULONG_PTR>(0));
}

inline HMODULE CompatLoadModule(PCWSTR name) {
    HMODULE mod = GetModuleHandleW(name);
    if (mod) return mod;

    WCHAR path[MAX_PATH];
    UINT dirLen = GetSystemDirectoryW(path, MAX_PATH);
    int nameLen = lstrlenW(name);
    if (dirLen != 0 && dirLen + 1 + static_cast<UINT>(nameLen) < MAX_PATH) {
        path[dirLen] = L'\\';
        lstrcpyW(path + dirLen + 1, name);
        mod = LoadLibraryW(path);
        if (mod) return mod;
    }

#if COMPAT_ALLOW_APPLOCAL_FALLBACK
    mod = LoadLibraryW(name);
#endif
    return mod;
}

inline PVOID CompatResolveProc(CompatProcSlot* slot, PCWSTR dll, PCSTR name) {
    PVOID cached = InterlockedCompareExchangePointer(&slot->value, NULL, NULL);
    if (cached) {
        return (cached == CompatProcMissing()) ? NULL : cached;
    }

    PVOID proc = NULL;
    HMODULE mod = CompatLoadModule(dll);
    if (mod) {
        proc = reinterpret_cast<PVOID>(GetProcAddress(mod, name));
    }

    InterlockedCompareExchangePointer(&slot->value,
                                      proc ? proc : CompatProcMissing(),
                                      NULL);
    return proc;
}

template <typename Fn>
inline Fn CompatGetProc(CompatProcSlot& slot, PCWSTR dll, PCSTR name) {
    return reinterpret_cast<Fn>(CompatResolveProc(&slot, dll, name));
}

#define COMPAT_PROC(var, Fn, dll, name)              \
    static CompatProcSlot var##_slot;                \
    Fn var = CompatGetProc<Fn>(var##_slot, dll, name)

// ============================================================================
// OS version check
// ============================================================================

inline bool IsWhistlerBuild2542OrOlder() {
    static volatile LONG s_state = -1;

    LONG state = s_state;
    if (state != -1) return state == 1;

    OSVERSIONINFOW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    BOOL ok = GetVersionExW(&osvi);

    LONG result = 0;
    if (ok) {
        if (osvi.dwMajorVersion < 5 ||
            (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)) {
            result = 1;
        } else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 &&
                   osvi.dwBuildNumber <= 2542) {
            result = 1;
        }
    }

    InterlockedExchange(&s_state, result);
    return result == 1;
}

inline bool IsXPNoSPOrOlder() {
    static volatile LONG s_state = -1;

    LONG state = s_state;
    if (state != -1) return state == 1;

    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    BOOL ok = GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&osvi));

    LONG result = 0;
    if (ok) {
        if (osvi.dwMajorVersion < 5 ||
            (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)) {
            result = 1;
        } else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
            if (osvi.dwBuildNumber <= 2542) {
                result = 1;
            } else if (osvi.dwBuildNumber == 2600 && osvi.wServicePackMajor == 0) {
                result = 1;
            }
        }
    }

    InterlockedExchange(&s_state, result);
    return result == 1;
}