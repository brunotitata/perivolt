import { sanitizeEvidenceText } from "./evidence-text";
export { sanitizeEvidenceText } from "./evidence-text";
import { mkdirSync, readFileSync, renameSync, statSync, unlinkSync, writeFileSync } from "node:fs";
import path from "node:path";
import type { Evidence, NormalizedStatus, OwnerMapEntry, StatusMap, Task } from "../shared/api-types";
import { computeSnapshotDigest } from "./digest";
import { JsonStrictError, parseJsonStrict } from "./json-strict";
import { PROJECT_DEFAULTS, SNAPSHOT_LIMITS, STATUS_MAP_DEFAULT } from "./limits";
import { validateSnapshotDocument } from "./snapshot";

export interface ExportRow {
  id?: string;
  task_code: string;
  area: string;
  phase: number | null;
  priority: string;
  title: string;
  owner: string;
  depends_on: string;
  requirements: string;
  acceptance: string;
  status: string;
  evidence: string | null;
}

export interface ExportInput {
  exported_by: string;
  observed_at: string;
  generated_at?: string;
  coverage?: "complete" | "partial";
  stale_after_seconds?: number;
  owner_map?: Record<string, OwnerMapEntry | string>;
  status_map?: StatusMap;
  formats?: Record<string, string>;
  evidence?: Evidence[];
  rows: ExportRow[];
}

export interface ExportSummary {
  outPath: string;
  bytes: number;
  digest: string;
  tasks: number;
  counts: Record<string, number>;
  observedAt: string;
}

const ISO_UTC = /^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d{1,9})?Z$/;
const NORMALIZED = new Set(["pending", "in_progress", "in_review", "blocked", "done", "excluded", "unknown"]);

function fail(message: string): never {
  throw new Error(message);
}

function record(value: unknown, field: string): Record<string, unknown> {
  if (value === null || typeof value !== "object" || Array.isArray(value)) {
    fail(`${field} deve ser objeto`);
  }
  return value as Record<string, unknown>;
}

function string(value: unknown, field: string, max = 100_000): string {
  if (typeof value !== "string") {
    fail(`${field} deve ser texto`);
  }
  if (value.length > max) {
    fail(`${field} excede o limite`);
  }
  return value;
}

function nullableString(value: unknown, field: string, max = 4000): string | null {
  if (value === null || value === undefined) {
    return null;
  }
  return string(value, field, max);
}

function iso(value: unknown, field: string): string {
  const parsed = string(value, field, 40);
  if (!ISO_UTC.test(parsed) || Number.isNaN(Date.parse(parsed))) {
    fail(`${field} deve ser ISO 8601 UTC`);
  }
  return parsed;
}


function normalizeOwnerMap(value: unknown, owners: Set<string>): Record<string, OwnerMapEntry> {
  const result: Record<string, OwnerMapEntry> = {};
  if (value !== undefined && value !== null) {
    const raw = record(value, "owner_map");
    for (const code of Object.keys(raw)) {
      const entry = raw[code];
      if (typeof entry === "string") {
        result[code] = { name: code, agent_id: entry };
        continue;
      }
      const parsed = record(entry, `owner_map.${code}`);
      const name = string(parsed["name"], `owner_map.${code}.name`, 200);
      const agentId = parsed["agent_id"];
      result[code] =
        agentId === undefined
          ? { name }
          : typeof agentId === "string"
            ? { name, agent_id: agentId }
            : fail(`owner_map.${code}.agent_id invalido`);
    }
  }
  for (const owner of owners) {
    if (result[owner] === undefined) {
      result[owner] = { name: owner };
    }
  }
  return result;
}

function normalizeStatusMap(value: unknown): StatusMap {
  if (value === undefined || value === null) {
    return {
      version: STATUS_MAP_DEFAULT.version,
      entries: { ...STATUS_MAP_DEFAULT.entries } as Record<string, NormalizedStatus>
    };
  }
  const raw = record(value, "status_map");
  const entriesRaw = record(raw["entries"], "status_map.entries");
  const entries: Record<string, NormalizedStatus> = {};
  for (const key of Object.keys(entriesRaw)) {
    const entry = entriesRaw[key];
    if (typeof entry !== "string" || !NORMALIZED.has(entry)) {
      fail("status_map contem valor normalizado invalido");
    }
    entries[key] = entry as NormalizedStatus;
  }
  return { version: string(raw["version"], "status_map.version", 64), entries };
}

function parseRows(value: unknown, statusMap: StatusMap): { tasks: Task[]; owners: Set<string> } {
  if (!Array.isArray(value)) {
    fail("rows deve ser array");
  }
  if (value.length > SNAPSHOT_LIMITS.maxTasks) {
    fail("rows excede o limite");
  }
  const codes = new Set<string>();
  const owners = new Set<string>();
  const tasks: Task[] = value.map((item, index) => {
    const row = record(item, `rows[${index}]`);
    const code = string(row["task_code"], `rows[${index}].task_code`, 64);
    if (codes.has(code)) {
      fail("task_code duplicado");
    }
    codes.add(code);
    const statusRaw = string(row["status"], `rows[${index}].status`, 80);
    const phaseRaw = row["phase"];
    const phase =
      phaseRaw === null
        ? null
        : typeof phaseRaw === "number" && Number.isInteger(phaseRaw) && phaseRaw >= 1 && phaseRaw <= 7
          ? phaseRaw
          : fail(`rows[${index}].phase invalido`);
    const owner = string(row["owner"], `rows[${index}].owner`, 40);
    owners.add(owner);
    const depends = string(row["depends_on"] ?? "nenhuma", `rows[${index}].depends_on`, 2000);
    const requirements = string(row["requirements"] ?? "nenhuma", `rows[${index}].requirements`, 2000);
    return {
      task_code: code,
      title: string(row["title"], `rows[${index}].title`),
      area: string(row["area"], `rows[${index}].area`, 80),
      phase,
      priority: string(row["priority"], `rows[${index}].priority`, 40),
      status_raw: statusRaw,
      status_normalized: statusMap.entries[statusRaw] ?? "unknown",
      owner_code: owner,
      depends_on_text: depends.trim() === "nenhuma" ? null : depends,
      requirements_text: requirements.trim() === "nenhuma" ? null : requirements,
      acceptance: nullableString(row["acceptance"], `rows[${index}].acceptance`, 20_000),
      evidence_text: sanitizeEvidenceText(nullableString(row["evidence"], `rows[${index}].evidence`, 4000)),
      external_refs: []
    } satisfies Task;
  });
  tasks.sort((left, right) => left.task_code.localeCompare(right.task_code, "pt-BR", { numeric: true }));
  return { tasks, owners };
}

function parseEvidence(value: unknown, taskCodes: Set<string>): Evidence[] {
  if (value === undefined || value === null) {
    return [];
  }
  if (!Array.isArray(value)) {
    fail("evidence deve ser array");
  }
  if (value.length > SNAPSHOT_LIMITS.maxEvidence) {
    fail("evidence excede o limite");
  }
  return value.map((item, index) => {
    const entry = record(item, `evidence[${index}]`);
    const taskCode = string(entry["task_code"], `evidence[${index}].task_code`, 64);
    if (!taskCodes.has(taskCode)) {
      fail("evidence aponta para tarefa inexistente");
    }
    const state = entry["state"];
    if (state !== "located" && state !== "missing" && state !== "unreadable") {
      fail("evidence.state invalido");
    }
    const locator = string(entry["locator"], `evidence[${index}].locator`, 400);
    if (locator.includes("\u0000") || /^([A-Za-z]:|\\\\|\/)/.test(locator) || locator.split(/[\\/]/).includes("..")) {
      fail("evidence.locator inseguro");
    }
    const sha = nullableString(entry["sha256"], `evidence[${index}].sha256`, 80);
    if (sha !== null && !/^[a-f0-9]{64}$/.test(sha)) {
      fail("evidence.sha256 invalido");
    }
    return {
      id: string(entry["id"], `evidence[${index}].id`, 120),
      task_code: taskCode,
      locator,
      kind: string(entry["kind"], `evidence[${index}].kind`, 40),
      sha256: sha,
      observed_at: iso(entry["observed_at"], `evidence[${index}].observed_at`),
      state
    };
  });
}

export function buildSnapshotDocument(input: unknown, now: Date = new Date()): Record<string, unknown> {
  const root = record(input, "entrada");
  const exportedBy = string(root["exported_by"] ?? "po-agent", "exported_by", 200);
  const observedAt = iso(root["observed_at"], "observed_at");
  const generatedAt = root["generated_at"] === undefined ? now.toISOString() : iso(root["generated_at"], "generated_at");
  const coverage = root["coverage"] ?? "complete";
  if (coverage !== "complete" && coverage !== "partial") {
    fail("coverage invalido");
  }
  const staleAfter = root["stale_after_seconds"] ?? PROJECT_DEFAULTS.stale_after_seconds;
  if (typeof staleAfter !== "number" || !Number.isInteger(staleAfter) || staleAfter < 0) {
    fail("stale_after_seconds invalido");
  }
  const statusMap = normalizeStatusMap(root["status_map"]);
  const parsedRows = parseRows(root["rows"], statusMap);
  const taskCodes = new Set(parsedRows.tasks.map((task) => task.task_code));
  const evidence = parseEvidence(root["evidence"], taskCodes);
  const ownerMap = normalizeOwnerMap(root["owner_map"], parsedRows.owners);
  const formatsRaw = root["formats"];
  const formats = formatsRaw === undefined || formatsRaw === null ? undefined : record(formatsRaw, "formats");

  const document: Record<string, unknown> = {
    schema: "dashboard-snapshot@1",
    generated_at: generatedAt,
    project: { id: PROJECT_DEFAULTS.id, name: PROJECT_DEFAULTS.name },
    sources: [
      {
        id: PROJECT_DEFAULTS.source_id,
        kind: PROJECT_DEFAULTS.source_kind,
        observed_at: observedAt,
        coverage,
        stale_after_seconds: staleAfter
      }
    ],
    owner_map: ownerMap,
    ...(formats === undefined ? {} : { formats }),
    status_map: statusMap,
    tasks: parsedRows.tasks,
    evidence
  };
  document["digest"] = computeSnapshotDigest(document);
  validateSnapshotDocument(document);
  return document;
}

export function writeSnapshotAtomic(document: Record<string, unknown>, outPath: string): number {
  const directory = path.dirname(outPath);
  mkdirSync(directory, { recursive: true });
  const text = `${JSON.stringify(document, null, 2)}\n`;
  const tempPath = path.join(directory, `.${path.basename(outPath)}.tmp-${process.pid}-${Date.now()}`);
  writeFileSync(tempPath, text, "utf8");
  try {
    renameSync(tempPath, outPath);
  } catch (error) {
    try {
      unlinkSync(tempPath);
    } catch {
      /* temporario ja removido */
    }
    throw error;
  }
  return Buffer.byteLength(text, "utf8");
}

export function exportSnapshotFromFile(inputPath: string, outPath: string, now: Date = new Date()): ExportSummary {
  const stat = statSync(inputPath);
  if (stat.size > SNAPSHOT_LIMITS.maxBytes) {
    fail("entrada acima de 20 MB");
  }
  const raw = readFileSync(inputPath, "utf8").replace(/^\uFEFF/, "");
  let parsed: unknown;
  try {
    parsed = parseJsonStrict(raw, SNAPSHOT_LIMITS.json).value;
  } catch (error) {
    if (error instanceof JsonStrictError) {
      fail(`entrada JSON invalida (${error.code})`);
    }
    throw error;
  }
  const document = buildSnapshotDocument(parsed, now);
  const bytes = writeSnapshotAtomic(document, outPath);
  const validated = validateSnapshotDocument(document);
  const counts: Record<string, number> = {};
  for (const task of validated.tasks) {
    counts[task.status_normalized] = (counts[task.status_normalized] ?? 0) + 1;
  }
  return {
    outPath,
    bytes,
    digest: validated.digest,
    tasks: validated.tasks.length,
    counts,
    observedAt: validated.observedAt
  };
}
