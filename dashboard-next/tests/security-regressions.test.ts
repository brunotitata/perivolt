import assert from 'node:assert/strict';
import test from 'node:test';
import { sanitizeEvidenceText } from '../server/export';
import { GET } from '../app/api/stream/route';

test('SEC-01 oculta caminhos embutidos e preserva evidencia relativa/URL', () => {
  for (const value of ['Log em C:\\Users\\Example\\private\\trace.txt', 'Log em /home/example/private/trace.txt', 'Log em \\\\host\\private\\trace.txt', 'Log em ../private/trace.txt', 'Log em "C:\\User Files\\trace.txt"']) {
    assert.equal(sanitizeEvidenceText(value), '[caminho nao exibido]');
  }
  for (const value of ['server/export.ts', 'Log em server/export.ts', 'Veja https://example.org/log']) {
    assert.equal(sanitizeEvidenceText(value), value);
  }
});

for (const mode of ['before', 'during', 'after', 'cancel'] as const) {
  test(`SEC-02 encerra SSE: ${mode}`, async () => {
    const abort = new AbortController();
    const timers = new Set<ReturnType<typeof setInterval>>();
    const originalSet = globalThis.setInterval;
    const originalClear = globalThis.clearInterval;
    const globals = globalThis as typeof globalThis & { __perivoltSnapshotStore?: unknown };
    const oldStore = globals.__perivoltSnapshotStore;
    let calls = 0;
    let release!: () => void;
    const pending = new Promise<void>(resolve => { release = resolve; });
    globals.__perivoltSnapshotStore = { async get() { calls++; if (mode === 'during') await pending; return { kind: 'error', error: 'snapshot_missing' }; } };
    globalThis.setInterval = ((...args: Parameters<typeof setInterval>) => { const timer = originalSet(...args); timers.add(timer); return timer; }) as typeof setInterval;
    globalThis.clearInterval = ((timer: ReturnType<typeof setInterval>) => { timers.delete(timer); originalClear(timer); }) as typeof clearInterval;
    let response: Response | undefined;
    try {
      if (mode === 'before') abort.abort();
      response = await GET(new Request('http://localhost/api/stream', { signal: abort.signal }));
      if (mode === 'during') { abort.abort(); release(); }
      await new Promise(resolve => setImmediate(resolve));
      if (mode === 'after') abort.abort();
      if (mode === 'cancel') await response.body!.cancel();
      assert.equal(timers.size, 0);
      if (mode === 'before') assert.equal(calls, 0);
    } finally {
      release();
      await response?.body?.cancel();
      for (const timer of timers) originalClear(timer);
      globalThis.setInterval = originalSet;
      globalThis.clearInterval = originalClear;
      globals.__perivoltSnapshotStore = oldStore;
    }
  });
}
import { sampleDocument } from './helpers';
import { computeSnapshotDigest } from '../server/digest';
import { validateSnapshotDocument, toSnapshotResponse } from '../server/snapshot';

test('SEC-01 projecao API oculta caminho sem alterar documento ou digest de origem', () => {
  const document = sampleDocument();
  const tasks = document.tasks as Array<{ evidence_text: string | null }>;
  tasks[0]!.evidence_text = 'Log em C:\\Users\\Example\\private\\trace.txt';
  document.digest = computeSnapshotDigest(document);
  const before = JSON.stringify(document);
  const response = toSnapshotResponse(validateSnapshotDocument(document), '2026-11-17T14:01:00Z');
  assert.equal(response.tasks[0]!.evidence_text, '[caminho nao exibido]');
  assert.equal(response.digest, document.digest);
  assert.equal(JSON.stringify(document), before);
});
test('SEC-01 caminho entre colchetes', () => {
  assert.equal(sanitizeEvidenceText(String.raw`Log em [C:\Users\Example\private\trace.txt]`), '[caminho nao exibido]');
});
