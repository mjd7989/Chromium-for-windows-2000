#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include "W2kCompatCommon.h"

// ============================================================================
// WinHTTP forwarders
// ============================================================================

#define WINHTTP_FORWARD(RetType, Name, Params, Args, FailValue)              \
    extern "C" RetType WINAPI Lib##Name Params {                             \
        typedef RetType (WINAPI *Fn) Params;                                 \
        COMPAT_PROC(pfn, Fn, L"winhttp.dll", #Name);                         \
        if (pfn) return pfn Args;                                            \
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);                            \
        return FailValue;                                                    \
    }

WINHTTP_FORWARD(BOOL, WinHttpAddRequestHeaders,
    (HINTERNET hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwModifiers),
    (hRequest, lpszHeaders, dwHeadersLength, dwModifiers),
    FALSE)

WINHTTP_FORWARD(BOOL, WinHttpCloseHandle,
    (HINTERNET hInternet),
    (hInternet),
    FALSE)

WINHTTP_FORWARD(HINTERNET, WinHttpConnect,
    (HINTERNET hSession, LPCWSTR pswzServerName, INTERNET_PORT nServerPort, DWORD dwReserved),
    (hSession, pswzServerName, nServerPort, dwReserved),
    NULL)

WINHTTP_FORWARD(BOOL, WinHttpCrackUrl,
    (LPCWSTR pwszUrl, DWORD dwUrlLength, DWORD dwFlags, LPURL_COMPONENTS lpUrlComponents),
    (pwszUrl, dwUrlLength, dwFlags, lpUrlComponents),
    FALSE)

WINHTTP_FORWARD(BOOL, WinHttpGetIEProxyConfigForCurrentUser,
    (WINHTTP_CURRENT_USER_IE_PROXY_CONFIG* pProxyConfig),
    (pProxyConfig),
    FALSE)

WINHTTP_FORWARD(BOOL, WinHttpGetProxyForUrl,
    (HINTERNET hSession, LPCWSTR lpcwszUrl, WINHTTP_AUTOPROXY_OPTIONS* pAutoProxyOptions, WINHTTP_PROXY_INFO* pProxyInfo),
    (hSession, lpcwszUrl, pAutoProxyOptions, pProxyInfo),
    FALSE)

WINHTTP_FORWARD(HINTERNET, WinHttpOpen,
    (LPCWSTR pszAgentW, DWORD dwAccessType, LPCWSTR pszProxyW, LPCWSTR pszProxyBypassW, DWORD dwFlags),
    (pszAgentW, dwAccessType, pszProxyW, pszProxyBypassW, dwFlags),
    NULL)

WINHTTP_FORWARD(HINTERNET, WinHttpOpenRequest,
    (HINTERNET hConnect, LPCWSTR pwszVerb, LPCWSTR pwszObjectName, LPCWSTR pwszVersion,
     LPCWSTR pwszReferrer, LPCWSTR* ppwszAcceptTypes, DWORD dwFlags),
    (hConnect, pwszVerb, pwszObjectName, pwszVersion, pwszReferrer, ppwszAcceptTypes, dwFlags),
    NULL)

WINHTTP_FORWARD(BOOL, WinHttpQueryDataAvailable,
    (HINTERNET hRequest, LPDWORD lpdwNumberOfBytesAvailable),
    (hRequest, lpdwNumberOfBytesAvailable),
    FALSE)

WINHTTP_FORWARD(BOOL, WinHttpQueryHeaders,
    (HINTERNET hRequest, DWORD dwInfoLevel, LPCWSTR pwszName, LPVOID lpBuffer,
     LPDWORD lpdwBufferLength, LPDWORD lpdwIndex),
    (hRequest, dwInfoLevel, pwszName, lpBuffer, lpdwBufferLength, lpdwIndex),
    FALSE)

WINHTTP_FORWARD(BOOL, WinHttpQueryOption,
    (HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, LPDWORD lpdwBufferLength),
    (hInternet, dwOption, lpBuffer, lpdwBufferLength),
    FALSE)

WINHTTP_FORWARD(BOOL, WinHttpReadData,
    (HINTERNET hRequest, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead),
    (hRequest, lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead),
    FALSE)

WINHTTP_FORWARD(BOOL, WinHttpReceiveResponse,
    (HINTERNET hRequest, LPVOID lpReserved),
    (hRequest, lpReserved),
    FALSE)

WINHTTP_FORWARD(BOOL, WinHttpSendRequest,
    (HINTERNET hRequest, LPCWSTR pwszHeaders, DWORD dwHeadersLength, LPVOID lpOptional,
     DWORD dwOptionalLength, DWORD dwTotalLength, DWORD_PTR dwContext),
    (hRequest, pwszHeaders, dwHeadersLength, lpOptional, dwOptionalLength, dwTotalLength, dwContext),
    FALSE)

WINHTTP_FORWARD(BOOL, WinHttpSetOption,
    (HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength),
    (hInternet, dwOption, lpBuffer, dwBufferLength),
    FALSE)

WINHTTP_FORWARD(WINHTTP_STATUS_CALLBACK, WinHttpSetStatusCallback,
    (HINTERNET hInternet, WINHTTP_STATUS_CALLBACK lpfnInternetCallback,
     DWORD dwNotificationFlags, DWORD_PTR dwReserved),
    (hInternet, lpfnInternetCallback, dwNotificationFlags, dwReserved),
    WINHTTP_INVALID_STATUS_CALLBACK)

WINHTTP_FORWARD(BOOL, WinHttpSetTimeouts,
    (HINTERNET hInternet, int nResolveTimeout, int nConnectTimeout, int nSendTimeout, int nReceiveTimeout),
    (hInternet, nResolveTimeout, nConnectTimeout, nSendTimeout, nReceiveTimeout),
    FALSE)

WINHTTP_FORWARD(BOOL, WinHttpWriteData,
    (HINTERNET hRequest, LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, LPDWORD lpdwNumberOfBytesWritten),
    (hRequest, lpBuffer, dwNumberOfBytesToWrite, lpdwNumberOfBytesWritten),
    FALSE)

#undef WINHTTP_FORWARD
