.model flat

EXTERN _Libgetaddrinfo@16 : PROC
EXTERN _Libfreeaddrinfo@4 : PROC
EXTERN _LibWSALookupServiceBeginW@12 : PROC
EXTERN _LibWSALookupServiceNextW@16 : PROC
EXTERN _LibWSALookupServiceEnd@4 : PROC

.code

PUBLIC _getaddrinfo@16
_getaddrinfo@16 PROC
    jmp _Libgetaddrinfo@16
_getaddrinfo@16 ENDP

PUBLIC _freeaddrinfo@4
_freeaddrinfo@4 PROC
    jmp _Libfreeaddrinfo@4
_freeaddrinfo@4 ENDP

PUBLIC _WSALookupServiceBeginW@12
_WSALookupServiceBeginW@12 PROC
    jmp _LibWSALookupServiceBeginW@12
_WSALookupServiceBeginW@12 ENDP

PUBLIC _WSALookupServiceNextW@16
_WSALookupServiceNextW@16 PROC
    jmp _LibWSALookupServiceNextW@16
_WSALookupServiceNextW@16 ENDP

PUBLIC _WSALookupServiceEnd@4
_WSALookupServiceEnd@4 PROC
    jmp _LibWSALookupServiceEnd@4
_WSALookupServiceEnd@4 ENDP

CONST SEGMENT

PUBLIC __imp__getaddrinfo@16
__imp__getaddrinfo@16 dd _Libgetaddrinfo@16

PUBLIC __imp__freeaddrinfo@4
__imp__freeaddrinfo@4 dd _Libfreeaddrinfo@4

PUBLIC __imp__WSALookupServiceBeginW@12
__imp__WSALookupServiceBeginW@12 dd _LibWSALookupServiceBeginW@12

PUBLIC __imp__WSALookupServiceNextW@16
__imp__WSALookupServiceNextW@16 dd _LibWSALookupServiceNextW@16

PUBLIC __imp__WSALookupServiceEnd@4
__imp__WSALookupServiceEnd@4 dd _LibWSALookupServiceEnd@4

CONST ENDS

END