import assert from "node:assert/strict";
import path from "node:path";
import test from "node:test";
import { healthHttp, snapshotHttp } from "../server/http";
import { SnapshotStore } from "../server/snapshot-store";
import { sampleDocument, tempDir, writeJson } from "./helpers";

function storeWith(document: Record<string, unknown> | null, nowIso: string): SnapshotStore {
  const dir = tempDir();
  const file = path.join(dir, "snapshot.json");
  if (document !== null) {
    writeJson(file, document);
  }
  return new SnapshotStore({ paths: [file], now: () => new Date(nowIso) });
}

test("GET /api/snapshot devolve 200 com contrato SnapshotResponse", async () => {
  const store = storeWith(sampleDocument(), "2026-11-17T14:10:00Z");
  const result = await snapshotHttp(store, () => new Date("2026-11-17T14:10:00Z"));
  assert.equal(result.status, 200);
  const body = result.body as { ok: boolean; schema: string; project: { id: string; name: string } };
  assert.equal(body.ok, true);
  assert.equal(body.schema, "dashboard-snapshot@1");
  assert.equal(body.project.id, "peripheral-battery-monitor");
  assert.equal(body.project.name, "PeriVolt");
});

test("corpo 200 tem idade, stale, cobertura e tarefas do snapshot", async () => {
  const document = sampleDocument();
  const store = storeWith(document, "2026-11-17T14:10:00Z");
  const result = await snapshotHttp(store, () => new Date("2026-11-17T15:00:00Z"));
  const body = result.body as {
    project: { id: string; name: string };
    age_seconds: number;
    stale: boolean;
    coverage: string;
    tasks: Array<{ task_code: string }>;
    version: string;
    digest: string;
  };
  assert.equal(body.project.id, "peripheral-battery-monitor");
  assert.equal(body.project.name, "PeriVolt");
  assert.equal(body.age_seconds, 3600);
  assert.equal(body.stale, false);
  assert.equal(body.coverage, "complete");
  assert.equal(body.tasks.length, 2);
  assert.equal(body.version, body.digest);
});

test("observacao acima do TTL e marcada stale sem inventar zero", async () => {
  const store = storeWith(sampleDocument(), "2026-11-17T14:10:00Z");
  const result = await snapshotHttp(store, () => new Date("2026-11-19T14:10:00Z"));
  const body = result.body as { stale: boolean; age_seconds: number };
  assert.equal(body.stale, true);
  assert.equal(body.age_seconds, 173400);
});

test("ausente devolve 503 snapshot_missing", async () => {
  const store = storeWith(null, "2026-11-17T14:10:00Z");
  const result = await snapshotHttp(store, () => new Date("2026-11-17T14:10:00Z"));
  assert.equal(result.status, 503);
  assert.deepEqual(result.body, {
    ok: false,
    error: "snapshot_missing",
    served_at: "2026-11-17T14:10:00.000Z"
  });
});

test("invalido devolve 503 snapshot_invalid", async () => {
  const document = sampleDocument();
  const tasks = document["tasks"] as Array<Record<string, unknown>>;
  tasks[0]!["title"] = "quebrou o digest";
  const store = storeWith(document, "2026-11-17T14:10:00Z");
  const result = await snapshotHttp(store, () => new Date("2026-11-17T14:10:00Z"));
  assert.equal(result.status, 503);
  assert.equal((result.body as { error: string }).error, "snapshot_invalid");
});

test("GET /api/health devolve versao, observed_at, idade e stale", async () => {
  const store = storeWith(sampleDocument(), "2026-11-17T14:10:00Z");
  const result = await healthHttp(store, () => new Date("2026-11-17T14:10:00Z"));
  assert.equal(result.status, 200);
  assert.deepEqual(result.body, {
    ok: true,
    snapshot: {
      version: (sampleDocument()["digest"] as string),
      observed_at: "2026-11-17T14:00:00Z",
      age_seconds: 600,
      stale: false
    },
    source: "file"
  });
});

test("health tambem devolve 503 quando nao ha snapshot valido", async () => {
  const store = storeWith(null, "2026-11-17T14:10:00Z");
  const result = await healthHttp(store, () => new Date("2026-11-17T14:10:00Z"));
  assert.equal(result.status, 503);
  assert.equal((result.body as { error: string }).error, "snapshot_missing");
});
