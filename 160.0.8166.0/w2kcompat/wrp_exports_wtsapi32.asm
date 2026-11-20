.model flat

EXTERN _LibWTSQueryUserToken@8 : PROC
EXTERN _LibWTSRegisterSessionNotification@8 : PROC
EXTERN _LibWTSUnRegisterSessionNotification@4 : PROC
EXTERN _LibWTSQuerySessionInformationW@20 : PROC
EXTERN _LibWTSFreeMemory@4 : PROC

.code

PUBLIC _WTSQueryUserToken@8
_WTSQueryUserToken@8 PROC
    jmp _LibWTSQueryUserToken@8
_WTSQueryUserToken@8 ENDP

PUBLIC _WTSRegisterSessionNotification@8
_WTSRegisterSessionNotification@8 PROC
    jmp _LibWTSRegisterSessionNotification@8
_WTSRegisterSessionNotification@8 ENDP

PUBLIC _WTSUnRegisterSessionNotification@4
_WTSUnRegisterSessionNotification@4 PROC
    jmp _LibWTSUnRegisterSessionNotification@4
_WTSUnRegisterSessionNotification@4 ENDP

PUBLIC _WTSQuerySessionInformationW@20
_WTSQuerySessionInformationW@20 PROC
    jmp _LibWTSQuerySessionInformationW@20
_WTSQuerySessionInformationW@20 ENDP

PUBLIC _WTSFreeMemory@4
_WTSFreeMemory@4 PROC
    jmp _LibWTSFreeMemory@4
_WTSFreeMemory@4 ENDP

CONST SEGMENT

PUBLIC __imp__WTSQueryUserToken@8
__imp__WTSQueryUserToken@8 dd _LibWTSQueryUserToken@8

PUBLIC __imp__WTSRegisterSessionNotification@8
__imp__WTSRegisterSessionNotification@8 dd _LibWTSRegisterSessionNotification@8

PUBLIC __imp__WTSUnRegisterSessionNotification@4
__imp__WTSUnRegisterSessionNotification@4 dd _LibWTSUnRegisterSessionNotification@4

PUBLIC __imp__WTSQuerySessionInformationW@20
__imp__WTSQuerySessionInformationW@20 dd _LibWTSQuerySessionInformationW@20

PUBLIC __imp__WTSFreeMemory@4
__imp__WTSFreeMemory@4 dd _LibWTSFreeMemory@4

CONST ENDS

END