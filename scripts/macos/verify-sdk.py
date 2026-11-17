import hashlib,json,sys
from pathlib import Path
lock=json.loads(Path("scripts/macos/qt-sdk-lock.json").read_text())
directory=Path(sys.argv[1])
for item in lock["archives"]:
    matches=list(directory.rglob(item["name"])) + list(directory.rglob(item["name"].removeprefix(lock["build"])))
    if len(matches)!=1: raise SystemExit("Expected exactly one pinned SDK archive: "+item["name"])
    with matches[0].open("rb") as f: actual=hashlib.file_digest(f,"sha256").hexdigest()
    if actual!=item["sha256"]: raise SystemExit("SDK SHA256 mismatch: "+item["name"])
    print("SDK SHA256 OK:",item["name"],actual)
