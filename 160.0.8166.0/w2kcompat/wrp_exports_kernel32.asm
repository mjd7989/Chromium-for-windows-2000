.model flat

EXTERN _LibCheckRemoteDebuggerPresent@8 : PROC
EXTERN _LibDecodePointer@4 : PROC
EXTERN _LibEncodePointer@4 : PROC
EXTERN _LibGetLogicalProcessorInformation@8 : PROC
EXTERN _LibGetProcessHandleCount@8 : PROC
EXTERN _LibGetProcessId@4 : PROC
EXTERN _LibGetNativeSystemInfo@4 : PROC
EXTERN _LibConvertFiberToThread@0 : PROC
EXTERN _LibCreateFiberEx@20 : PROC
EXTERN _LibGetModuleHandleExA@12 : PROC
EXTERN _LibGetModuleHandleExW@12 : PROC
EXTERN _LibRtlCaptureContext@4 : PROC
EXTERN _LibRtlCaptureStackBackTrace@16 : PROC
EXTERN _LibHeapSetInformation@16 : PROC
EXTERN _LibHeapQueryInformation@20 : PROC
EXTERN _LibIsWow64Process@8 : PROC
EXTERN _LibAttachConsole@4 : PROC
EXTERN _LibGetGeoInfoW@20 : PROC
EXTERN _LibGetUserGeoID@4 : PROC
EXTERN _LibGetVolumePathNamesForVolumeNameW@16 : PROC
EXTERN _LibIsProcessInJob@12 : PROC
EXTERN _LibTzSpecificLocalTimeToSystemTime@12 : PROC
EXTERN _LibWTSGetActiveConsoleSessionId@0 : PROC
EXTERN _LibInitializeSListHead@4 : PROC
EXTERN _LibInterlockedPushEntrySList@8 : PROC
EXTERN _LibInterlockedFlushSList@4 : PROC
EXTERN _LibAddVectoredExceptionHandler@8 : PROC
EXTERN _LibRemoveVectoredExceptionHandler@4 : PROC

.code

PUBLIC _CheckRemoteDebuggerPresent@8
_CheckRemoteDebuggerPresent@8 PROC
    jmp _LibCheckRemoteDebuggerPresent@8
_CheckRemoteDebuggerPresent@8 ENDP

PUBLIC _DecodePointer@4
_DecodePointer@4 PROC
    jmp _LibDecodePointer@4
_DecodePointer@4 ENDP

PUBLIC _EncodePointer@4
_EncodePointer@4 PROC
    jmp _LibEncodePointer@4
_EncodePointer@4 ENDP

PUBLIC _GetLogicalProcessorInformation@8
_GetLogicalProcessorInformation@8 PROC
    jmp _LibGetLogicalProcessorInformation@8
_GetLogicalProcessorInformation@8 ENDP

PUBLIC _GetProcessHandleCount@8
_GetProcessHandleCount@8 PROC
    jmp _LibGetProcessHandleCount@8
_GetProcessHandleCount@8 ENDP

PUBLIC _GetProcessId@4
_GetProcessId@4 PROC
    jmp _LibGetProcessId@4
_GetProcessId@4 ENDP

PUBLIC _GetNativeSystemInfo@4
_GetNativeSystemInfo@4 PROC
    jmp _LibGetNativeSystemInfo@4
_GetNativeSystemInfo@4 ENDP

PUBLIC _ConvertFiberToThread@0
_ConvertFiberToThread@0 PROC
    jmp _LibConvertFiberToThread@0
_ConvertFiberToThread@0 ENDP

PUBLIC _CreateFiberEx@20
_CreateFiberEx@20 PROC
    jmp _LibCreateFiberEx@20
_CreateFiberEx@20 ENDP

PUBLIC _GetModuleHandleExA@12
_GetModuleHandleExA@12 PROC
    jmp _LibGetModuleHandleExA@12
_GetModuleHandleExA@12 ENDP

PUBLIC _GetModuleHandleExW@12
_GetModuleHandleExW@12 PROC
    jmp _LibGetModuleHandleExW@12
_GetModuleHandleExW@12 ENDP

PUBLIC _RtlCaptureContext@4
_RtlCaptureContext@4 PROC
    jmp _LibRtlCaptureContext@4
_RtlCaptureContext@4 ENDP

PUBLIC _RtlCaptureStackBackTrace@16
_RtlCaptureStackBackTrace@16 PROC
    jmp _LibRtlCaptureStackBackTrace@16
_RtlCaptureStackBackTrace@16 ENDP

PUBLIC _HeapSetInformation@16
_HeapSetInformation@16 PROC
    jmp _LibHeapSetInformation@16
_HeapSetInformation@16 ENDP

PUBLIC _HeapQueryInformation@20
_HeapQueryInformation@20 PROC
    jmp _LibHeapQueryInformation@20
_HeapQueryInformation@20 ENDP

PUBLIC _IsWow64Process@8
_IsWow64Process@8 PROC
    jmp _LibIsWow64Process@8
_IsWow64Process@8 ENDP

PUBLIC _AttachConsole@4
_AttachConsole@4 PROC
    jmp _LibAttachConsole@4
_AttachConsole@4 ENDP

PUBLIC _GetGeoInfoW@20
_GetGeoInfoW@20 PROC
    jmp _LibGetGeoInfoW@20
_GetGeoInfoW@20 ENDP

PUBLIC _GetUserGeoID@4
_GetUserGeoID@4 PROC
    jmp _LibGetUserGeoID@4
_GetUserGeoID@4 ENDP

PUBLIC _GetVolumePathNamesForVolumeNameW@16
_GetVolumePathNamesForVolumeNameW@16 PROC
    jmp _LibGetVolumePathNamesForVolumeNameW@16
_GetVolumePathNamesForVolumeNameW@16 ENDP

PUBLIC _IsProcessInJob@12
_IsProcessInJob@12 PROC
    jmp _LibIsProcessInJob@12
_IsProcessInJob@12 ENDP

PUBLIC _TzSpecificLocalTimeToSystemTime@12
_TzSpecificLocalTimeToSystemTime@12 PROC
    jmp _LibTzSpecificLocalTimeToSystemTime@12
_TzSpecificLocalTimeToSystemTime@12 ENDP

PUBLIC _WTSGetActiveConsoleSessionId@0
_WTSGetActiveConsoleSessionId@0 PROC
    jmp _LibWTSGetActiveConsoleSessionId@0
_WTSGetActiveConsoleSessionId@0 ENDP

PUBLIC _InitializeSListHead@4
_InitializeSListHead@4 PROC
    jmp _LibInitializeSListHead@4
_InitializeSListHead@4 ENDP

PUBLIC _InterlockedPushEntrySList@8
_InterlockedPushEntrySList@8 PROC
    jmp _LibInterlockedPushEntrySList@8
_InterlockedPushEntrySList@8 ENDP

PUBLIC _InterlockedFlushSList@4
_InterlockedFlushSList@4 PROC
    jmp _LibInterlockedFlushSList@4
_InterlockedFlushSList@4 ENDP

PUBLIC _AddVectoredExceptionHandler@8
_AddVectoredExceptionHandler@8 PROC
    jmp _LibAddVectoredExceptionHandler@8
_AddVectoredExceptionHandler@8 ENDP

PUBLIC _RemoveVectoredExceptionHandler@4
_RemoveVectoredExceptionHandler@4 PROC
    jmp _LibRemoveVectoredExceptionHandler@4
_RemoveVectoredExceptionHandler@4 ENDP

CONST SEGMENT

PUBLIC __imp__CheckRemoteDebuggerPresent@8
__imp__CheckRemoteDebuggerPresent@8 dd _LibCheckRemoteDebuggerPresent@8

PUBLIC __imp__DecodePointer@4
__imp__DecodePointer@4 dd _LibDecodePointer@4

PUBLIC __imp__EncodePointer@4
__imp__EncodePointer@4 dd _LibEncodePointer@4

PUBLIC __imp__GetLogicalProcessorInformation@8
__imp__GetLogicalProcessorInformation@8 dd _LibGetLogicalProcessorInformation@8

PUBLIC __imp__GetProcessHandleCount@8
__imp__GetProcessHandleCount@8 dd _LibGetProcessHandleCount@8

PUBLIC __imp__GetProcessId@4
__imp__GetProcessId@4 dd _LibGetProcessId@4

PUBLIC __imp__GetNativeSystemInfo@4
__imp__GetNativeSystemInfo@4 dd _LibGetNativeSystemInfo@4

PUBLIC __imp__ConvertFiberToThread@0
__imp__ConvertFiberToThread@0 dd _LibConvertFiberToThread@0

PUBLIC __imp__CreateFiberEx@20
__imp__CreateFiberEx@20 dd _LibCreateFiberEx@20

PUBLIC __imp__GetModuleHandleExA@12
__imp__GetModuleHandleExA@12 dd _LibGetModuleHandleExA@12

PUBLIC __imp__GetModuleHandleExW@12
__imp__GetModuleHandleExW@12 dd _LibGetModuleHandleExW@12

PUBLIC __imp__RtlCaptureContext@4
__imp__RtlCaptureContext@4 dd _LibRtlCaptureContext@4

PUBLIC __imp__RtlCaptureStackBackTrace@16
__imp__RtlCaptureStackBackTrace@16 dd _LibRtlCaptureStackBackTrace@16

PUBLIC __imp__HeapSetInformation@16
__imp__HeapSetInformation@16 dd _LibHeapSetInformation@16

PUBLIC __imp__HeapQueryInformation@20
__imp__HeapQueryInformation@20 dd _LibHeapQueryInformation@20

PUBLIC __imp__IsWow64Process@8
__imp__IsWow64Process@8 dd _LibIsWow64Process@8

PUBLIC __imp__AttachConsole@4
__imp__AttachConsole@4 dd _LibAttachConsole@4

PUBLIC __imp__GetGeoInfoW@20
__imp__GetGeoInfoW@20 dd _LibGetGeoInfoW@20

PUBLIC __imp__GetUserGeoID@4
__imp__GetUserGeoID@4 dd _LibGetUserGeoID@4

PUBLIC __imp__GetVolumePathNamesForVolumeNameW@16
__imp__GetVolumePathNamesForVolumeNameW@16 dd _LibGetVolumePathNamesForVolumeNameW@16

PUBLIC __imp__IsProcessInJob@12
__imp__IsProcessInJob@12 dd _LibIsProcessInJob@12

PUBLIC __imp__TzSpecificLocalTimeToSystemTime@12
__imp__TzSpecificLocalTimeToSystemTime@12 dd _LibTzSpecificLocalTimeToSystemTime@12

PUBLIC __imp__WTSGetActiveConsoleSessionId@0
__imp__WTSGetActiveConsoleSessionId@0 dd _LibWTSGetActiveConsoleSessionId@0

PUBLIC __imp__InitializeSListHead@4
__imp__InitializeSListHead@4 dd _LibInitializeSListHead@4

PUBLIC __imp__InterlockedPushEntrySList@8
__imp__InterlockedPushEntrySList@8 dd _LibInterlockedPushEntrySList@8

PUBLIC __imp__InterlockedFlushSList@4
__imp__InterlockedFlushSList@4 dd _LibInterlockedFlushSList@4

PUBLIC __imp__AddVectoredExceptionHandler@8
__imp__AddVectoredExceptionHandler@8 dd _LibAddVectoredExceptionHandler@8

PUBLIC __imp__RemoveVectoredExceptionHandler@4
__imp__RemoveVectoredExceptionHandler@4 dd _LibRemoveVectoredExceptionHandler@4

CONST ENDS

END