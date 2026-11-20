#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wtsapi32.h>
#include "W2kCompatCommon.h"

// ============================================================================
// Helpers
// ============================================================================

typedef BOOL (WINAPI *PFN_WTSQuerySessionInformationW)(HANDLE, DWORD, WTS_INFO_CLASS, LPWSTR*, DWORD*);
typedef void (WINAPI *PFN_WTSFreeMemory)(PVOID);

static CompatProcSlot g_slotQuerySession;
static CompatProcSlot g_slotFreeMemory;

static PFN_WTSQuerySessionInformationW GetNativeQuerySession() {
    return CompatGetProc<PFN_WTSQuerySessionInformationW>(
        g_slotQuerySession, L"wtsapi32.dll", "WTSQuerySessionInformationW");
}

static PFN_WTSFreeMemory GetNativeFreeMemory() {
    return CompatGetProc<PFN_WTSFreeMemory>(
        g_slotFreeMemory, L"wtsapi32.dll", "WTSFreeMemory");
}

static BOOL ReturnBuffer(const void* data, DWORD bytes, LPWSTR* ppBuffer, DWORD* pBytesReturned) {
    void* buf = LocalAlloc(LPTR, bytes);
    if (!buf) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    RtlCopyMemory(buf, data, bytes);
    *ppBuffer = static_cast<LPWSTR>(buf);
    *pBytesReturned = bytes;
    return TRUE;
}

static BOOL ReturnString(LPCWSTR s, LPWSTR* ppBuffer, DWORD* pBytesReturned) {
    DWORD bytes = (static_cast<DWORD>(lstrlenW(s)) + 1) * sizeof(WCHAR);
    return ReturnBuffer(s, bytes, ppBuffer, pBytesReturned);
}

// ============================================================================
// Exports
// ============================================================================

typedef BOOL (WINAPI *PFN_WTSQueryUserToken)(ULONG, PHANDLE);

extern "C" BOOL WINAPI LibWTSQueryUserToken(ULONG SessionId, PHANDLE phToken) {
    COMPAT_PROC(pfn, PFN_WTSQueryUserToken, L"wtsapi32.dll", "WTSQueryUserToken");
    if (pfn) return pfn(SessionId, phToken);

    if (!phToken) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    HANDLE processToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &processToken)) {
        return FALSE;
    }

    BOOL ok = DuplicateTokenEx(processToken, MAXIMUM_ALLOWED, NULL,
                               SecurityImpersonation, TokenPrimary, phToken);
    DWORD err = ok ? ERROR_SUCCESS : GetLastError();
    CloseHandle(processToken);
    if (!ok) SetLastError(err);
    return ok;
}

typedef BOOL (WINAPI *PFN_WTSRegisterSessionNotification)(HWND, DWORD);
extern "C" BOOL WINAPI LibWTSRegisterSessionNotification(HWND hWnd, DWORD dwFlags) {
    COMPAT_PROC(pfn, PFN_WTSRegisterSessionNotification, L"wtsapi32.dll", "WTSRegisterSessionNotification");
    if (pfn) return pfn(hWnd, dwFlags);
    return TRUE;
}

typedef BOOL (WINAPI *PFN_WTSUnRegisterSessionNotification)(HWND);
extern "C" BOOL WINAPI LibWTSUnRegisterSessionNotification(HWND hWnd) {
    COMPAT_PROC(pfn, PFN_WTSUnRegisterSessionNotification, L"wtsapi32.dll", "WTSUnRegisterSessionNotification");
    if (pfn) return pfn(hWnd);
    return TRUE;
}

typedef BOOL (WINAPI *PFN_ProcessIdToSessionId)(DWORD, DWORD*);

static DWORD GetCurrentSessionIdBestEffort() {
    COMPAT_PROC(pfn, PFN_ProcessIdToSessionId, L"kernel32.dll", "ProcessIdToSessionId");
    DWORD session = 0;
    if (pfn && pfn(GetCurrentProcessId(), &session)) return session;
    return 0;
}

extern "C" BOOL WINAPI LibWTSQuerySessionInformationW(
    HANDLE hServer,
    DWORD SessionId,
    WTS_INFO_CLASS WTSInfoClass,
    LPWSTR* ppBuffer,
    DWORD* pBytesReturned
) {
    if (PFN_WTSQuerySessionInformationW native = GetNativeQuerySession()) {
        return native(hServer, SessionId, WTSInfoClass, ppBuffer, pBytesReturned);
    }

    if (!ppBuffer || !pBytesReturned) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    *ppBuffer = NULL;
    *pBytesReturned = 0;

    if (hServer != WTS_CURRENT_SERVER_HANDLE) {
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    switch (WTSInfoClass) {
        case WTSUserName: {
            WCHAR name[257];
            DWORD chars = sizeof(name) / sizeof(name[0]);
            if (!GetUserNameW(name, &chars)) return FALSE;
            return ReturnString(name, ppBuffer, pBytesReturned);
        }

        case WTSWinStationName:
            return ReturnString(L"Console", ppBuffer, pBytesReturned);

        case WTSDomainName: {
            WCHAR domain[256] = {};
            const DWORD capacity = sizeof(domain) / sizeof(domain[0]);

            DWORD len = GetEnvironmentVariableW(L"USERDOMAIN", domain, capacity);
            if (len == 0 || len >= capacity) {
                DWORD chars = capacity;
                if (!GetComputerNameW(domain, &chars)) domain[0] = L'\0';
            }
            return ReturnString(domain, ppBuffer, pBytesReturned);
        }

        case WTSConnectState: {
            WTS_CONNECTSTATE_CLASS state = WTSActive;
            return ReturnBuffer(&state, sizeof(state), ppBuffer, pBytesReturned);
        }

        case WTSSessionId: {
            DWORD session = GetCurrentSessionIdBestEffort();
            return ReturnBuffer(&session, sizeof(session), ppBuffer, pBytesReturned);
        }

        default:
            SetLastError(ERROR_NOT_SUPPORTED);
            return FALSE;
    }
}

extern "C" void WINAPI LibWTSFreeMemory(PVOID pMemory) {
    if (!pMemory) return;

    if (GetNativeQuerySession()) {
        if (PFN_WTSFreeMemory native = GetNativeFreeMemory()) native(pMemory);
        return;
    }

    LocalFree(pMemory);
}
