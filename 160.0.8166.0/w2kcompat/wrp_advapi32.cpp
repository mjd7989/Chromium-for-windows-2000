#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#include "W2kCompatCommon.h"

// ============================================================================
// Helpers
// ============================================================================

template <typename T>
static bool ParseDecimal(LPCWSTR s, LPCWSTR* end, T* out) {
    const T kMax = static_cast<T>(~static_cast<T>(0));
    if (*s < L'0' || *s > L'9') return false;

    T value = 0;
    while (*s >= L'0' && *s <= L'9') {
        T digit = static_cast<T>(*s - L'0');
        if (value > (kMax - digit) / 10) return false;
        value = static_cast<T>(value * 10 + digit);
        ++s;
    }
    *end = s;
    *out = value;
    return true;
}

static bool ParseHex48(LPCWSTR s, LPCWSTR* end, ULONG64* out) {
    ULONG64 value = 0;
    int digits = 0;
    for (;; ++s, ++digits) {
        WCHAR c = *s;
        ULONG d;
        if (c >= L'0' && c <= L'9')      d = c - L'0';
        else if (c >= L'a' && c <= L'f') d = c - L'a' + 10;
        else if (c >= L'A' && c <= L'F') d = c - L'A' + 10;
        else break;
        if (digits == 12) return false;
        value = (value << 4) | d;
    }
    if (digits == 0) return false;
    *end = s;
    *out = value;
    return true;
}

static WCHAR AsciiUpper(WCHAR c) {
    return (c >= L'a' && c <= L'z') ? static_cast<WCHAR>(c - (L'a' - L'A')) : c;
}

static bool EqualsNoCase(LPCWSTR a, LPCWSTR b) {
    for (;; ++a, ++b) {
        WCHAR ca = AsciiUpper(*a);
        if (ca != AsciiUpper(*b)) return false;
        if (ca == 0) return true;
    }
}

enum SidAuthorityIndex { kAuthNull, kAuthWorld, kAuthLocal, kAuthCreator, kAuthNt };

static const SID_IDENTIFIER_AUTHORITY kAuthorities[] = {
    SECURITY_NULL_SID_AUTHORITY,
    SECURITY_WORLD_SID_AUTHORITY,
    SECURITY_LOCAL_SID_AUTHORITY,
    SECURITY_CREATOR_SID_AUTHORITY,
    SECURITY_NT_AUTHORITY,
};

struct WellKnownSidDef {
    WELL_KNOWN_SID_TYPE type;
    BYTE                authority;
    BYTE                subAuthCount;
    DWORD               rid[2];
};

static const WellKnownSidDef kWellKnownSids[] = {
    { WinNullSid,                    kAuthNull,    1, { 0, 0 } },
    { WinWorldSid,                   kAuthWorld,   1, { 0, 0 } },
    { WinLocalSid,                   kAuthLocal,   1, { 0, 0 } },
    { WinCreatorOwnerSid,            kAuthCreator, 1, { 0, 0 } },
    { WinCreatorGroupSid,            kAuthCreator, 1, { 1, 0 } },

    { WinDialupSid,                  kAuthNt,      1, { 1, 0 } },
    { WinNetworkSid,                 kAuthNt,      1, { 2, 0 } },
    { WinBatchSid,                   kAuthNt,      1, { 3, 0 } },
    { WinInteractiveSid,             kAuthNt,      1, { 4, 0 } },
    { WinServiceSid,                 kAuthNt,      1, { 6, 0 } },
    { WinAnonymousSid,               kAuthNt,      1, { 7, 0 } },
    { WinSelfSid,                    kAuthNt,      1, { 10, 0 } },
    { WinAuthenticatedUserSid,       kAuthNt,      1, { 11, 0 } },
    { WinRestrictedCodeSid,          kAuthNt,      1, { 12, 0 } },
    { WinTerminalServerSid,          kAuthNt,      1, { 13, 0 } },
    { WinRemoteLogonIdSid,           kAuthNt,      1, { 14, 0 } },
    { WinLocalSystemSid,             kAuthNt,      1, { 18, 0 } },
    { WinLocalServiceSid,            kAuthNt,      1, { 19, 0 } },
    { WinNetworkServiceSid,          kAuthNt,      1, { 20, 0 } },
    { WinWriteRestrictedCodeSid,       kAuthNt,      1, { SECURITY_WRITE_RESTRICTED_CODE_RID, 0 } },

    { WinBuiltinAdministratorsSid,   kAuthNt,      2, { SECURITY_BUILTIN_DOMAIN_RID, 544 } },
    { WinBuiltinUsersSid,            kAuthNt,      2, { SECURITY_BUILTIN_DOMAIN_RID, 545 } },
    { WinBuiltinGuestsSid,           kAuthNt,      2, { SECURITY_BUILTIN_DOMAIN_RID, 546 } },
    { WinBuiltinPowerUsersSid,       kAuthNt,      2, { SECURITY_BUILTIN_DOMAIN_RID, 547 } },
    { WinBuiltinAccountOperatorsSid, kAuthNt,      2, { SECURITY_BUILTIN_DOMAIN_RID, 548 } },
    { WinBuiltinSystemOperatorsSid,  kAuthNt,      2, { SECURITY_BUILTIN_DOMAIN_RID, 549 } },
    { WinBuiltinPrintOperatorsSid,   kAuthNt,      2, { SECURITY_BUILTIN_DOMAIN_RID, 550 } },
    { WinBuiltinBackupOperatorsSid,  kAuthNt,      2, { SECURITY_BUILTIN_DOMAIN_RID, 551 } },
    { WinBuiltinReplicatorSid,       kAuthNt,      2, { SECURITY_BUILTIN_DOMAIN_RID, 552 } },
};

static const WellKnownSidDef* FindWellKnownSid(WELL_KNOWN_SID_TYPE type) {
    for (size_t i = 0; i < sizeof(kWellKnownSids) / sizeof(kWellKnownSids[0]); ++i) {
        if (kWellKnownSids[i].type == type) return &kWellKnownSids[i];
    }
    return nullptr;
}

static bool BuildSid(const WellKnownSidDef& def, PSID sid) {
    PSID_IDENTIFIER_AUTHORITY auth =
        const_cast<PSID_IDENTIFIER_AUTHORITY>(&kAuthorities[def.authority]);
    if (!InitializeSid(sid, auth, def.subAuthCount)) return false;
    for (BYTE i = 0; i < def.subAuthCount; ++i) {
        *GetSidSubAuthority(sid, i) = def.rid[i];
    }
    return true;
}

struct SidAlias {
    LPCWSTR             alias;
    WELL_KNOWN_SID_TYPE type;
};

static const SidAlias kSidAliases[] = {
    { L"WD", WinWorldSid },
    { L"SY", WinLocalSystemSid },
    { L"BA", WinBuiltinAdministratorsSid },
    { L"BU", WinBuiltinUsersSid },
    { L"BG", WinBuiltinGuestsSid },
    { L"PU", WinBuiltinPowerUsersSid },
    { L"AO", WinBuiltinAccountOperatorsSid },
    { L"SO", WinBuiltinSystemOperatorsSid },
    { L"PO", WinBuiltinPrintOperatorsSid },
    { L"BO", WinBuiltinBackupOperatorsSid },
    { L"RE", WinBuiltinReplicatorSid },
    { L"CO", WinCreatorOwnerSid },
    { L"CG", WinCreatorGroupSid },
    { L"NU", WinNetworkSid },
    { L"IU", WinInteractiveSid },
    { L"SU", WinServiceSid },
    { L"AN", WinAnonymousSid },
    { L"AU", WinAuthenticatedUserSid },
    { L"RC", WinRestrictedCodeSid },
    { L"RESTRICTED", WinRestrictedCodeSid },
    { L"WR", WinWriteRestrictedCodeSid },
    { L"LS", WinLocalServiceSid },
    { L"NS", WinNetworkServiceSid },
    { L"PS", WinSelfSid },
};

static const WellKnownSidDef* FindDefByAlias(LPCWSTR str) {
    for (size_t i = 0; i < sizeof(kSidAliases) / sizeof(kSidAliases[0]); ++i) {
        if (EqualsNoCase(str, kSidAliases[i].alias)) {
            return FindWellKnownSid(kSidAliases[i].type);
        }
    }
    return nullptr;
}

static DWORD ParseNumericSid(LPCWSTR str, PSID* outSid) {
    if ((str[0] != L'S' && str[0] != L's') || str[1] != L'-') {
        return ERROR_INVALID_SID;
    }
    LPCWSTR p = str + 2;

    ULONG revision;
    if (!ParseDecimal<ULONG>(p, &p, &revision) || revision != SID_REVISION || *p != L'-') {
        return ERROR_INVALID_SID;
    }
    ++p;

    ULONG64 authority;
    if (p[0] == L'0' && (p[1] == L'x' || p[1] == L'X')) {
        if (!ParseHex48(p + 2, &p, &authority)) return ERROR_INVALID_SID;
    } else {
        if (!ParseDecimal<ULONG64>(p, &p, &authority)) return ERROR_INVALID_SID;
        if (authority > 0xFFFFFFFFFFFFULL) return ERROR_INVALID_SID;
    }

    SID_IDENTIFIER_AUTHORITY sia = {};
    for (int i = 0; i < 6; ++i) {
        sia.Value[5 - i] = static_cast<BYTE>((authority >> (8 * i)) & 0xFF);
    }

    DWORD subAuths[SID_MAX_SUB_AUTHORITIES] = {};
    UCHAR count = 0;
    while (*p != L'\0') {
        if (*p != L'-') return ERROR_INVALID_SID;
        ++p;
        if (count >= SID_MAX_SUB_AUTHORITIES) return ERROR_INVALID_SID;

        ULONG value;
        if (!ParseDecimal<ULONG>(p, &p, &value)) return ERROR_INVALID_SID;
        subAuths[count++] = value;
    }

    PSID sid = LocalAlloc(LPTR, GetSidLengthRequired(count));
    if (!sid) return ERROR_NOT_ENOUGH_MEMORY;

    if (!InitializeSid(sid, &sia, count)) {
        DWORD err = GetLastError();
        LocalFree(sid);
        return err ? err : ERROR_INVALID_SID;
    }
    for (UCHAR i = 0; i < count; ++i) {
        *GetSidSubAuthority(sid, i) = subAuths[i];
    }

    *outSid = sid;
    return ERROR_SUCCESS;
}

// ============================================================================
// Exports
// ============================================================================

typedef BOOLEAN (APIENTRY *PFN_SystemFunction036)(PVOID, ULONG);

extern "C" BOOLEAN APIENTRY LibSystemFunction036(PVOID RandomBuffer, ULONG RandomBufferLength) {
    COMPAT_PROC(pfn, PFN_SystemFunction036, L"advapi32.dll", "SystemFunction036");
    if (pfn) return pfn(RandomBuffer, RandomBufferLength);

    static PVOID volatile s_provider = NULL;

    HCRYPTPROV prov = reinterpret_cast<HCRYPTPROV>(
        InterlockedCompareExchangePointer(&s_provider, NULL, NULL));
    if (!prov) {
        HCRYPTPROV fresh = 0;
        if (!CryptAcquireContextW(&fresh, NULL, NULL, PROV_RSA_FULL,
                                  CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
            return FALSE;
        }
        PVOID prev = InterlockedCompareExchangePointer(
            &s_provider, reinterpret_cast<PVOID>(fresh), NULL);
        if (prev) {
            CryptReleaseContext(fresh, 0);
            prov = reinterpret_cast<HCRYPTPROV>(prev);
        } else {
            prov = fresh;
        }
    }

    return CryptGenRandom(prov, RandomBufferLength, static_cast<BYTE*>(RandomBuffer)) ? TRUE : FALSE;
}

typedef BOOL (WINAPI *PFN_CreateWellKnownSid)(WELL_KNOWN_SID_TYPE, PSID, PSID, DWORD*);

extern "C" BOOL WINAPI LibCreateWellKnownSid(
    WELL_KNOWN_SID_TYPE WellKnownSidType,
    PSID DomainSid,
    PSID pSid,
    DWORD* cbSid
) {
    COMPAT_PROC(pfn, PFN_CreateWellKnownSid, L"advapi32.dll", "CreateWellKnownSid");
    if (pfn) return pfn(WellKnownSidType, DomainSid, pSid, cbSid);

    if (!cbSid) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    const WellKnownSidDef* def = FindWellKnownSid(WellKnownSidType);
    if (!def) {
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    DWORD needed = GetSidLengthRequired(def->subAuthCount);
    if (!pSid || *cbSid < needed) {
        *cbSid = needed;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    if (!BuildSid(*def, pSid)) return FALSE;
    *cbSid = needed;
    return TRUE;
}

typedef BOOL (WINAPI *PFN_ConvertStringSidToSidW)(LPCWSTR, PSID*);

extern "C" BOOL WINAPI LibConvertStringSidToSidW(LPCWSTR StringSid, PSID* Sid) {
    if (!StringSid || !Sid) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!IsWhistlerBuild2542OrOlder()) {
        COMPAT_PROC(pfn, PFN_ConvertStringSidToSidW, L"advapi32.dll", "ConvertStringSidToSidW");
        if (pfn && pfn(StringSid, Sid)) return TRUE;
    }

    if ((StringSid[0] == L'S' || StringSid[0] == L's') && StringSid[1] == L'-') {
        DWORD err = ParseNumericSid(StringSid, Sid);
        if (err == ERROR_SUCCESS) return TRUE;
        SetLastError(err);
        return FALSE;
    }

    if (const WellKnownSidDef* def = FindDefByAlias(StringSid)) {
        BYTE stackSid[SECURITY_MAX_SID_SIZE];
        if (BuildSid(*def, stackSid)) {
            DWORD len = GetLengthSid(stackSid);
            PSID heapSid = LocalAlloc(LPTR, len);
            if (!heapSid) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }
            if (CopySid(len, heapSid, stackSid)) {
                *Sid = heapSid;
                return TRUE;
            }
            LocalFree(heapSid);
            return FALSE;
        }
    }

    SetLastError(ERROR_INVALID_SID);
    return FALSE;
}
