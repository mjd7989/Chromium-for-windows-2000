#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>
#include <intrin.h>
#include "W2kCompatCommon.h"

// ============================================================================
// Helpers
// ============================================================================

typedef NTSTATUS (NTAPI *PFN_NtQueryInformationProcess)(HANDLE, ULONG, PVOID, ULONG, PULONG);
typedef ULONG    (NTAPI *PFN_RtlNtStatusToDosError)(NTSTATUS);

static CompatProcSlot g_slotNtQueryInformationProcess;
static CompatProcSlot g_slotRtlNtStatusToDosError;

static const ULONG kProcessBasicInformation = 0;
static const ULONG kProcessDebugPort        = 7;
static const ULONG kProcessHandleCount      = 20;

static void SetLastErrorFromNtStatus(NTSTATUS status) {
    PFN_RtlNtStatusToDosError convert = CompatGetProc<PFN_RtlNtStatusToDosError>(
        g_slotRtlNtStatusToDosError, L"ntdll.dll", "RtlNtStatusToDosError");
    SetLastError(convert ? convert(status) : ERROR_GEN_FAILURE);
}

static BOOL QueryProcessInformation(HANDLE process, ULONG infoClass, PVOID buffer, ULONG length) {
    PFN_NtQueryInformationProcess query = CompatGetProc<PFN_NtQueryInformationProcess>(
        g_slotNtQueryInformationProcess, L"ntdll.dll", "NtQueryInformationProcess");
    if (!query) {
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }
    NTSTATUS status = query(process, infoClass, buffer, length, NULL);
    if (status >= 0) return TRUE;
    SetLastErrorFromNtStatus(status);
    return FALSE;
}

// ============================================================================
// Exports
// ============================================================================

typedef BOOL (WINAPI *PFN_CheckRemoteDebuggerPresent)(HANDLE, PBOOL);

extern "C" BOOL WINAPI LibCheckRemoteDebuggerPresent(HANDLE hProcess, PBOOL pbDebuggerPresent) {
    COMPAT_PROC(pfn, PFN_CheckRemoteDebuggerPresent, L"kernel32.dll", "CheckRemoteDebuggerPresent");
    if (pfn) return pfn(hProcess, pbDebuggerPresent);

    if (!pbDebuggerPresent) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    DWORD_PTR debugPort = 0;
    if (!QueryProcessInformation(hProcess, kProcessDebugPort, &debugPort, sizeof(debugPort))) {
        return FALSE;
    }
    *pbDebuggerPresent = (debugPort != 0) ? TRUE : FALSE;
    return TRUE;
}

typedef BOOL (WINAPI *PFN_GetProcessHandleCount)(HANDLE, PDWORD);

extern "C" BOOL WINAPI LibGetProcessHandleCount(HANDLE hProcess, PDWORD pdwHandleCount) {
    COMPAT_PROC(pfn, PFN_GetProcessHandleCount, L"kernel32.dll", "GetProcessHandleCount");
    if (pfn) return pfn(hProcess, pdwHandleCount);

    if (!pdwHandleCount) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ULONG count = 0;
    if (!QueryProcessInformation(hProcess, kProcessHandleCount, &count, sizeof(count))) {
        return FALSE;
    }
    *pdwHandleCount = count;
    return TRUE;
}

typedef DWORD (WINAPI *PFN_GetProcessId)(HANDLE);

extern "C" DWORD WINAPI LibGetProcessId(HANDLE Process) {
    COMPAT_PROC(pfn, PFN_GetProcessId, L"kernel32.dll", "GetProcessId");
    if (pfn) return pfn(Process);

    PROCESS_BASIC_INFORMATION pbi = {};
    if (!QueryProcessInformation(Process, kProcessBasicInformation, &pbi, sizeof(pbi))) {
        return 0;
    }
    return static_cast<DWORD>(pbi.UniqueProcessId);
}

typedef PVOID (WINAPI *PFN_CodePointer)(PVOID);

extern "C" PVOID WINAPI LibDecodePointer(PVOID Ptr) {
    COMPAT_PROC(pfn, PFN_CodePointer, L"kernel32.dll", "DecodePointer");
    return pfn ? pfn(Ptr) : Ptr;
}

extern "C" PVOID WINAPI LibEncodePointer(PVOID Ptr) {
    COMPAT_PROC(pfn, PFN_CodePointer, L"kernel32.dll", "EncodePointer");
    return pfn ? pfn(Ptr) : Ptr;
}

struct CompatLogicalProcessorInfo {
    ULONG_PTR ProcessorMask;
    DWORD     Relationship;
    union {
        struct { BYTE Flags; } ProcessorCore;
        ULONGLONG Reserved[2];
    };
};
static_assert(sizeof(CompatLogicalProcessorInfo) == (sizeof(void*) == 4 ? 24 : 32),
              "must match SYSTEM_LOGICAL_PROCESSOR_INFORMATION");

typedef BOOL (WINAPI *PFN_GetLogicalProcessorInformation)(PVOID, PDWORD);

extern "C" BOOL WINAPI LibGetLogicalProcessorInformation(PVOID Buffer, PDWORD ReturnedLength) {
    COMPAT_PROC(pfn, PFN_GetLogicalProcessorInformation, L"kernel32.dll", "GetLogicalProcessorInformation");
    if (pfn) return pfn(Buffer, ReturnedLength);

    if (!ReturnedLength) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    ULONG_PTR activeMask = si.dwActiveProcessorMask ? si.dwActiveProcessorMask : 1;

    DWORD count = 0;
    for (ULONG_PTR m = activeMask; m; m &= m - 1) ++count;

    const DWORD required = count * sizeof(CompatLogicalProcessorInfo);
    if (!Buffer || *ReturnedLength < required) {
        *ReturnedLength = required;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    CompatLogicalProcessorInfo* entry = static_cast<CompatLogicalProcessorInfo*>(Buffer);
    for (unsigned bit = 0; bit < sizeof(ULONG_PTR) * 8; ++bit) {
        const ULONG_PTR processorBit = static_cast<ULONG_PTR>(1) << bit;
        if (!(activeMask & processorBit)) continue;

        RtlZeroMemory(entry, sizeof(*entry));
        entry->ProcessorMask = processorBit;
        entry->Relationship  = 0;
        ++entry;
    }

    *ReturnedLength = required;
    return TRUE;
}

typedef void (WINAPI *PFN_GetNativeSystemInfo)(LPSYSTEM_INFO);

extern "C" void WINAPI LibGetNativeSystemInfo(LPSYSTEM_INFO lpSystemInfo) {
    COMPAT_PROC(pfn, PFN_GetNativeSystemInfo, L"kernel32.dll", "GetNativeSystemInfo");
    if (pfn) {
        pfn(lpSystemInfo);
        return;
    }
    GetSystemInfo(lpSystemInfo);
}

typedef BOOL (WINAPI *PFN_IsWow64Process)(HANDLE, PBOOL);

extern "C" BOOL WINAPI LibIsWow64Process(HANDLE hProcess, PBOOL Wow64Process) {
    COMPAT_PROC(pfn, PFN_IsWow64Process, L"kernel32.dll", "IsWow64Process");
    if (pfn) return pfn(hProcess, Wow64Process);

    if (!Wow64Process) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    *Wow64Process = FALSE;
    return TRUE;
}

typedef BOOL (WINAPI *PFN_IsProcessInJob)(HANDLE, HANDLE, PBOOL);

extern "C" BOOL WINAPI LibIsProcessInJob(HANDLE ProcessHandle, HANDLE JobHandle, PBOOL Result) {
    COMPAT_PROC(pfn, PFN_IsProcessInJob, L"kernel32.dll", "IsProcessInJob");
    if (pfn) return pfn(ProcessHandle, JobHandle, Result);

    if (!Result) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    *Result = FALSE;
    return TRUE;
}

typedef DWORD (WINAPI *PFN_WTSGetActiveConsoleSessionId)(void);

extern "C" DWORD WINAPI LibWTSGetActiveConsoleSessionId(void) {
    COMPAT_PROC(pfn, PFN_WTSGetActiveConsoleSessionId, L"kernel32.dll", "WTSGetActiveConsoleSessionId");
    return pfn ? pfn() : 0;
}

typedef BOOL (WINAPI *PFN_AttachConsole)(DWORD);

extern "C" BOOL WINAPI LibAttachConsole(DWORD dwProcessId) {
    COMPAT_PROC(pfn, PFN_AttachConsole, L"kernel32.dll", "AttachConsole");
    if (pfn) return pfn(dwProcessId);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

typedef BOOL   (WINAPI *PFN_ConvertFiberToThread)(void);
typedef LPVOID (WINAPI *PFN_CreateFiberEx)(SIZE_T, SIZE_T, DWORD, LPVOID, LPVOID);

extern "C" BOOL WINAPI LibConvertFiberToThread(void) {
    COMPAT_PROC(pfn, PFN_ConvertFiberToThread, L"kernel32.dll", "ConvertFiberToThread");
    return pfn ? pfn() : TRUE;
}

extern "C" LPVOID WINAPI LibCreateFiberEx(SIZE_T dwStackCommitSize, SIZE_T dwStackReserveSize,
                                          DWORD dwFlags, LPVOID lpStartAddress, LPVOID lpParameter) {
    COMPAT_PROC(pfn, PFN_CreateFiberEx, L"kernel32.dll", "CreateFiberEx");
    if (pfn) return pfn(dwStackCommitSize, dwStackReserveSize, dwFlags, lpStartAddress, lpParameter);
    return CreateFiber(dwStackCommitSize, reinterpret_cast<LPFIBER_START_ROUTINE>(lpStartAddress), lpParameter);
}

static BOOL GetModuleHandleExFallback(DWORD flags, const void* nameOrAddress, bool wide, HMODULE* phModule) {
    if (!phModule) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    *phModule = NULL;

    const DWORD kPinAndUnchanged =
        GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    if ((flags & kPinAndUnchanged) == kPinAndUnchanged) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    HMODULE module = NULL;
    if (flags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(nameOrAddress, &mbi, sizeof(mbi))) return FALSE;
        if (mbi.Type != MEM_IMAGE) {
            SetLastError(ERROR_MOD_NOT_FOUND);
            return FALSE;
        }
        module = static_cast<HMODULE>(mbi.AllocationBase);
    } else {
        module = wide ? GetModuleHandleW(static_cast<LPCWSTR>(nameOrAddress))
                      : GetModuleHandleA(static_cast<LPCSTR>(nameOrAddress));
        if (!module) return FALSE;
    }

    if (!(flags & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT) && module != GetModuleHandleW(NULL)) {
        WCHAR path[1024];
        DWORD length = GetModuleFileNameW(module, path, sizeof(path) / sizeof(path[0]));
        if (length == 0) return FALSE;
        if (length >= sizeof(path) / sizeof(path[0])) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }
        if (!LoadLibraryW(path)) return FALSE;
    }

    *phModule = module;
    return TRUE;
}

typedef BOOL (WINAPI *PFN_GetModuleHandleExA)(DWORD, LPCSTR, HMODULE*);
typedef BOOL (WINAPI *PFN_GetModuleHandleExW)(DWORD, LPCWSTR, HMODULE*);

extern "C" BOOL WINAPI LibGetModuleHandleExA(DWORD dwFlags, LPCSTR lpModuleName, HMODULE* phModule) {
    if (!IsWhistlerBuild2542OrOlder()) {
        COMPAT_PROC(pfn, PFN_GetModuleHandleExA, L"kernel32.dll", "GetModuleHandleExA");
        if (pfn) return pfn(dwFlags, lpModuleName, phModule);
    }

    return GetModuleHandleExFallback(dwFlags, lpModuleName, false, phModule);
}

extern "C" BOOL WINAPI LibGetModuleHandleExW(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE* phModule) {
    if (!IsWhistlerBuild2542OrOlder()) {
        COMPAT_PROC(pfn, PFN_GetModuleHandleExW, L"kernel32.dll", "GetModuleHandleExW");
        if (pfn) return pfn(dwFlags, lpModuleName, phModule);
    }

    return GetModuleHandleExFallback(dwFlags, lpModuleName, true, phModule);
}

typedef void (WINAPI *PFN_RtlCaptureContext)(PVOID);
typedef WORD (WINAPI *PFN_RtlCaptureStackBackTrace)(DWORD, DWORD, PVOID*, PDWORD);

extern "C" void WINAPI LibRtlCaptureContext(PVOID ContextRecord) {
    COMPAT_PROC(pfnK32, PFN_RtlCaptureContext, L"kernel32.dll", "RtlCaptureContext");
    if (pfnK32) { pfnK32(ContextRecord); return; }

    COMPAT_PROC(pfnNt, PFN_RtlCaptureContext, L"ntdll.dll", "RtlCaptureContext");
    if (pfnNt) { pfnNt(ContextRecord); return; }

    if (ContextRecord) {
        RtlZeroMemory(ContextRecord, sizeof(CONTEXT));
    }
}

extern "C" WORD WINAPI LibRtlCaptureStackBackTrace(DWORD FramesToSkip, DWORD FramesToCapture,
                                                   PVOID* BackTrace, PDWORD BackTraceHash) {
    COMPAT_PROC(pfnK32, PFN_RtlCaptureStackBackTrace, L"kernel32.dll", "RtlCaptureStackBackTrace");
    if (pfnK32) return pfnK32(FramesToSkip, FramesToCapture, BackTrace, BackTraceHash);

    COMPAT_PROC(pfnNt, PFN_RtlCaptureStackBackTrace, L"ntdll.dll", "RtlCaptureStackBackTrace");
    if (pfnNt) return pfnNt(FramesToSkip, FramesToCapture, BackTrace, BackTraceHash);

    if (BackTraceHash) *BackTraceHash = 0;
    return 0;
}

typedef BOOL (WINAPI *PFN_HeapSetInformation)(HANDLE, DWORD, PVOID, SIZE_T);
typedef BOOL (WINAPI *PFN_HeapQueryInformation)(HANDLE, DWORD, PVOID, SIZE_T, PSIZE_T);

extern "C" BOOL WINAPI LibHeapSetInformation(HANDLE HeapHandle, DWORD HeapInformationClass,
                                             PVOID HeapInformation, SIZE_T HeapInformationLength) {
    COMPAT_PROC(pfn, PFN_HeapSetInformation, L"kernel32.dll", "HeapSetInformation");
    if (pfn) return pfn(HeapHandle, HeapInformationClass, HeapInformation, HeapInformationLength);
    return TRUE;
}

extern "C" BOOL WINAPI LibHeapQueryInformation(HANDLE HeapHandle, DWORD HeapInformationClass,
                                               PVOID HeapInformation, SIZE_T HeapInformationLength,
                                               PSIZE_T ReturnLength) {
    COMPAT_PROC(pfn, PFN_HeapQueryInformation, L"kernel32.dll", "HeapQueryInformation");
    if (pfn) return pfn(HeapHandle, HeapInformationClass, HeapInformation, HeapInformationLength, ReturnLength);

    if (HeapInformationClass != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (ReturnLength) *ReturnLength = sizeof(ULONG);
    if (!HeapInformation || HeapInformationLength < sizeof(ULONG)) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    *static_cast<ULONG*>(HeapInformation) = 0;
    return TRUE;
}

typedef int   (WINAPI *PFN_GetGeoInfoW)(GEOID, GEOTYPE, LPWSTR, int, LANGID);
typedef GEOID (WINAPI *PFN_GetUserGeoID)(GEOCLASS);

extern "C" int WINAPI LibGetGeoInfoW(GEOID Location, GEOTYPE GeoType, LPWSTR lpGeoData, int cchData, LANGID LangId) {
    COMPAT_PROC(pfn, PFN_GetGeoInfoW, L"kernel32.dll", "GetGeoInfoW");
    if (pfn) return pfn(Location, GeoType, lpGeoData, cchData, LangId);

    switch (GeoType) {
        case GEO_ISO2:
            return GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, lpGeoData, cchData);
        case GEO_ISO3:
            return GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SABBREVCTRYNAME, lpGeoData, cchData);
        default:
            SetLastError(ERROR_INVALID_FLAGS);
            return 0;
    }
}

extern "C" GEOID WINAPI LibGetUserGeoID(GEOCLASS GeoClass) {
    COMPAT_PROC(pfn, PFN_GetUserGeoID, L"kernel32.dll", "GetUserGeoID");
    return pfn ? pfn(GeoClass) : GEOID_NOT_AVAILABLE;
}

typedef BOOL (WINAPI *PFN_GetVolumePathNamesForVolumeNameW)(LPCWSTR, LPWSTR, DWORD, PDWORD);

extern "C" BOOL WINAPI LibGetVolumePathNamesForVolumeNameW(LPCWSTR lpszVolumeName, LPWSTR lpszVolumePathNames,
                                                           DWORD cchBufferLength, PDWORD lpcchReturnLength) {
    COMPAT_PROC(pfn, PFN_GetVolumePathNamesForVolumeNameW, L"kernel32.dll", "GetVolumePathNamesForVolumeNameW");
    if (pfn) return pfn(lpszVolumeName, lpszVolumePathNames, cchBufferLength, lpcchReturnLength);

    if (!lpszVolumeName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    WCHAR drives[26 * 4 + 1];
    const DWORD driveCapacity = sizeof(drives) / sizeof(drives[0]);
    DWORD driveChars = GetLogicalDriveStringsW(driveCapacity - 1, drives);
    if (driveChars == 0 || driveChars >= driveCapacity) return FALSE;

    const WCHAR* matches[26];
    DWORD matchCount = 0;
    DWORD totalChars = 1;

    for (const WCHAR* drive = drives; *drive && matchCount < 26; drive += lstrlenW(drive) + 1) {
        WCHAR volume[64];
        if (!GetVolumeNameForVolumeMountPointW(drive, volume, sizeof(volume) / sizeof(volume[0]))) continue;
        if (lstrcmpiW(volume, lpszVolumeName) != 0) continue;

        matches[matchCount++] = drive;
        totalChars += static_cast<DWORD>(lstrlenW(drive)) + 1;
    }

    if (lpcchReturnLength) *lpcchReturnLength = totalChars;
    if (!lpszVolumePathNames || cchBufferLength < totalChars) {
        SetLastError(ERROR_MORE_DATA);
        return FALSE;
    }

    WCHAR* out = lpszVolumePathNames;
    for (DWORD i = 0; i < matchCount; ++i) {
        size_t bytes = (static_cast<size_t>(lstrlenW(matches[i])) + 1) * sizeof(WCHAR);
        RtlCopyMemory(out, matches[i], bytes);
        out += bytes / sizeof(WCHAR);
    }
    *out = L'\0';
    return TRUE;
}

namespace {

const LONGLONG k100nsPerMinute = 600000000LL;

int DaysInMonth(int year, int month) {
    static const BYTE kDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    return (month == 2 && leap) ? 29 : kDays[month - 1];
}

bool SystemTimeToTicks(const SYSTEMTIME& st, ULONGLONG* out) {
    FILETIME ft;
    if (!SystemTimeToFileTime(&st, &ft)) return false;
    ULARGE_INTEGER u;
    u.LowPart  = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    *out = u.QuadPart;
    return true;
}

bool ResolveTransition(const SYSTEMTIME& rule, WORD year, SYSTEMTIME* out) {
    if (rule.wMonth < 1 || rule.wMonth > 12) return false;

    *out = rule;
    out->wYear = year;

    if (rule.wYear != 0) {
        return rule.wYear == year;
    }
    if (rule.wDay < 1 || rule.wDay > 5) return false;

    SYSTEMTIME first = {};
    first.wYear  = year;
    first.wMonth = rule.wMonth;
    first.wDay   = 1;
    FILETIME ft;
    if (!SystemTimeToFileTime(&first, &ft) || !FileTimeToSystemTime(&ft, &first)) return false;

    int day = 1 + (rule.wDayOfWeek - first.wDayOfWeek + 7) % 7 + (rule.wDay - 1) * 7;
    const int daysInMonth = DaysInMonth(year, rule.wMonth);
    while (day > daysInMonth) day -= 7;

    out->wDay = static_cast<WORD>(day);
    return true;
}

}  // namespace

typedef BOOL (WINAPI *PFN_TzSpecificLocalTimeToSystemTime)(LPTIME_ZONE_INFORMATION, LPSYSTEMTIME, LPSYSTEMTIME);

extern "C" BOOL WINAPI LibTzSpecificLocalTimeToSystemTime(LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
                                                          LPSYSTEMTIME lpLocalTime,
                                                          LPSYSTEMTIME lpUniversalTime) {
    COMPAT_PROC(pfn, PFN_TzSpecificLocalTimeToSystemTime, L"kernel32.dll", "TzSpecificLocalTimeToSystemTime");
    if (pfn) return pfn(lpTimeZoneInformation, lpLocalTime, lpUniversalTime);

    if (!lpLocalTime || !lpUniversalTime) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    TIME_ZONE_INFORMATION currentZone;
    const TIME_ZONE_INFORMATION* zone = lpTimeZoneInformation;
    if (!zone) {
        if (GetTimeZoneInformation(&currentZone) == TIME_ZONE_ID_INVALID) return FALSE;
        zone = &currentZone;
    }

    ULONGLONG local;
    if (!SystemTimeToTicks(*lpLocalTime, &local)) return FALSE;

    bool inDaylightTime = false;
    SYSTEMTIME startRule, endRule;
    ULONGLONG start, end;
    if (ResolveTransition(zone->DaylightDate, lpLocalTime->wYear, &startRule) &&
        ResolveTransition(zone->StandardDate, lpLocalTime->wYear, &endRule) &&
        SystemTimeToTicks(startRule, &start) && SystemTimeToTicks(endRule, &end)) {
        inDaylightTime = (start < end) ? (local >= start && local < end)
                                       : (local >= start || local < end);
    }

    const LONG totalBias = zone->Bias + (inDaylightTime ? zone->DaylightBias : zone->StandardBias);

    ULARGE_INTEGER utc;
    utc.QuadPart = local + static_cast<ULONGLONG>(static_cast<LONGLONG>(totalBias) * k100nsPerMinute);

    FILETIME ft;
    ft.dwLowDateTime  = utc.LowPart;
    ft.dwHighDateTime = utc.HighPart;
    return FileTimeToSystemTime(&ft, lpUniversalTime);
}

typedef void         (WINAPI *PFN_InitializeSListHead)(PSLIST_HEADER);
typedef PSLIST_ENTRY (WINAPI *PFN_InterlockedPushEntrySList)(PSLIST_HEADER, PSLIST_ENTRY);
typedef PSLIST_ENTRY (WINAPI *PFN_InterlockedFlushSList)(PSLIST_HEADER);

extern "C" void WINAPI LibInitializeSListHead(PSLIST_HEADER ListHead) {
    COMPAT_PROC(pfn, PFN_InitializeSListHead, L"kernel32.dll", "InitializeSListHead");
    if (pfn) {
        pfn(ListHead);
        return;
    }
    if (ListHead) RtlZeroMemory(ListHead, sizeof(SLIST_HEADER));
}

#if defined(_M_IX86)
static volatile LONGLONG* SListWord(PSLIST_HEADER head) {
    return reinterpret_cast<volatile LONGLONG*>(&head->Alignment);
}
#endif

extern "C" PSLIST_ENTRY WINAPI LibInterlockedPushEntrySList(PSLIST_HEADER ListHead, PSLIST_ENTRY SListEntry) {
    COMPAT_PROC(pfn, PFN_InterlockedPushEntrySList, L"kernel32.dll", "InterlockedPushEntrySList");
    if (pfn) return pfn(ListHead, SListEntry);

#if defined(_M_IX86)
    volatile LONGLONG* word = SListWord(ListHead);
    LONGLONG expected = _InterlockedCompareExchange64(word, 0, 0);
    for (;;) {
        const ULONGLONG old = static_cast<ULONGLONG>(expected);
        PSLIST_ENTRY oldFirst = reinterpret_cast<PSLIST_ENTRY>(static_cast<ULONG_PTR>(static_cast<DWORD>(old)));
        const WORD depth    = static_cast<WORD>(old >> 32);
        const WORD sequence = static_cast<WORD>(old >> 48);

        SListEntry->Next = oldFirst;
        const ULONGLONG desired =
            static_cast<ULONGLONG>(reinterpret_cast<ULONG_PTR>(SListEntry)) |
            (static_cast<ULONGLONG>(static_cast<WORD>(depth + 1)) << 32) |
            (static_cast<ULONGLONG>(static_cast<WORD>(sequence + 1)) << 48);

        const LONGLONG previous = _InterlockedCompareExchange64(word, static_cast<LONGLONG>(desired), expected);
        if (previous == expected) return oldFirst;
        expected = previous;
    }
#else
    PSLIST_ENTRY oldFirst;
    do {
        oldFirst = ListHead->Next.Next;
        SListEntry->Next = oldFirst;
    } while (InterlockedCompareExchangePointer(reinterpret_cast<PVOID volatile*>(&ListHead->Next.Next),
                                               SListEntry, oldFirst) != oldFirst);
    return oldFirst;
#endif
}

extern "C" PSLIST_ENTRY WINAPI LibInterlockedFlushSList(PSLIST_HEADER ListHead) {
    COMPAT_PROC(pfn, PFN_InterlockedFlushSList, L"kernel32.dll", "InterlockedFlushSList");
    if (pfn) return pfn(ListHead);

#if defined(_M_IX86)
    volatile LONGLONG* word = SListWord(ListHead);
    LONGLONG expected = _InterlockedCompareExchange64(word, 0, 0);
    for (;;) {
        const ULONGLONG old = static_cast<ULONGLONG>(expected);
        const ULONGLONG desired = (old >> 48) << 48;

        const LONGLONG previous = _InterlockedCompareExchange64(word, static_cast<LONGLONG>(desired), expected);
        if (previous == expected) {
            return reinterpret_cast<PSLIST_ENTRY>(static_cast<ULONG_PTR>(static_cast<DWORD>(old)));
        }
        expected = previous;
    }
#else
    return static_cast<PSLIST_ENTRY>(InterlockedExchangePointer(
        reinterpret_cast<PVOID volatile*>(&ListHead->Next.Next), NULL));
#endif
}

typedef PVOID (WINAPI *PFN_AddVectoredExceptionHandler)(ULONG, PVOID);
typedef ULONG (WINAPI *PFN_RemoveVectoredExceptionHandler)(PVOID);

extern "C" PVOID WINAPI LibAddVectoredExceptionHandler(ULONG First, PVOID Handler) {
    COMPAT_PROC(pfn, PFN_AddVectoredExceptionHandler, L"kernel32.dll", "AddVectoredExceptionHandler");
    if (pfn) return pfn(First, Handler);

    if (!Handler) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }
    return Handler;
}

extern "C" ULONG WINAPI LibRemoveVectoredExceptionHandler(PVOID Handle) {
    COMPAT_PROC(pfn, PFN_RemoveVectoredExceptionHandler, L"kernel32.dll", "RemoveVectoredExceptionHandler");
    if (pfn) return pfn(Handle);
    return Handle ? 1 : 0;
}