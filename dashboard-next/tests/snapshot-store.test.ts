import assert from "node:assert/strict";
import { writeFileSync } from "node:fs";
import path from "node:path";
import test from "node:test";
import { SnapshotStore } from "../server/snapshot-store";
import { writeSnapshotAtomic } from "../server/export";
import { sampleDocument, tempDir, writeJson } from "./helpers";

const NOW = () => new Date("2026-11-17T14:10:00Z");

test("arquivo ausente vira snapshot_missing, nunca snapshot vazio", async () => {
  const dir = tempDir();
  const store = new SnapshotStore({ paths: [path.join(dir, "nao-existe.json")], now: NOW });
  const state = await store.get();
  assert.equal(state.kind, "missing");
  assert.equal(state.error, "snapshot_missing");
  assert.equal(state.snapshot, null);
});

test("snapshot valido e servido com versao igual ao digest", async () => {
  const dir = tempDir();
  const file = path.join(dir, "snapshot.json");
  const document = sampleDocument();
  writeJson(file, document);
  const store = new SnapshotStore({ paths: [file], now: NOW });
  const state = await store.get();
  assert.equal(state.kind, "ok");
  assert.equal(state.snapshot?.version, document["digest"]);
  assert.equal(state.snapshot?.tasks.length, 2);
  assert.equal(state.snapshot?.tasks[0]?.owner_code, "DEV");
  assert.equal(state.file.name, "snapshot.json");
});

test("troca atomica do arquivo muda a versao no seguinte refresh", async () => {
  const dir = tempDir();
  const file = path.join(dir, "snapshot.json");
  writeJson(file, sampleDocument());
  const store = new SnapshotStore({ paths: [file], now: NOW });
  const first = await store.get();
  assert.equal(first.kind, "ok");
  const changed = sampleDocument();
  const tasks = changed["tasks"] as Array<Record<string, unknown>>;
  tasks[0]!["title"] = "Titulo alterado de forma real e mais longo";
  const digestModule = await import("../server/digest");
  changed["digest"] = digestModule.computeSnapshotDigest(changed);
  const bytes = writeSnapshotAtomic(changed, file);
  assert.ok(bytes > 0);
  const second = await store.refresh();
  assert.equal(second.kind, "ok");
  assert.notEqual(second.snapshot?.version, first.snapshot?.version);
  assert.equal(second.snapshot?.tasks[0]?.title, "Titulo alterado de forma real e mais longo");
});

test("digest alterado sem recálculo vira snapshot_invalid", async () => {
  const dir = tempDir();
  const file = path.join(dir, "snapshot.json");
  const document = sampleDocument();
  const tasks = document["tasks"] as Array<Record<string, unknown>>;
  tasks[0]!["title"] = "Mudou sem atualizar o digest";
  writeJson(file, document);
  const store = new SnapshotStore({ paths: [file], now: NOW });
  const state = await store.get();
  assert.equal(state.kind, "invalid");
  assert.equal(state.error, "snapshot_invalid");
  assert.equal(state.snapshot, null);
});

test("chave duplicada no JSON e recusada", async () => {
  const dir = tempDir();
  const file = path.join(dir, "snapshot.json");
  writeFileSync(file, '{"schema":"dashboard-snapshot@1","schema":"dashboard-snapshot@1"}', "utf8");
  const store = new SnapshotStore({ paths: [file], now: NOW });
  const state = await store.get();
  assert.equal(state.kind, "invalid");
  assert.equal(state.reason, "json_invalid");
});

test("aceita project.json como caminho alternativo da ponte", async () => {
  const dir = tempDir();
  const file = path.join(dir, "project.json");
  writeJson(file, sampleDocument());
  const store = new SnapshotStore({ paths: [path.join(dir, "snapshot.json"), file], now: NOW });
  const state = await store.get();
  assert.equal(state.kind, "ok");
  assert.equal(state.file.name, "project.json");
});
