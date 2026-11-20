.model flat

EXTERN _LibGetPerformanceInfo@8 : PROC

.code

PUBLIC _GetPerformanceInfo@8
_GetPerformanceInfo@8 PROC
    jmp _LibGetPerformanceInfo@8
_GetPerformanceInfo@8 ENDP

CONST SEGMENT

PUBLIC __imp__GetPerformanceInfo@8
__imp__GetPerformanceInfo@8 dd _LibGetPerformanceInfo@8

CONST ENDS

END