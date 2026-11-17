"""Collect pinned Qt source and notices for candidate legal inspection, not approval."""
from pathlib import Path
import hashlib
import json
import shutil
import sys
import tarfile
import urllib.request

url = 'https://download.qt.io/official_releases/qt/6.12/6.12.0/submodules/qtbase-everywhere-src-6.12.0.tar.xz'
expected = 'a951bd163c7b80fc6b8c88d7668fb56abf91c152373e13c10666763238131307'
out, notices = map(Path, sys.argv[1:])
out.mkdir(parents=True, exist_ok=False)
archive = out/'qtbase-everywhere-src-6.12.0.tar.xz'
with urllib.request.urlopen(url, timeout=120) as response, archive.open('wb') as stream:
    if not response.url.startswith('https://'):
        raise RuntimeError('HTTPS required for upstream source')
    shutil.copyfileobj(response, stream)
with archive.open('rb') as stream:
    sha = hashlib.file_digest(stream, 'sha256').hexdigest()
if sha != expected:
    raise RuntimeError('Qt source SHA256 mismatch')
source = out/'source'
source.mkdir()
with tarfile.open(archive) as tar:
    tar.extractall(source, filter='data')
base = source/'qtbase-everywhere-src-6.12.0'
destination = notices/'QtBase-6.12.0'
destination.mkdir(parents=True, exist_ok=False)
entries, attributions = [], []
for path in sorted(base.rglob('*')):
    if path.is_symlink() or not path.is_file():
        continue
    relative = path.relative_to(base)
    if relative.parts[0]=='LICENSES' or path.name=='qt_attribution.json' or (
        '3rdparty' in relative.parts and path.name.lower().startswith(
            ('license','licence','copying','copyright','notice'))):
        target = destination/relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, target)
        entries.append({'path':relative.as_posix(),
                        'sha256':hashlib.sha256(path.read_bytes()).hexdigest()})
        if path.name=='qt_attribution.json':
            attributions.append({'path':relative.as_posix(),
                                 'data':json.loads(path.read_text())})
report={'qt_version':'6.12.0','source_url':url,'source_archive_sha256':sha,
        'source_patches_applied':[], 'notices':entries,'attributions':attributions,
        'scope':'Entire upstream QtBase source metadata, not an assertion that every component ships in the app',
        'limits':'SDK build configuration, embedded components and actual runtime bundle still require licensing review; SDK prefix patching by aqt is logged separately'}
(out/'qt-source-evidence.json').write_text(json.dumps(report,indent=2)+'\n')
(destination/'INDEX.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({'source_sha256':sha,'notice_files':len(entries),'attributions':len(attributions)}))
