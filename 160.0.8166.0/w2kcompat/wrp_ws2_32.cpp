#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "W2kCompatCommon.h"

// ============================================================================
// Helpers
// ============================================================================

namespace {

const int kMaxFallbackAddresses = 32;

struct FallbackAddrInfoNode {
    addrinfo    ai;
    sockaddr_in sa;
};

bool ParsePort(const char* s, WORD* outPort) {
    if (!s || !s[0]) return false;
    unsigned long value = 0;
    for (; *s; ++s) {
        if (*s < '0' || *s > '9') return false;
        unsigned long digit = static_cast<unsigned long>(*s - '0');
        if (value > (65535UL - digit) / 10) return false;
        value = value * 10 + digit;
    }
    *outPort = static_cast<WORD>(value);
    return true;
}

void FreeFallbackAddrInfo(addrinfo* head) {
    HANDLE heap = GetProcessHeap();
    while (head) {
        addrinfo* next = head->ai_next;
        if (head->ai_canonname) HeapFree(heap, 0, head->ai_canonname);
        HeapFree(heap, 0, head);
        head = next;
    }
}

addrinfo* AllocFallbackNode(const in_addr& address, WORD portNetworkOrder, int socktype, int protocol) {
    FallbackAddrInfoNode* node = static_cast<FallbackAddrInfoNode*>(
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*node)));
    if (!node) return nullptr;

    node->sa.sin_family = AF_INET;
    node->sa.sin_port   = portNetworkOrder;
    node->sa.sin_addr   = address;

    node->ai.ai_family   = AF_INET;
    node->ai.ai_socktype = socktype;
    node->ai.ai_protocol = protocol;
    node->ai.ai_addrlen  = sizeof(node->sa);
    node->ai.ai_addr     = reinterpret_cast<sockaddr*>(&node->sa);
    return &node->ai;
}

bool SetCanonName(addrinfo* ai, const char* name) {
    size_t bytes = static_cast<size_t>(lstrlenA(name)) + 1;
    char* copy = static_cast<char*>(HeapAlloc(GetProcessHeap(), 0, bytes));
    if (!copy) return false;
    RtlCopyMemory(copy, name, bytes);
    ai->ai_canonname = copy;
    return true;
}

int ResolveService(PCSTR service, int socktype, int flags, WORD* portNetworkOrder) {
    *portNetworkOrder = 0;
    if (!service) return 0;

    WORD port;
    if (ParsePort(service, &port)) {
        *portNetworkOrder = htons(port);
        return 0;
    }
    if (flags & AI_NUMERICSERV) return EAI_NONAME;

    servent* entry = getservbyname(service, socktype == SOCK_DGRAM ? "udp" : "tcp");
    if (!entry) return EAI_SERVICE;
    *portNetworkOrder = static_cast<WORD>(entry->s_port);
    return 0;
}

}  // namespace

// ============================================================================
// Exports
// ============================================================================

typedef INT  (WSAAPI *PFN_getaddrinfo)(PCSTR, PCSTR, const addrinfo*, addrinfo**);

extern "C" INT WSAAPI Libgetaddrinfo(PCSTR pNodeName, PCSTR pServiceName,
                                     const addrinfo* pHints, addrinfo** ppResult) {
    COMPAT_PROC(pfnNative, PFN_getaddrinfo, L"ws2_32.dll", "getaddrinfo");
    if (pfnNative) return pfnNative(pNodeName, pServiceName, pHints, ppResult);

    if (!ppResult) return EAI_FAIL;
    *ppResult = nullptr;

    if (!pNodeName && !pServiceName) return EAI_NONAME;

    int flags = 0, family = AF_UNSPEC, socktype = 0, protocol = 0;
    if (pHints) {
        flags    = pHints->ai_flags;
        family   = pHints->ai_family;
        socktype = pHints->ai_socktype;
        protocol = pHints->ai_protocol;
    }
    if (family != AF_UNSPEC && family != AF_INET) return EAI_FAMILY;

    if (socktype == 0) socktype = (protocol == IPPROTO_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    if (protocol == 0) {
        protocol = (socktype == SOCK_STREAM) ? IPPROTO_TCP
                 : (socktype == SOCK_DGRAM)  ? IPPROTO_UDP : 0;
    }

    WORD port;
    int err = ResolveService(pServiceName, socktype, flags, &port);
    if (err) return err;

    in_addr addresses[kMaxFallbackAddresses];
    int count = 0;
    const char* canonName = nullptr;

    if (pNodeName) {
        if (!pNodeName[0]) return EAI_NONAME;

        unsigned long ip = inet_addr(pNodeName);
        if (ip != INADDR_NONE || lstrcmpA(pNodeName, "255.255.255.255") == 0) {
            addresses[count++].s_addr = ip;
            canonName = pNodeName;
        } else {
            if (flags & AI_NUMERICHOST) return EAI_NONAME;

            hostent* host = gethostbyname(pNodeName);
            if (!host || host->h_addrtype != AF_INET ||
                host->h_length != static_cast<int>(sizeof(in_addr)) ||
                !host->h_addr_list || !host->h_addr_list[0]) {
                return EAI_NONAME;
            }
            for (int i = 0; host->h_addr_list[i] && count < kMaxFallbackAddresses; ++i) {
                RtlCopyMemory(&addresses[count++], host->h_addr_list[i], sizeof(in_addr));
            }
            canonName = host->h_name;
        }
    } else {
        addresses[count++].s_addr = htonl((flags & AI_PASSIVE) ? INADDR_ANY : INADDR_LOOPBACK);
    }

    addrinfo* head = nullptr;
    addrinfo* tail = nullptr;
    for (int i = 0; i < count; ++i) {
        addrinfo* node = AllocFallbackNode(addresses[i], port, socktype, protocol);
        if (!node) {
            FreeFallbackAddrInfo(head);
            return EAI_MEMORY;
        }
        if (tail) tail->ai_next = node; else head = node;
        tail = node;
    }

    if ((flags & AI_CANONNAME) && canonName && !SetCanonName(head, canonName)) {
        FreeFallbackAddrInfo(head);
        return EAI_MEMORY;
    }

    *ppResult = head;
    return 0;
}

typedef void (WSAAPI *PFN_freeaddrinfo)(addrinfo*);
extern "C" void WSAAPI Libfreeaddrinfo(addrinfo* pAddrInfo) {
    COMPAT_PROC(pfnNative, PFN_freeaddrinfo, L"ws2_32.dll", "freeaddrinfo");
    if (pfnNative) {
        pfnNative(pAddrInfo);
        return;
    }
    FreeFallbackAddrInfo(pAddrInfo);
}

namespace {

const DWORD kFakeLookupSignature = 0x4B4C5357;

struct FakeLookupContext {
    DWORD signature;
    DWORD controlFlags;
    BOOL  completed;
};

FakeLookupContext* AsFakeContext(HANDLE handle) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return nullptr;
    FakeLookupContext* ctx = static_cast<FakeLookupContext*>(handle);
    return ctx->signature == kFakeLookupSignature ? ctx : nullptr;
}

}  // namespace

typedef INT (WSAAPI *PFN_WSALookupServiceBeginW)(LPWSAQUERYSETW, DWORD, LPHANDLE);
typedef INT (WSAAPI *PFN_WSALookupServiceNextW)(HANDLE, DWORD, LPDWORD, LPWSAQUERYSETW);
typedef INT (WSAAPI *PFN_WSALookupServiceEnd)(HANDLE);

extern "C" INT WSAAPI LibWSALookupServiceBeginW(LPWSAQUERYSETW lpqsRestrictions,
                                                DWORD dwControlFlags,
                                                LPHANDLE lphLookup) {
    if (!IsWhistlerBuild2542OrOlder()) {
        COMPAT_PROC(pfnBegin, PFN_WSALookupServiceBeginW, L"ws2_32.dll", "WSALookupServiceBeginW");
        if (pfnBegin) return pfnBegin(lpqsRestrictions, dwControlFlags, lphLookup);
        WSASetLastError(WSAEOPNOTSUPP);
        return SOCKET_ERROR;
    }

    if (!lphLookup) {
        WSASetLastError(WSA_INVALID_PARAMETER);
        return SOCKET_ERROR;
    }

    FakeLookupContext* ctx = static_cast<FakeLookupContext*>(
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*ctx)));
    if (!ctx) {
        WSASetLastError(WSA_NOT_ENOUGH_MEMORY);
        return SOCKET_ERROR;
    }

    ctx->signature    = kFakeLookupSignature;
    ctx->controlFlags = dwControlFlags;
    ctx->completed    = FALSE;

    *lphLookup = ctx;
    return 0;
}

extern "C" INT WSAAPI LibWSALookupServiceNextW(HANDLE hLookup,
                                               DWORD dwControlFlags,
                                               LPDWORD lpdwBufferLength,
                                               LPWSAQUERYSETW lpqsResults) {
    if (!IsWhistlerBuild2542OrOlder()) {
        COMPAT_PROC(pfnNext, PFN_WSALookupServiceNextW, L"ws2_32.dll", "WSALookupServiceNextW");
        if (pfnNext) return pfnNext(hLookup, dwControlFlags, lpdwBufferLength, lpqsResults);
        WSASetLastError(WSAEOPNOTSUPP);
        return SOCKET_ERROR;
    }

    FakeLookupContext* ctx = AsFakeContext(hLookup);
    if (!ctx) {
        WSASetLastError(WSA_INVALID_HANDLE);
        return SOCKET_ERROR;
    }

    if (ctx->completed) {
        WSASetLastError(WSA_E_NO_MORE);
        return SOCKET_ERROR;
    }

    if (lpqsResults && lpdwBufferLength && *lpdwBufferLength >= sizeof(WSAQUERYSETW)) {
        RtlZeroMemory(lpqsResults, sizeof(WSAQUERYSETW));
        lpqsResults->dwSize = sizeof(WSAQUERYSETW);
        ctx->completed = TRUE;
        return 0;
    }

    if (lpdwBufferLength) *lpdwBufferLength = sizeof(WSAQUERYSETW);
    WSASetLastError(WSAEFAULT);
    return SOCKET_ERROR;
}

extern "C" INT WSAAPI LibWSALookupServiceEnd(HANDLE hLookup) {
    if (!IsWhistlerBuild2542OrOlder()) {
        COMPAT_PROC(pfnEnd, PFN_WSALookupServiceEnd, L"ws2_32.dll", "WSALookupServiceEnd");
        if (pfnEnd) return pfnEnd(hLookup);
        WSASetLastError(WSAEOPNOTSUPP);
        return SOCKET_ERROR;
    }

    FakeLookupContext* ctx = AsFakeContext(hLookup);
    if (!ctx) {
        WSASetLastError(WSA_INVALID_HANDLE);
        return SOCKET_ERROR;
    }

    ctx->signature = 0;
    HeapFree(GetProcessHeap(), 0, ctx);
    return 0;
}
