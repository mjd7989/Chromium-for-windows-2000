.model flat

EXTERN _LibWinHttpAddRequestHeaders@16 : PROC
EXTERN _LibWinHttpCloseHandle@4 : PROC
EXTERN _LibWinHttpConnect@16 : PROC
EXTERN _LibWinHttpCrackUrl@16 : PROC
EXTERN _LibWinHttpGetIEProxyConfigForCurrentUser@4 : PROC
EXTERN _LibWinHttpGetProxyForUrl@16 : PROC
EXTERN _LibWinHttpOpen@20 : PROC
EXTERN _LibWinHttpOpenRequest@28 : PROC
EXTERN _LibWinHttpQueryDataAvailable@8 : PROC
EXTERN _LibWinHttpQueryHeaders@24 : PROC
EXTERN _LibWinHttpQueryOption@16 : PROC
EXTERN _LibWinHttpReadData@16 : PROC
EXTERN _LibWinHttpReceiveResponse@8 : PROC
EXTERN _LibWinHttpSendRequest@28 : PROC
EXTERN _LibWinHttpSetOption@16 : PROC
EXTERN _LibWinHttpSetStatusCallback@16 : PROC
EXTERN _LibWinHttpSetTimeouts@20 : PROC
EXTERN _LibWinHttpWriteData@16 : PROC

.code

PUBLIC _WinHttpAddRequestHeaders@16
_WinHttpAddRequestHeaders@16 PROC
    jmp _LibWinHttpAddRequestHeaders@16
_WinHttpAddRequestHeaders@16 ENDP

PUBLIC _WinHttpCloseHandle@4
_WinHttpCloseHandle@4 PROC
    jmp _LibWinHttpCloseHandle@4
_WinHttpCloseHandle@4 ENDP

PUBLIC _WinHttpConnect@16
_WinHttpConnect@16 PROC
    jmp _LibWinHttpConnect@16
_WinHttpConnect@16 ENDP

PUBLIC _WinHttpCrackUrl@16
_WinHttpCrackUrl@16 PROC
    jmp _LibWinHttpCrackUrl@16
_WinHttpCrackUrl@16 ENDP

PUBLIC _WinHttpGetIEProxyConfigForCurrentUser@4
_WinHttpGetIEProxyConfigForCurrentUser@4 PROC
    jmp _LibWinHttpGetIEProxyConfigForCurrentUser@4
_WinHttpGetIEProxyConfigForCurrentUser@4 ENDP

PUBLIC _WinHttpGetProxyForUrl@16
_WinHttpGetProxyForUrl@16 PROC
    jmp _LibWinHttpGetProxyForUrl@16
_WinHttpGetProxyForUrl@16 ENDP

PUBLIC _WinHttpOpen@20
_WinHttpOpen@20 PROC
    jmp _LibWinHttpOpen@20
_WinHttpOpen@20 ENDP

PUBLIC _WinHttpOpenRequest@28
_WinHttpOpenRequest@28 PROC
    jmp _LibWinHttpOpenRequest@28
_WinHttpOpenRequest@28 ENDP

PUBLIC _WinHttpQueryDataAvailable@8
_WinHttpQueryDataAvailable@8 PROC
    jmp _LibWinHttpQueryDataAvailable@8
_WinHttpQueryDataAvailable@8 ENDP

PUBLIC _WinHttpQueryHeaders@24
_WinHttpQueryHeaders@24 PROC
    jmp _LibWinHttpQueryHeaders@24
_WinHttpQueryHeaders@24 ENDP

PUBLIC _WinHttpQueryOption@16
_WinHttpQueryOption@16 PROC
    jmp _LibWinHttpQueryOption@16
_WinHttpQueryOption@16 ENDP

PUBLIC _WinHttpReadData@16
_WinHttpReadData@16 PROC
    jmp _LibWinHttpReadData@16
_WinHttpReadData@16 ENDP

PUBLIC _WinHttpReceiveResponse@8
_WinHttpReceiveResponse@8 PROC
    jmp _LibWinHttpReceiveResponse@8
_WinHttpReceiveResponse@8 ENDP

PUBLIC _WinHttpSendRequest@28
_WinHttpSendRequest@28 PROC
    jmp _LibWinHttpSendRequest@28
_WinHttpSendRequest@28 ENDP

PUBLIC _WinHttpSetOption@16
_WinHttpSetOption@16 PROC
    jmp _LibWinHttpSetOption@16
_WinHttpSetOption@16 ENDP

PUBLIC _WinHttpSetStatusCallback@16
_WinHttpSetStatusCallback@16 PROC
    jmp _LibWinHttpSetStatusCallback@16
_WinHttpSetStatusCallback@16 ENDP

PUBLIC _WinHttpSetTimeouts@20
_WinHttpSetTimeouts@20 PROC
    jmp _LibWinHttpSetTimeouts@20
_WinHttpSetTimeouts@20 ENDP

PUBLIC _WinHttpWriteData@16
_WinHttpWriteData@16 PROC
    jmp _LibWinHttpWriteData@16
_WinHttpWriteData@16 ENDP

CONST SEGMENT

PUBLIC __imp__WinHttpAddRequestHeaders@16
__imp__WinHttpAddRequestHeaders@16 dd _LibWinHttpAddRequestHeaders@16

PUBLIC __imp__WinHttpCloseHandle@4
__imp__WinHttpCloseHandle@4 dd _LibWinHttpCloseHandle@4

PUBLIC __imp__WinHttpConnect@16
__imp__WinHttpConnect@16 dd _LibWinHttpConnect@16

PUBLIC __imp__WinHttpCrackUrl@16
__imp__WinHttpCrackUrl@16 dd _LibWinHttpCrackUrl@16

PUBLIC __imp__WinHttpGetIEProxyConfigForCurrentUser@4
__imp__WinHttpGetIEProxyConfigForCurrentUser@4 dd _LibWinHttpGetIEProxyConfigForCurrentUser@4

PUBLIC __imp__WinHttpGetProxyForUrl@16
__imp__WinHttpGetProxyForUrl@16 dd _LibWinHttpGetProxyForUrl@16

PUBLIC __imp__WinHttpOpen@20
__imp__WinHttpOpen@20 dd _LibWinHttpOpen@20

PUBLIC __imp__WinHttpOpenRequest@28
__imp__WinHttpOpenRequest@28 dd _LibWinHttpOpenRequest@28

PUBLIC __imp__WinHttpQueryDataAvailable@8
__imp__WinHttpQueryDataAvailable@8 dd _LibWinHttpQueryDataAvailable@8

PUBLIC __imp__WinHttpQueryHeaders@24
__imp__WinHttpQueryHeaders@24 dd _LibWinHttpQueryHeaders@24

PUBLIC __imp__WinHttpQueryOption@16
__imp__WinHttpQueryOption@16 dd _LibWinHttpQueryOption@16

PUBLIC __imp__WinHttpReadData@16
__imp__WinHttpReadData@16 dd _LibWinHttpReadData@16

PUBLIC __imp__WinHttpReceiveResponse@8
__imp__WinHttpReceiveResponse@8 dd _LibWinHttpReceiveResponse@8

PUBLIC __imp__WinHttpSendRequest@28
__imp__WinHttpSendRequest@28 dd _LibWinHttpSendRequest@28

PUBLIC __imp__WinHttpSetOption@16
__imp__WinHttpSetOption@16 dd _LibWinHttpSetOption@16

PUBLIC __imp__WinHttpSetStatusCallback@16
__imp__WinHttpSetStatusCallback@16 dd _LibWinHttpSetStatusCallback@16

PUBLIC __imp__WinHttpSetTimeouts@20
__imp__WinHttpSetTimeouts@20 dd _LibWinHttpSetTimeouts@20

PUBLIC __imp__WinHttpWriteData@16
__imp__WinHttpWriteData@16 dd _LibWinHttpWriteData@16

CONST ENDS

END