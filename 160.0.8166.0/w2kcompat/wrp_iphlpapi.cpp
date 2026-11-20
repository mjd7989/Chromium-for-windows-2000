#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include "W2kCompatCommon.h"

// ============================================================================
// Helpers
// ============================================================================

namespace {

bool ParseIPv4(const char* s, ULONG* outNetworkOrder) {
    if (!s) return false;

    ULONG octet[4];
    for (int i = 0; i < 4; ++i) {
        if (*s < '0' || *s > '9') return false;
        ULONG value = 0;
        int digits = 0;
        while (*s >= '0' && *s <= '9') {
            value = value * 10 + static_cast<ULONG>(*s - '0');
            if (++digits > 3 || value > 255) return false;
            ++s;
        }
        octet[i] = value;
        if (i < 3) {
            if (*s != '.') return false;
            ++s;
        }
    }
    if (*s != '\0') return false;

    *outNetworkOrder = htonl((octet[0] << 24) | (octet[1] << 16) | (octet[2] << 8) | octet[3]);
    return true;
}

bool ParseUsableIPv4(const char* s, ULONG* outNetworkOrder) {
    return ParseIPv4(s, outNetworkOrder) && *outNetworkOrder != 0;
}

bool MaskToPrefix(const char* maskString, UINT8* outPrefix, ULONG* outMaskNetworkOrder) {
    ULONG maskNet;
    if (!ParseIPv4(maskString, &maskNet)) return false;

    ULONG mask = ntohl(maskNet);
    ULONG inverted = ~mask;
    if (inverted & (inverted + 1)) return false;

    UINT8 prefix = 0;
    while (mask & 0x80000000UL) {
        ++prefix;
        mask <<= 1;
    }
    *outPrefix = prefix;
    *outMaskNetworkOrder = maskNet;
    return true;
}

class HeapBuffer {
public:
    HeapBuffer() : data_(nullptr) {}
    ~HeapBuffer() { Free(); }
    HeapBuffer(const HeapBuffer&) = delete;
    HeapBuffer& operator=(const HeapBuffer&) = delete;

    bool Alloc(SIZE_T bytes) {
        Free();
        data_ = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytes);
        return data_ != nullptr;
    }
    void* get() const { return data_; }

private:
    void Free() {
        if (data_) HeapFree(GetProcessHeap(), 0, data_);
        data_ = nullptr;
    }
    void* data_;
};

ULONG LoadAdaptersInfo(HeapBuffer* buffer) {
    ULONG size = 0;
    ULONG err = GetAdaptersInfo(NULL, &size);
    for (int attempt = 0; attempt < 4 && err == ERROR_BUFFER_OVERFLOW; ++attempt) {
        if (!buffer->Alloc(size)) return ERROR_NOT_ENOUGH_MEMORY;
        err = GetAdaptersInfo(static_cast<PIP_ADAPTER_INFO>(buffer->get()), &size);
    }
    if (err == NO_ERROR && !buffer->get()) return ERROR_NO_DATA;
    return err;
}

ULONG LoadNetworkParams(HeapBuffer* buffer) {
    ULONG size = 0;
    ULONG err = GetNetworkParams(NULL, &size);
    for (int attempt = 0; attempt < 4 && err == ERROR_BUFFER_OVERFLOW; ++attempt) {
        if (!buffer->Alloc(size)) return ERROR_NOT_ENOUGH_MEMORY;
        err = GetNetworkParams(static_cast<PFIXED_INFO>(buffer->get()), &size);
    }
    if (err == NO_ERROR && !buffer->get()) return ERROR_NO_DATA;
    return err;
}

class Arena {
public:
    Arena(BYTE* base, ULONG capacity) : base_(base), capacity_(capacity), used_(0), failed_(false) {}

    void* Alloc(size_t size) {
        if (failed_) return nullptr;

        const ULONG kAlign = 8;
        if (used_ > ULONG_MAX - (kAlign - 1)) { failed_ = true; return nullptr; }
        ULONG start = (used_ + kAlign - 1) & ~(kAlign - 1);
        if (size > ULONG_MAX - start)          { failed_ = true; return nullptr; }

        used_ = start + static_cast<ULONG>(size);
        if (!base_) return nullptr;
        if (used_ > capacity_) { failed_ = true; return nullptr; }
        return base_ + start;
    }

    template <typename T>
    T* Alloc() { return static_cast<T*>(Alloc(sizeof(T))); }

    ULONG used() const   { return used_; }
    bool  failed() const { return failed_; }

private:
    BYTE* base_;
    ULONG capacity_;
    ULONG used_;
    bool  failed_;
};

PWSTR AllocWideFromAnsi(Arena& arena, const char* ansi) {
    int chars = MultiByteToWideChar(CP_ACP, 0, ansi, -1, NULL, 0);
    if (chars <= 0) chars = 1;
    PWSTR wide = static_cast<PWSTR>(arena.Alloc(static_cast<size_t>(chars) * sizeof(WCHAR)));
    if (wide && chars > 1) {
        MultiByteToWideChar(CP_ACP, 0, ansi, -1, wide, chars);
    }
    return wide;
}

template <typename Node>
Node* AppendAddressNode(Arena& arena, ULONG addressNetworkOrder, Node** head, Node** tail) {
    Node*        node = arena.Alloc<Node>();
    sockaddr_in* sa   = arena.Alloc<sockaddr_in>();
    if (!node || !sa) return nullptr;

    sa->sin_family      = AF_INET;
    sa->sin_addr.s_addr = addressNetworkOrder;

    node->Length                   = sizeof(Node);
    node->Address.lpSockaddr       = reinterpret_cast<LPSOCKADDR>(sa);
    node->Address.iSockaddrLength  = sizeof(sockaddr_in);

    if (*tail) (*tail)->Next = node; else *head = node;
    *tail = node;
    return node;
}

bool BuildAdapterList(Arena& arena,
                      const IP_ADAPTER_INFO* adapters,
                      const FIXED_INFO* fixedInfo,
                      ULONG flags,
                      PIP_ADAPTER_ADDRESSES* outHead) {
    PIP_ADAPTER_ADDRESSES head = nullptr, tail = nullptr;
    bool dnsAssigned = false;

    for (const IP_ADAPTER_INFO* cur = adapters; cur; cur = cur->Next) {
        PIP_ADAPTER_ADDRESSES adapter = arena.Alloc<IP_ADAPTER_ADDRESSES>();

        char* name = static_cast<char*>(arena.Alloc(static_cast<size_t>(lstrlenA(cur->AdapterName)) + 1));
        PWSTR friendlyName = (flags & GAA_FLAG_SKIP_FRIENDLY_NAME) ? nullptr
                                                                    : AllocWideFromAnsi(arena, cur->Description);
        PWSTR description = AllocWideFromAnsi(arena, cur->Description);
        PWSTR dnsSuffix   = static_cast<PWSTR>(arena.Alloc(sizeof(WCHAR)));

        PIP_ADAPTER_UNICAST_ADDRESS firstUnicast = nullptr, lastUnicast = nullptr;
        PIP_ADAPTER_PREFIX          firstPrefix  = nullptr, lastPrefix  = nullptr;
        bool hasAddress = false;

        for (const IP_ADDR_STRING* a = &cur->IpAddressList; a; a = a->Next) {
            ULONG ip;
            if (!ParseUsableIPv4(a->IpAddress.String, &ip)) continue;
            hasAddress = true;

            UINT8 prefixLength = 0;
            ULONG maskNet = 0;
            const bool validMask = MaskToPrefix(a->IpMask.String, &prefixLength, &maskNet);

            if (!(flags & GAA_FLAG_SKIP_UNICAST)) {
                PIP_ADAPTER_UNICAST_ADDRESS u =
                    AppendAddressNode<IP_ADAPTER_UNICAST_ADDRESS>(arena, ip, &firstUnicast, &lastUnicast);
                if (u) {
                    u->DadState           = IpDadStatePreferred;
                    u->ValidLifetime      = 0xFFFFFFFF;
                    u->PreferredLifetime  = 0xFFFFFFFF;
                    u->PrefixOrigin       = cur->DhcpEnabled ? IpPrefixOriginDhcp : IpPrefixOriginManual;
                    u->SuffixOrigin       = cur->DhcpEnabled ? IpSuffixOriginDhcp : IpSuffixOriginManual;
                    u->OnLinkPrefixLength = validMask ? prefixLength : 0;
                }
            }

            if ((flags & GAA_FLAG_INCLUDE_PREFIX) && validMask) {
                PIP_ADAPTER_PREFIX p =
                    AppendAddressNode<IP_ADAPTER_PREFIX>(arena, ip & maskNet, &firstPrefix, &lastPrefix);
                if (p) p->PrefixLength = prefixLength;
            }
        }

        PIP_ADAPTER_GATEWAY_ADDRESS firstGateway = nullptr, lastGateway = nullptr;
        if (flags & GAA_FLAG_INCLUDE_GATEWAYS) {
            for (const IP_ADDR_STRING* g = &cur->GatewayList; g; g = g->Next) {
                ULONG gw;
                if (ParseUsableIPv4(g->IpAddress.String, &gw)) {
                    AppendAddressNode<IP_ADAPTER_GATEWAY_ADDRESS>(arena, gw, &firstGateway, &lastGateway);
                }
            }
        }

        PIP_ADAPTER_DNS_SERVER_ADDRESS firstDns = nullptr, lastDns = nullptr;
        if (fixedInfo && !dnsAssigned && !(flags & GAA_FLAG_SKIP_DNS_SERVER)) {
            for (const IP_ADDR_STRING* d = &fixedInfo->DnsServerList; d; d = d->Next) {
                ULONG dns;
                if (ParseUsableIPv4(d->IpAddress.String, &dns)) {
                    AppendAddressNode<IP_ADAPTER_DNS_SERVER_ADDRESS>(arena, dns, &firstDns, &lastDns);
                }
            }
            dnsAssigned = true;
        }

        if (adapter) {
            if (name) RtlCopyMemory(name, cur->AdapterName, static_cast<size_t>(lstrlenA(cur->AdapterName)));

            adapter->Length       = sizeof(IP_ADAPTER_ADDRESSES);
            adapter->IfIndex      = cur->Index;
            adapter->Ipv6IfIndex  = 0;
            adapter->IfType       = cur->Type;
            adapter->OperStatus   = hasAddress ? IfOperStatusUp : IfOperStatusDown;
            adapter->Flags        = cur->DhcpEnabled ? IP_ADAPTER_DHCP_ENABLED : 0;

            adapter->AdapterName  = name;
            adapter->FriendlyName = friendlyName;
            adapter->Description  = description;
            adapter->DnsSuffix    = dnsSuffix;

            UINT addressLength = cur->AddressLength;
            if (addressLength > MAX_ADAPTER_ADDRESS_LENGTH) addressLength = MAX_ADAPTER_ADDRESS_LENGTH;
            adapter->PhysicalAddressLength = addressLength;
            RtlCopyMemory(adapter->PhysicalAddress, cur->Address, addressLength);

            adapter->FirstUnicastAddress   = firstUnicast;
            adapter->FirstPrefix           = firstPrefix;
            adapter->FirstGatewayAddress   = firstGateway;
            adapter->FirstDnsServerAddress = firstDns;

            if (tail) tail->Next = adapter; else head = adapter;
            tail = adapter;
        }
    }

    *outHead = head;
    return !arena.failed();
}

ULONG EmulateGetAdaptersAddresses(ULONG family, ULONG flags,
                                  PIP_ADAPTER_ADDRESSES buffer, PULONG sizePointer) {
    if (!sizePointer) return ERROR_INVALID_PARAMETER;

    if (family == AF_INET6) {
        *sizePointer = 0;
        return ERROR_NO_DATA;
    }
    if (family != AF_UNSPEC && family != AF_INET) return ERROR_INVALID_PARAMETER;

    HeapBuffer infoBuffer;
    ULONG err = LoadAdaptersInfo(&infoBuffer);
    if (err != NO_ERROR) {
        *sizePointer = 0;
        return err;
    }

    HeapBuffer fixedBuffer;
    if (LoadNetworkParams(&fixedBuffer) != NO_ERROR) {
        fixedBuffer.Alloc(0);
    }

    const IP_ADAPTER_INFO* adapters = static_cast<const IP_ADAPTER_INFO*>(infoBuffer.get());
    const FIXED_INFO*      fixed    = static_cast<const FIXED_INFO*>(fixedBuffer.get());
    PIP_ADAPTER_ADDRESSES  unused   = nullptr;

    Arena measure(nullptr, 0);
    BuildAdapterList(measure, adapters, fixed, flags, &unused);
    if (measure.failed()) return ERROR_ARITHMETIC_OVERFLOW;

    const ULONG needed = measure.used();
    if (!buffer || *sizePointer < needed) {
        *sizePointer = needed;
        return ERROR_BUFFER_OVERFLOW;
    }

    RtlZeroMemory(buffer, needed);
    Arena fill(reinterpret_cast<BYTE*>(buffer), needed);
    if (!BuildAdapterList(fill, adapters, fixed, flags, &unused)) {
        *sizePointer = needed;
        return ERROR_BUFFER_OVERFLOW;
    }

    *sizePointer = fill.used();
    return ERROR_SUCCESS;
}

}  // namespace

// ============================================================================
// Exports
// ============================================================================

typedef ULONG (WINAPI *PFN_GetAdaptersAddresses)(ULONG, ULONG, PVOID, PIP_ADAPTER_ADDRESSES, PULONG);

extern "C" ULONG WINAPI LibGetAdaptersAddresses(
    ULONG Family,
    ULONG Flags,
    PVOID Reserved,
    PIP_ADAPTER_ADDRESSES AdapterAddresses,
    PULONG SizePointer) {

    if (!IsXPNoSPOrOlder()) {
        COMPAT_PROC(pfn, PFN_GetAdaptersAddresses, L"iphlpapi.dll", "GetAdaptersAddresses");
        if (pfn) return pfn(Family, Flags, Reserved, AdapterAddresses, SizePointer);
    }

    return EmulateGetAdaptersAddresses(Family, Flags, AdapterAddresses, SizePointer);
}

typedef BOOL (WINAPI *PFN_CancelIPChangeNotify)(LPOVERLAPPED);

extern "C" DWORD WINAPI LibCancelIPChangeNotify(LPOVERLAPPED notifyOverlapped) {
    if (!IsXPNoSPOrOlder()) {
        COMPAT_PROC(pfn, PFN_CancelIPChangeNotify, L"iphlpapi.dll", "CancelIPChangeNotify");
        if (pfn) return static_cast<DWORD>(pfn(notifyOverlapped));
    }
    return TRUE;
}
