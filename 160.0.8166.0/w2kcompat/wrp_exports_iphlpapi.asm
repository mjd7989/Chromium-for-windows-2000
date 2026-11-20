.model flat

EXTERN _LibGetAdaptersAddresses@20 : PROC
EXTERN _LibCancelIPChangeNotify@4 : PROC

.code

PUBLIC _GetAdaptersAddresses@20
_GetAdaptersAddresses@20 PROC
    jmp _LibGetAdaptersAddresses@20
_GetAdaptersAddresses@20 ENDP

PUBLIC _CancelIPChangeNotify@4
_CancelIPChangeNotify@4 PROC
    jmp _LibCancelIPChangeNotify@4
_CancelIPChangeNotify@4 ENDP

CONST SEGMENT

PUBLIC __imp__GetAdaptersAddresses@20
__imp__GetAdaptersAddresses@20 dd _LibGetAdaptersAddresses@20

PUBLIC __imp__CancelIPChangeNotify@4
__imp__CancelIPChangeNotify@4 dd _LibCancelIPChangeNotify@4

CONST ENDS

END