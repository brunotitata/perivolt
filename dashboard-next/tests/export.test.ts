import assert from "node:assert/strict";
import { readFileSync, writeFileSync } from "node:fs";
import path from "node:path";
import test from "node:test";
import { computeSnapshotDigest } from "../server/digest";
import { exportSnapshotFromFile } from "../server/export";
import { validateSnapshotDocument } from "../server/snapshot";
import { sampleRows, tempDir } from "./helpers";

function inputFile(dir: string, overrides: Record<string, unknown> = {}): string {
  const file = path.join(dir, "linhas.json");
  writeFileSync(
    file,
    JSON.stringify(
      {
        exported_by: "agent-po",
        observed_at: "2026-11-17T14:20:00Z",
        owner_map: {
          DEV: { name: "Desenvolvimento & Dashboard", agent_id: "agent-dev" },
          QA: { name: "Qualidade & Releases", agent_id: "agent-qa" }
        },
        rows: sampleRows(),
        ...overrides
      },
      null,
      2
    ),
    "utf8"
  );
  return file;
}

test("CLI publica snapshot valido com digest e contagens reais", () => {
  const dir = tempDir();
  const input = inputFile(dir);
  const out = path.join(dir, "data", "snapshot.json");
  const summary = exportSnapshotFromFile(input, out, new Date("2026-11-17T14:20:05Z"));
  assert.equal(summary.tasks, 2);
  assert.deepEqual(summary.counts, { in_review: 1, pending: 1 });
  assert.equal(summary.digest.startsWith("sha256:"), true);
  const written = JSON.parse(readFileSync(out, "utf8")) as Record<string, unknown>;
  assert.equal(written["digest"], summary.digest);
  assert.equal(computeSnapshotDigest(written), summary.digest);
  const validated = validateSnapshotDocument(written);
  assert.equal(validated.tasks.length, 2);
  assert.equal(validated.ownerMap["DEV"]?.name, "Desenvolvimento & Dashboard");
  assert.equal(validated.observedAt, "2026-11-17T14:20:00Z");
});

test("status fora do mapa vira unknown, nunca pending", () => {
  const dir = tempDir();
  const rows = sampleRows();
  rows[0]!.status = "EM_TRIAGEM";
  const input = inputFile(dir, { rows });
  const out = path.join(dir, "snapshot.json");
  exportSnapshotFromFile(input, out);
  const written = validateSnapshotDocument(JSON.parse(readFileSync(out, "utf8")));
  const task = written.tasks.find((item) => item.task_code === "T01");
  assert.equal(task?.status_normalized, "unknown");
});

test("locator absoluto ou com traversal e recusado", () => {
  const dir = tempDir();
  const input = inputFile(dir, {
    evidence: [
      {
        id: "ev-1",
        task_code: "T01",
        locator: "C:\\Users\\private\\segredo.md",
        kind: "document",
        sha256: null,
        observed_at: "2026-11-17T14:20:00Z",
        state: "located"
      }
    ]
  });
  assert.throws(() => exportSnapshotFromFile(input, path.join(dir, "snapshot.json")), /inseguro|traversal|locator/i);
});

test("entrada invalida nao sobrescreve o snapshot anterior", () => {
  const dir = tempDir();
  const out = path.join(dir, "snapshot.json");
  const good = inputFile(dir);
  exportSnapshotFromFile(good, out);
  const previous = readFileSync(out, "utf8");
  const bad = inputFile(dir, { rows: [{ task_code: "T01" }] });
  assert.throws(() => exportSnapshotFromFile(bad, out));
  assert.equal(readFileSync(out, "utf8"), previous);
});

test("duas publicacoes do mesmo conteudo produzem o mesmo digest", () => {
  const dir = tempDir();
  const input = inputFile(dir);
  const first = exportSnapshotFromFile(input, path.join(dir, "a.json"), new Date("2026-11-17T14:20:05Z"));
  const second = exportSnapshotFromFile(input, path.join(dir, "b.json"), new Date("2026-11-17T14:20:05Z"));
  assert.equal(first.digest, second.digest);
});
