import { mkdtempSync, writeFileSync } from "node:fs";
import os from "node:os";
import path from "node:path";
import { buildSnapshotDocument } from "../server/export";

export function tempDir(prefix = "perivolt-next-"): string {
  return mkdtempSync(path.join(os.tmpdir(), prefix));
}

export function sampleRows() {
  return [
    {
      id: "PBM-T01",
      task_code: "T01",
      area: "dashboard",
      phase: 2,
      priority: "P0",
      title: "Definir contrato",
      owner: "DEV",
      depends_on: "nenhuma",
      requirements: "D01",
      acceptance: "Aceite.",
      status: "EM_REVISAO",
      evidence: "dashboard/data/contract-v1.md"
    },
    {
      id: "PBM-T02",
      task_code: "T02",
      area: "qualidade",
      phase: 5,
      priority: "P0",
      title: "Revisar contrato",
      owner: "QA",
      depends_on: "T01",
      requirements: "nenhuma",
      acceptance: "Aceite.",
      status: "PLANEJADO",
      evidence: null
    }
  ];
}

export function sampleDocument(overrides: Record<string, unknown> = {}): Record<string, unknown> {
  return buildSnapshotDocument(
    {
      exported_by: "agent-test",
      observed_at: "2026-11-17T14:00:00Z",
      generated_at: "2026-11-17T14:00:01Z",
      owner_map: {
        DEV: { name: "Desenvolvimento & Dashboard", agent_id: "agent-dev" },
        QA: { name: "Qualidade & Releases", agent_id: "agent-qa" }
      },
      rows: sampleRows(),
      ...overrides
    },
    new Date("2026-11-17T14:00:02Z")
  );
}

export function writeJson(filePath: string, value: unknown): void {
  writeFileSync(filePath, JSON.stringify(value, null, 2), "utf8");
}
