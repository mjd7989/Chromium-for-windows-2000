#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>
#include <psapi.h>
#include "W2kCompatCommon.h"

// ============================================================================
// Exports
// ============================================================================

typedef BOOL     (WINAPI *PFN_GetPerformanceInfo)(PPERFORMANCE_INFORMATION, DWORD);
typedef BOOL     (WINAPI *PFN_GlobalMemoryStatusEx)(LPMEMORYSTATUSEX);
typedef NTSTATUS (NTAPI  *PFN_NtQuerySystemInformation)(ULONG, PVOID, ULONG, PULONG);

static const ULONG    kSystemPerformanceInformation = 2;
static const ULONG    kSystemFileCacheInformation   = 21;
static const NTSTATUS kStatusInfoLengthMismatch     = (NTSTATUS)0xC0000004L;

struct SystemPerfPrefix {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER IoReadTransferCount;
    LARGE_INTEGER IoWriteTransferCount;
    LARGE_INTEGER IoOtherTransferCount;
    ULONG         IoReadOperationCount;
    ULONG         IoWriteOperationCount;
    ULONG         IoOtherOperationCount;
    ULONG         AvailablePages;
    ULONG         CommittedPages;
    ULONG         CommitLimit;
    ULONG         PeakCommitment;
};

static bool QuerySystemInfo(PFN_NtQuerySystemInformation query, ULONG infoClass,
                            void* buffer, ULONG capacity) {
    RtlZeroMemory(buffer, capacity);
    ULONG returned = 0;
    NTSTATUS status = query(infoClass, buffer, capacity, &returned);
    if (status == kStatusInfoLengthMismatch && returned != 0 && returned <= capacity) {
        RtlZeroMemory(buffer, capacity);
        status = query(infoClass, buffer, returned, &returned);
    }
    return status >= 0;
}

extern "C" BOOL WINAPI LibGetPerformanceInfo(PPERFORMANCE_INFORMATION info, DWORD cb) {
    COMPAT_PROC(pfnPsapi, PFN_GetPerformanceInfo, L"psapi.dll", "GetPerformanceInfo");
    if (pfnPsapi) return pfnPsapi(info, cb);

    COMPAT_PROC(pfnK32, PFN_GetPerformanceInfo, L"kernel32.dll", "K32GetPerformanceInfo");
    if (pfnK32) return pfnK32(info, cb);

    if (!info) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (cb < sizeof(PERFORMANCE_INFORMATION)) {
        SetLastError(ERROR_BAD_LENGTH);
        return FALSE;
    }

    RtlZeroMemory(info, sizeof(*info));
    info->cb = sizeof(*info);

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    const DWORD pageSize = si.dwPageSize ? si.dwPageSize : 4096;
    info->PageSize = pageSize;

    COMPAT_PROC(pfnMemEx, PFN_GlobalMemoryStatusEx, L"kernel32.dll", "GlobalMemoryStatusEx");
    if (pfnMemEx) {
        MEMORYSTATUSEX mex = {};
        mex.dwLength = sizeof(mex);
        if (pfnMemEx(&mex)) {
            info->PhysicalTotal     = static_cast<SIZE_T>(mex.ullTotalPhys / pageSize);
            info->PhysicalAvailable = static_cast<SIZE_T>(mex.ullAvailPhys / pageSize);
            info->CommitLimit       = static_cast<SIZE_T>(mex.ullTotalPageFile / pageSize);
            info->CommitTotal       = static_cast<SIZE_T>((mex.ullTotalPageFile - mex.ullAvailPageFile) / pageSize);
        }
    }
    info->CommitPeak = info->CommitTotal;

    COMPAT_PROC(pfnNtQuery, PFN_NtQuerySystemInformation, L"ntdll.dll", "NtQuerySystemInformation");
    if (pfnNtQuery) {
        LARGE_INTEGER perfBuf[64];
        if (QuerySystemInfo(pfnNtQuery, kSystemPerformanceInformation, perfBuf, sizeof(perfBuf))) {
            const SystemPerfPrefix* spi = reinterpret_cast<const SystemPerfPrefix*>(perfBuf);
            if (spi->AvailablePages) info->PhysicalAvailable = spi->AvailablePages;
            info->CommitTotal = spi->CommittedPages;
            if (spi->CommitLimit) info->CommitLimit = spi->CommitLimit;
            info->CommitPeak = spi->PeakCommitment;
        }

        LARGE_INTEGER cacheBuf[16];
        if (QuerySystemInfo(pfnNtQuery, kSystemFileCacheInformation, cacheBuf, sizeof(cacheBuf))) {
            info->SystemCache = *reinterpret_cast<const SIZE_T*>(cacheBuf) / pageSize;
        }
    }

    if (info->CommitPeak  < info->CommitTotal) info->CommitPeak  = info->CommitTotal;
    if (info->CommitLimit < info->CommitTotal) info->CommitLimit = info->CommitTotal;

    return TRUE;
}
