# Application composition — technical preview, NOT battery MVP

`monitor.hpp` / `monitor.cpp`: C++20 application boundary, using the existing `ui::Snapshot` DTO without Qt linkage. No alert evaluation, HID, shell, server or hardware read. Core and adapters unchanged.

## API and threading

- `Monitor(ttl_ms=30000)` starts empty, consent OFF, OS permission unknown and battery capability unavailable. It does not enumerate or read anything.
- `request_read(bool)` returns `SessionToken {generation,opt_in}`; this is a request, not proof of permission or a start of I/O. OFF/new activation invalidate earlier results and clear session projections. `cancel()` is OFF.
- `source_status(capability,permission)` is for trusted producer status. Denied/unknown permission or unavailable capability invalidates pending tokens and clears battery state; a later grant requires a new request.
- `submit(token,Sample)` is the injectable completion boundary. Requires current consent token, capability and OS permission granted, explicit session origin, declared identity equal to Observation.device, nonempty source/source_epoch. Calls register_device/observe on the real domain. Synthetic/cache/metadata/history origins cannot enter session battery state. Test inputs remain synthetic tests, not proof of any native capability.
- `snapshot(now_ms,epoch)` returns a full value copy of ui::Snapshot, with state from Battery.project and age from the last accepted observation only. Reads never renew measurement time; wrong epoch/future/missing time gives unknown age. Caller/producer must share steady_clock milliseconds and one process epoch (main uses UUID). No wall-clock-to-monotonic conversion is invented.
- `refresh_metadata()` asynchronously calls Hardware `poll_native_metadata` on Apple only. Elsewhere it does nothing; never substitutes Windows data. Explicitly call once from main after construction for registry discovery. It never starts a battery transport, even with opt-in. Completion uses `submit_metadata`; `metadata()` exposes NativeSample presence/count/completeness/error separately. No canonical device IDs exist in that output, so snapshot.devices stays empty.
- Metadata completion is accepted only for matching generation/opt-in and an open monitor; percent/observed_at/live/read_available/active are forcibly removed/disabled, even if a malformed metadata producer sets them. It cannot call Battery.observe. OS error or incomplete enumeration remains unknown, not absent. Metadata receipt never becomes battery observation time.
- `shutdown()` closes the handoff before joining the outstanding registry worker outside the state mutex. OFF invalidates/discards immediately without waiting. Shutdown waits for OS registry enumeration to return: this API has no forced OS cancellation. No HID handle/read exists to cancel. Destructor calls shutdown. As with QObject lifecycle, request/refresh/shutdown are owner-thread operations; submit/snapshot/metadata are protected by the state mutex for cross-thread completion. No UI callback is retained.

Current native source has NO battery reading. Neither this module nor UI binding makes a functioning battery-monitor MVP. Completed session observations are point reads (`active=false`), never labelled continuous monitoring. No test sample is loaded in the app.

## Build integration for DevOps

No root CMake/CI was edited. In app/CMakeLists.txt, after the domain target and before UI target linkage:

```cmake
add_subdirectory(application)
# After qt_add_executable(perivolt_monitor ...):
target_link_libraries(perivolt_monitor PRIVATE perivolt_application)
```

Application CMake owns `perivolt_application` and `application_integration_test`; standalone fallback creates domain target only if absent. Apple target compiles Hardware producer_macos.cpp/discovery_macos.cpp and links IOKit/CoreFoundation. Do not additionally add those sources to this same target. No producer_main/discovery_main in GUI.

Standalone portable test:

```powershell
$toolchain = (Resolve-Path tools/w64devkit/bin).Path
tools/w64devkit/bin/cmake.exe -S app/application -B app/application/build -G Ninja "-DCMAKE_MAKE_PROGRAM=$toolchain/ninja.exe" "-DCMAKE_CXX_COMPILER=$toolchain/g++.exe" -DCMAKE_BUILD_TYPE=Release
tools/w64devkit/bin/cmake.exe --build app/application/build
tools/w64devkit/bin/ctest.exe --test-dir app/application/build --output-on-failure
```

Executed 2026-11-17 Windows/w64devkit: configure/build/CTest exit0, 1/1 integration test PASS with Release assertions retained. Also compiled directly with -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror and executed exit0. First compile before implementation failed due to missing monitor.hpp. Intermediate compile lacked stdexcept, fixed then compiled/passed.

Integration test uses real Battery and Hardware from_discovery, with SYNTHETIC injected data: valid zero/identity/source/age; stale cache from projection; epoch mismatch; duplicate observation; OFF and reactivation discard; synthetic origin rejection; unavailable/missing time; denied permission; disconnected reading; shutdown rejection; metadata absent/candidate/ambiguous/error/incomplete and forced-percent rejection. No native discovery or HID is executed by the test. Apple worker branch requires DevOps Apple build/run; Qt UI/DMG/hardware/macOS runtime NOT verified here.

Generated local artifacts: application/build (CMake objects/binaries), domain/build/application_test.exe (direct test). No Git/index/branch changes, no release/push, no Next/UI/adapters changes by Backend.
