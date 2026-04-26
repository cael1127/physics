# FullPhysicsC

A modular physics engine in **C** with **2D rigid bodies**, optional **smoke/dye fluid**, **particles**, and simplified **aero drag**, composed through a small **`FpScene`** runtime and a **Win32** demo.

## Build (Windows x64)

### One-click prerequisites (recommended)

Double-click **`install_prerequisites.cmd`** in the repo root (or run `scripts\install_prerequisites.ps1` in PowerShell). It uses **winget** to install anything missing:

- **CMake** (`Kitware.CMake`)
- **Windows 10 SDK** (for `rc.exe` / linker tools with MSVC)
- **Visual Studio 2022 Build Tools** with the **Desktop development with C++**–equivalent **VCTools** workload

Run as **Administrator** if installers prompt for elevation. Afterward, open a **new** terminal so `PATH` picks up CMake.

### Manual prereqs

Visual Studio 2022 (MSVC) or LLVM/clang-cl, CMake 3.20+

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

Run: `.\build\Release\fullphysics_app.exe`  
Tests: `ctest --test-dir build -C Release`

### One-click build + launch

Double-click **`run_fullphysics.cmd`** (or run `scripts\run_app.ps1` in PowerShell). It finds **MSVC** (`vcvars64.bat`), **configures** `build\` with **NMake + Release** if there is no CMake cache yet, **builds**, then starts **`fullphysics_app.exe`**.

- **`run_fullphysics.cmd -RunOnly`** — only launch if an exe is already under `build\` (skips configure/build).
- **`run_fullphysics.cmd -Clean`** — delete `build\`, then configure (NMake) and build (fixes broken caches / missing `Makefile`).

If you use the **Visual Studio** generator manually (`cmake -A x64`), the script detects it and runs **`cmake --build build --config Release`**. For **NMake** caches it runs **`cmake --build build`** (no stray `Makefile` lookup from the wrong generator).

### Single EXE launcher

Use **`FullPhysicsLauncher.exe`** in the repo root.

- Double-click `FullPhysicsLauncher.exe` to:
  1) run `scripts\install_prerequisites.ps1`,
  2) build and launch via `scripts\run_app.ps1`.
- Optional args (from terminal): `--run-only`, `--clean`, `--skip-install`.
- Rebuild the launcher from source with `build_launcher.cmd`.

## CMake targets

- `fullphysics_core`, `fullphysics_rigid2d`, `fullphysics_fluid2d`, `fullphysics_particles2d`, `fullphysics_aero2d`, `fullphysics_runtime`, `fullphysics_draw`, `fullphysics_simd`

Umbrella include: `src/fullphysics.h`.

## Reproducibility

Deterministic build flags are enabled by default with `-DFP_DETERMINISTIC=ON`.

- Determinism and runtime requirements: `docs/reproducibility/determinism_contract.md`
- Disable strict deterministic flags only for local exploration:
  - `cmake -S . -B build -A x64 -DFP_DETERMINISTIC=OFF`

## Documentation Tracks

- Learn path: `docs/learn/`
- Use path: `docs/use/`
- Contribute path: `docs/contribute/`
- Classroom labs: `docs/classroom/`

## Minimal integration

```c
#include "fullphysics.h"
void example(void) {
  FpSceneDesc desc; fp_scene_desc_default(&desc);
  FpScene scene; fp_scene_init(&scene, &desc);
  fp_scene_load_demo(&scene, FP_SCENE_DEMO_STACK);
  fp_scene_step(&scene, 1.0f/60.0f, 10, 4);
  fp_scene_destroy(&scene);
}
```

Link `fullphysics_runtime`; add `fullphysics_draw` + `fullphysics_platform_win32` for Win32 drawing.

## Demo controls

Space pause, Esc quit, R reset, 1-3 scenes, F/C/W toggles, Q/E wind, O/L gravity, D dye, H hold dye, P particles.
