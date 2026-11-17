#!/usr/bin/env python3
"""Prepare and inspect a candidate DMG on macOS; never publish or execute HID."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import plistlib
import re
import shutil
import subprocess
import sys


def run(*args, check=True):
    result = subprocess.run(args, capture_output=True, check=False)
    if check and result.returncode:
        raise RuntimeError(f"{args[0]} failed ({result.returncode}): "
                           + result.stderr.decode(errors="replace"))
    return result


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def inventory(root):
    entries = []
    for path in sorted(root.rglob("*")):
        relative = str(path.relative_to(root))
        if path.is_symlink():
            resolved = path.resolve()
            if not resolved.is_relative_to(root.resolve()):
                raise RuntimeError(f"External symlink: {relative}")
            entries.append({"path": relative, "symlink": os.readlink(path)})
        elif path.is_file():
            entries.append({"path": relative, "sha256": digest(path)})
    return entries


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--app", type=Path, required=True)
    parser.add_argument("--qt-root", type=Path, required=True)
    parser.add_argument("--revision", required=True)
    parser.add_argument("--notices", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    if sys.platform != "darwin":
        parser.error("macOS host required; no simulated packaging")
    if not re.fullmatch(r"[0-9a-f]{40}", args.revision):
        parser.error("revision must be the full source commit SHA")
    source_app = args.app.resolve(strict=True)
    qt = args.qt_root.resolve(strict=True)
    notices = args.notices.resolve(strict=True)
    if source_app.suffix != ".app" or not notices.is_dir():
        parser.error("existing .app and notices directory required")
    # Never overwrite another candidate, nor mutate the original app/SDK.
    args.out.mkdir(parents=True, exist_ok=False)
    out = args.out.resolve()
    stage = out / "stage"
    stage.mkdir()
    app = stage / "PeriVolt.app"
    run("ditto", str(source_app), str(app))
    destination = app / "Contents/Resources/licenses"
    if destination.exists():
        raise RuntimeError("bundle already has licenses; refusing overwrite")
    if any(p.is_symlink() for p in notices.rglob("*")):
        raise RuntimeError("notices directory must not contain symlinks")
    shutil.copytree(notices, destination)
    deployed = run(str(qt / "bin/macdeployqt"), str(app), "-verbose=2")
    (out / "macdeployqt.log").write_bytes(deployed.stdout + deployed.stderr)
    with (app / "Contents/Info.plist").open("rb") as stream:
        info = plistlib.load(stream)
    executable = app / "Contents/MacOS" / info["CFBundleExecutable"]
    architectures = run("lipo", "-archs", str(executable)).stdout.decode().split()
    cocoa = app / "Contents/PlugIns/platforms/libqcocoa.dylib"
    if not cocoa.is_file():
        raise RuntimeError("Cocoa platform plugin missing after deployment")
    binaries = []
    for path in sorted(app.rglob("*")):
        if path.is_symlink() or not path.is_file():
            continue
        if "Mach-O" not in run("file", "-b", str(path)).stdout.decode():
            continue
        archs = run("lipo", "-archs", str(path)).stdout.decode().split()
        if not set(architectures).issubset(archs):
            raise RuntimeError(f"Missing executable architecture in {path.name}")
        links = run("otool", "-L", str(path)).stdout.decode()
        for line in links.splitlines()[1:]:
            dependency = line.strip().split(" (", 1)[0]
            if dependency.startswith("/") and not dependency.startswith(
                    ("/System/Library/", "/usr/lib/")):
                raise RuntimeError(f"Unbundled absolute dependency: {dependency}")
        binaries.append({"path": str(path.relative_to(app)), "architectures": archs,
                         "links": links, "build_version": run(
                             "vtool", "-show-build", str(path)).stdout.decode()})
    if not binaries:
        raise RuntimeError("No Mach-O binaries found")
    signing = run("codesign", "-dvv", "--entitlements", ":-", str(app), check=False)
    # Report identity/entitlements; do not create a signature or bypass Gatekeeper.
    identities = run("security", "find-identity", "-v", "-p", "codesigning", check=False)
    (stage / "Applications").symlink_to("/Applications")
    manifest = inventory(app)
    dmg = out / f"PeriVolt-{args.revision[:12]}-{'-'.join(architectures)}-test.dmg"
    run("hdiutil", "create", "-volname", "PeriVolt Test", "-srcfolder", str(stage),
        "-format", "UDZO", str(dmg))
    run("hdiutil", "verify", str(dmg))
    mounted = plistlib.loads(run("hdiutil", "attach", "-readonly", "-nobrowse",
                                "-plist", str(dmg)).stdout)
    points = [e["mount-point"] for e in mounted["system-entities"] if "mount-point" in e]
    if len(points) != 1:
        raise RuntimeError("Unexpected DMG mount layout; inspect mounts manually")
    mount = Path(points[0])
    try:
        if inventory(mount / "PeriVolt.app") != manifest:
            raise RuntimeError("Mounted bundle differs from staged bundle")
        if not (mount / "Applications").is_symlink():
            raise RuntimeError("Applications installation shortcut missing")
    finally:
        run("hdiutil", "detach", str(mount))
    report = {"revision": args.revision, "architectures": architectures,
              "info_plist_minimum_os": info.get("LSMinimumSystemVersion"),
              "qt_version": run(str(qt / "bin/qmake"), "-query", "QT_VERSION").stdout.decode().strip(),
              "sdk": run("xcrun", "--show-sdk-version").stdout.decode().strip(),
              "host": run("sw_vers").stdout.decode(), "binaries": binaries,
              "bundle_files": manifest, "dmg": dmg.name, "sha256": digest(dmg),
              "codesign": (signing.stdout + signing.stderr).decode(errors="replace"),
              "available_signing_identities": identities.stdout.decode(errors="replace"),
              "dmg_verified_and_mounted_readonly": True,
              "gui_or_hardware_tested": False, "notarized": False,
              "release_approved": False,
              "limits": "Candidate inspection only. Requires integration acceptance, binary legal review, "
                        "Qt replacement/execution proof and explicit distribution terms before publication."}
    (out / "artifact-evidence.json").write_text(json.dumps(report, indent=2) + "\n")
    (out / "SHA256SUMS").write_text(f"{report['sha256']}  {dmg.name}\n")
    print(json.dumps({"dmg": str(dmg), "sha256": report["sha256"], "release_approved": False}))


if __name__ == "__main__":
    main()

