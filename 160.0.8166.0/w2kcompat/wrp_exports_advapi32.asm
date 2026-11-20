.model flat

EXTERN _LibSystemFunction036@8 : PROC
EXTERN _LibCreateWellKnownSid@16 : PROC
EXTERN _LibConvertStringSidToSidW@8 : PROC

.code

PUBLIC _SystemFunction036@8
_SystemFunction036@8 PROC
    jmp _LibSystemFunction036@8
_SystemFunction036@8 ENDP

PUBLIC _CreateWellKnownSid@16
_CreateWellKnownSid@16 PROC
    jmp _LibCreateWellKnownSid@16
_CreateWellKnownSid@16 ENDP

PUBLIC _ConvertStringSidToSidW@8
_ConvertStringSidToSidW@8 PROC
    jmp _LibConvertStringSidToSidW@8
_ConvertStringSidToSidW@8 ENDP

CONST SEGMENT

PUBLIC __imp__SystemFunction036@8
__imp__SystemFunction036@8 dd _LibSystemFunction036@8

PUBLIC __imp__CreateWellKnownSid@16
__imp__CreateWellKnownSid@16 dd _LibCreateWellKnownSid@16

PUBLIC __imp__ConvertStringSidToSidW@8
__imp__ConvertStringSidToSidW@8 dd _LibConvertStringSidToSidW@8

CONST ENDS

END