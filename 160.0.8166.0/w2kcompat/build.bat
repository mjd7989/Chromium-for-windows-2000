@echo off
set "LLVM_BIN=E:\src\chromium\src\third_party\llvm-build\Release+Asserts\bin"

for %%I in (kernel32 advapi32 iphlpapi psapi winhttp ws2_32 wtsapi32 ole32) do (
    echo ============================================================================
    echo Building wrp_%%I.lib...
    echo ============================================================================
    "%LLVM_BIN%\clang-cl.exe" -m32 /c /O2 /EHsc -fno-builtin /std:c++17 wrp_%%I.cpp /Fo:wrp_%%I.obj
    "%LLVM_BIN%\llvm-ml.exe" -m32 /safeseh -coff-safe-seh /c wrp_exports_%%I.asm /Fo wrp_exports_%%I.obj
    "%LLVM_BIN%\lld-link.exe" /lib /out:wrp_%%I.lib wrp_%%I.obj wrp_exports_%%I.obj
)

echo.
echo All libraries compiled successfully!