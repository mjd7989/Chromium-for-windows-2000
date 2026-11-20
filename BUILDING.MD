# Building Chromium for Windows 2000

## Prerequisites

- **Visual Studio 2026**
- **Visual Studio 2019**
- **Windows SDK 10.0.22621** and **Windows SDK 10.0.28000** with **Debugging Tools**
- **Git for Windows** (added to `PATH`)
- **Python 3.14.5** (added to `PATH`)

---

## 1. `depot_tools` setup

Clone `depot_tools` and add it to `PATH` **above** Python and Git:

```bat
mkdir C:\src && cd /d C:\src
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
```

Then type:

```bat
gclient
```

---

## 2. Environment variables

Set the required environment variables:

```bat
setx DEPOT_TOOLS_WIN_TOOLCHAIN 0
setx vs2026_install ""
setx GYP_MSVS_VERSION 2019
setx PYTHONIOENCODING utf-8
setx PYTHONUTF8 1
```

---

## 3. Fetch and sync Chromium source code

Create `C:\src\chromium\.gclient`:

```python
solutions = [
  {
    "managed": False,
    "name": "src",
    "url": "https://chromium.googlesource.com/chromium/src.git",
    "custom_deps": {},
    "safesync_url": "",
    "custom_vars": {
      "checkout_pgo_profiles": True,
    },
  },
]
```

Sync to target revision:

```bat
cd /d C:\src\chromium
gclient sync --revision src@160.0.8166.0 --no-history
```

---

## 4. Apply Chromium for Windows 7 Rework patches

```bat
git clone https://github.com/e3kskoy7wqk/Chromium-for-windows-7-REWORK.git
```

Go to the folder containing the patches for the version corresponding to the target version, set `REPO_PATH=C:\src\chromium\src` and change `!REPO_PATH!` to `%REPO_PATH%` in `apply_patches.bat` and run it.

---

## 5. Apply Windows 2000 patch

```bat
cd /d C:\src\chromium\src
git apply --ignore-whitespace <PATH_TO_W2K_PATCH>\160_8166_0_w2k.patch
```

---

## 6. Build and copy W2Kcompat

In `w2kcompat\build.bat`, point `LLVM_BIN` to Chromium's prebuilt LLVM:

```bat
set "LLVM_BIN=C:\src\chromium\src\third_party\llvm-build\Release+Asserts\bin"
```

Run `build.bat`, then copy the output .lib files into `base\w2kcompat`:

```bat
mkdir C:\src\chromium\src\base\w2kcompat
copy /y <PATH_TO_W2KCOMPAT>\*.lib "C:\src\chromium\src\base\w2kcompat\"
```

---

## 7. Copy `d3dcompiler_43.dll` & DirectWrite along with wrappers

Copy `d3dcompiler_43.dll` into the SDK redist folder:

```bat
copy /y <PATH_TO_DLL>\d3dcompiler_43.dll "C:\Program Files (x86)\Windows Kits\10\Redist\D3D\x86\"
```

Copy `DWrite.dll`, `dwrwp.dll`, and `dwrwpcrt.dll` into `third_party\additional_dlls`:

```bat
mkdir C:\src\chromium\src\third_party\additional_dlls
copy /y <PATH_TO_DWRITE_DLLS>\* "C:\src\chromium\src\third_party\additional_dlls\"
```

---

## 8. Hide VS 2026 & swap UCRT

Temporarily hide VS 2026 from `gn` and replace SDK `28000` UCRT libs with `22621`:

```bat
ren "C:\Program Files\Microsoft Visual Studio" "~Microsoft Visual Studio"
ren "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.28000.0\ucrt" "ucrt.bak"
xcopy /s /f /h /y "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\ucrt\*" "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.28000.0\ucrt\"
```

---

## 9. Configure `args.gn`

Run `gn args out\Release_x86` inside `src` and set:

```gn
is_official_build = true
is_component_build = false
is_debug = false
target_os = "win"
target_cpu = "x86"
proprietary_codecs = true
ffmpeg_branding = "Chrome"
treat_warnings_as_errors = false
use_siso = false
use_static_angle = false
enable_nacl = false
google_api_key = ""
google_default_client_id = ""
google_default_client_secret = ""
```

---

## 10. Restore VS 2026 Path

```bat
ren "C:\Program Files\~Microsoft Visual Studio" "Microsoft Visual Studio"
```

---

## 11. Build

```bat
cd /d C:\src\chromium\src
ninja -C out\Release_x86 mini_installer
```
