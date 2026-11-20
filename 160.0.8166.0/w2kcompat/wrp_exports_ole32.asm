.model flat

EXTERN _LibCoGetContextToken@4 : PROC

.code

PUBLIC _CoGetContextToken@4
_CoGetContextToken@4 PROC
    jmp _LibCoGetContextToken@4
_CoGetContextToken@4 ENDP

CONST SEGMENT

PUBLIC __imp__CoGetContextToken@4
__imp__CoGetContextToken@4 dd _LibCoGetContextToken@4

CONST ENDS

END