import { sanitizeEvidenceText } from "./evidence-text";
import type {
  Evidence,
  NormalizedStatus,
  OwnerMapEntry,
  SnapshotResponse,
  StatusMap,
  Task
} from "../shared/api-types";
import { computeSnapshotDigest } from "./digest";
import { parseJsonStrict } from "./json-strict";
import { PROJECT_DEFAULTS, SNAPSHOT_LIMITS } from "./limits";

export class SnapshotValidationError extends Error {
  readonly reason: string;

  constructor(reason: string) {
    super("snapshot invalido");
    this.name = "SnapshotValidationError";
    this.reason = reason;
  }
}

export interface ValidatedSnapshot {
  schema: "dashboard-snapshot@1";
  version: string;
  generatedAt: string;
  observedAt: string;
  coverage: "complete" | "partial";
  staleAfterSeconds: number;
  project: { id: string; name: string };
  source: { id: string; kind: string };
  ownerMap: Record<string, OwnerMapEntry>;
  statusMap: StatusMap;
  tasks: Task[];
  evidence: Evidence[];
  digest: string;
}

const ISO_UTC = /^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d{1,9})?Z$/;
const NORMALIZED = new Set([
  "pending",
  "in_progress",
  "in_review",
  "blocked",
  "done",
  "excluded",
  "unknown"
]);

function fail(reason: string): never {
  throw new SnapshotValidationError(reason);
}

function record(value: unknown, field: string): Record<string, unknown> {
  if (value === null || typeof value !== "object" || Array.isArray(value)) {
    fail(`${field}_not_object`);
  }
  return value as Record<string, unknown>;
}

function array(value: unknown, field: string, max: number): unknown[] {
  if (!Array.isArray(value)) {
    fail(`${field}_not_array`);
  }
  if (value.length > max) {
    fail(`${field}_too_large`);
  }
  return value;
}

function text(value: unknown, field: string, max = 100_000): string {
  if (typeof value !== "string") {
    fail(`${field}_not_string`);
  }
  if (value.length > max) {
    fail(`${field}_too_long`);
  }
  return value;
}

function optionalText(value: unknown, field: string, max = 4000): string | null {
  if (value === null || value === undefined) {
    return null;
  }
  return text(value, field, max);
}

function isoUtc(value: unknown, field: string): string {
  const parsed = text(value, field, 40);
  if (!ISO_UTC.test(parsed) || Number.isNaN(Date.parse(parsed))) {
    fail(`${field}_not_iso_utc`);
  }
  return parsed;
}

function parseOwnerMap(value: unknown): Record<string, OwnerMapEntry> {
  if (value === undefined || value === null) {
    return {};
  }
  const raw = record(value, "owner_map");
  const result: Record<string, OwnerMapEntry> = {};
  for (const code of Object.keys(raw)) {
    if (code.length === 0 || code.length > 40) {
      fail("owner_code_invalid");
    }
    const entry = raw[code];
    if (typeof entry === "string") {
      result[code] = { name: code, agent_id: entry };
      continue;
    }
    const parsed = record(entry, "owner_entry");
    const name = parsed["name"];
    const agentId = parsed["agent_id"];
    if (typeof name !== "string" || name.length === 0) {
      fail("owner_name_invalid");
    }
    result[code] =
      agentId === undefined
        ? { name }
        : typeof agentId === "string"
          ? { name, agent_id: agentId }
          : fail("owner_agent_invalid");
  }
  return result;
}

function parseStatusMap(value: unknown): StatusMap {
  const raw = record(value, "status_map");
  const entriesRaw = record(raw["entries"], "status_entries");
  const entries: Record<string, NormalizedStatus> = {};
  for (const key of Object.keys(entriesRaw)) {
    const normalized = entriesRaw[key];
    if (typeof normalized !== "string" || !NORMALIZED.has(normalized)) {
      fail("status_map_invalid");
    }
    entries[key] = normalized as NormalizedStatus;
  }
  return { version: text(raw["version"], "status_version", 64), entries };
}

export function parseSnapshotText(raw: string): unknown {
  try {
    return parseJsonStrict(raw, SNAPSHOT_LIMITS.json).value;
  } catch {
    throw new SnapshotValidationError("json_invalid");
  }
}

export function validateSnapshotDocument(value: unknown): ValidatedSnapshot {
  const root = record(value, "root");
  if (root["schema"] !== "dashboard-snapshot@1") {
    fail("schema_unsupported");
  }
  const projectRaw = record(root["project"], "project");
  const projectId = text(projectRaw["id"], "project_id", 120);
  if (projectId !== PROJECT_DEFAULTS.id) {
    fail("project_id_unexpected");
  }
  const projectName = text(projectRaw["name"], "project_name", 200);
  if (projectName !== PROJECT_DEFAULTS.name) {
    fail("project_name_unexpected");
  }
  const sourcesRaw = array(root["sources"], "sources", 50);
  if (sourcesRaw.length === 0) {
    fail("sources_empty");
  }
  const sources = sourcesRaw.map((item, index) => {
    const source = record(item, `source_${index}`);
    const coverage = source["coverage"];
    if (coverage !== "complete" && coverage !== "partial") {
      fail("coverage_invalid");
    }
    const staleAfter = source["stale_after_seconds"];
    if (typeof staleAfter !== "number" || !Number.isInteger(staleAfter) || staleAfter < 0) {
      fail("stale_after_invalid");
    }
    return {
      id: text(source["id"], "source_id", 120),
      kind: text(source["kind"], "source_kind", 60),
      observed_at: isoUtc(source["observed_at"], "source_observed_at"),
      coverage,
      stale_after_seconds: staleAfter
    };
  });
  const statusMap = parseStatusMap(root["status_map"]);
  const tasksRaw = array(root["tasks"], "tasks", SNAPSHOT_LIMITS.maxTasks);
  const codes = new Set<string>();
  const tasks: Task[] = tasksRaw.map((item, index) => {
    const task = record(item, `task_${index}`);
    const code = text(task["task_code"], "task_code", 64);
    if (codes.has(code)) {
      fail("task_duplicate");
    }
    codes.add(code);
    const rawStatus = text(task["status_raw"], "status_raw", 80);
    const normalized = task["status_normalized"];
    if (typeof normalized !== "string" || !NORMALIZED.has(normalized)) {
      fail("status_normalized_invalid");
    }
    const expected = statusMap.entries[rawStatus] ?? "unknown";
    if (normalized !== expected) {
      fail("status_map_mismatch");
    }
    const phaseRaw = task["phase"];
    const phase =
      phaseRaw === null
        ? null
        : typeof phaseRaw === "number" && Number.isInteger(phaseRaw) && phaseRaw >= 1 && phaseRaw <= 7
          ? phaseRaw
          : fail("phase_invalid");
    const refsRaw = task["external_refs"];
    const refs = refsRaw === undefined || refsRaw === null ? [] : array(refsRaw, "external_refs", 50);
    const external_refs = refs.map((ref) => {
      const entry = record(ref, "external_ref");
      const url = optionalText(entry["url"], "external_url", 2048);
      if (url !== null && !/^https?:\/\//i.test(url)) {
        fail("external_url_scheme");
      }
      return {
        source: text(entry["source"], "external_source", 80),
        type: text(entry["type"], "external_type", 80),
        id: text(entry["id"], "external_id", 200),
        url
      };
    });
    return {
      task_code: code,
      title: text(task["title"], "title"),
      area: text(task["area"], "area", 80),
      phase,
      priority: text(task["priority"], "priority", 40),
      status_raw: rawStatus,
      status_normalized: normalized as NormalizedStatus,
      owner_code: text(task["owner_code"], "owner_code", 40),
      depends_on_text: optionalText(task["depends_on_text"], "depends_on", 2000),
      requirements_text: optionalText(task["requirements_text"], "requirements", 2000),
      acceptance: optionalText(task["acceptance"], "acceptance", 20_000),
      evidence_text: sanitizeEvidenceText(optionalText(task["evidence_text"], "evidence_text", 4000)),
      external_refs
    };
  });
  const evidenceRaw = root["evidence"];
  const evidenceList = evidenceRaw === undefined || evidenceRaw === null ? [] : array(evidenceRaw, "evidence", SNAPSHOT_LIMITS.maxEvidence);
  const evidence: Evidence[] = evidenceList.map((item) => {
    const entry = record(item, "evidence_entry");
    const taskCode = text(entry["task_code"], "evidence_task", 64);
    if (!codes.has(taskCode)) {
      fail("evidence_task_unknown");
    }
    const state = entry["state"];
    if (state !== "located" && state !== "missing" && state !== "unreadable") {
      fail("evidence_state_invalid");
    }
    const locator = text(entry["locator"], "evidence_locator", 400);
    if (locator.includes("\u0000") || /^([A-Za-z]:|\\\\|\/)/.test(locator) || locator.split(/[\\/]/).includes("..")) {
      fail("evidence_locator_unsafe");
    }
    const sha = optionalText(entry["sha256"], "evidence_sha", 80);
    if (sha !== null && !/^[a-f0-9]{64}$/.test(sha)) {
      fail("evidence_sha_invalid");
    }
    return {
      id: text(entry["id"], "evidence_id", 120),
      task_code: taskCode,
      locator,
      kind: text(entry["kind"], "evidence_kind", 40),
      sha256: sha,
      observed_at: isoUtc(entry["observed_at"], "evidence_observed_at"),
      state
    };
  });
  const digest = text(root["digest"], "digest", 80);
  if (!/^sha256:[a-f0-9]{64}$/.test(digest)) {
    fail("digest_format");
  }
  const computed = computeSnapshotDigest(root);
  if (computed !== digest) {
    fail("digest_mismatch");
  }
  const generatedAt = isoUtc(root["generated_at"], "generated_at");
  const observedAt = sources[0]?.observed_at ?? generatedAt;
  const staleAfterSeconds = Math.min(
    ...sources.map((source) => source.stale_after_seconds).filter((value) => value > 0)
  );
  return {
    schema: "dashboard-snapshot@1",
    version: digest,
    generatedAt,
    observedAt,
    coverage: sources.some((source) => source.coverage === "partial") ? "partial" : "complete",
    staleAfterSeconds: Number.isFinite(staleAfterSeconds) ? staleAfterSeconds : PROJECT_DEFAULTS.stale_after_seconds,
    project: { id: projectId, name: projectName },
    source: { id: sources[0]!.id, kind: sources[0]!.kind },
    ownerMap: parseOwnerMap(root["owner_map"]),
    statusMap,
    tasks,
    evidence,
    digest
  };
}

export function toSnapshotResponse(snapshot: ValidatedSnapshot, servedAt: string): SnapshotResponse {
  const observedMs = Date.parse(snapshot.observedAt);
  const servedMs = Date.parse(servedAt);
  const ageSeconds = Number.isNaN(observedMs) || Number.isNaN(servedMs) ? 0 : Math.max(0, Math.floor((servedMs - observedMs) / 1000));
  return {
    ok: true,
    schema: "dashboard-snapshot@1",
    version: snapshot.version,
    generated_at: snapshot.generatedAt,
    observed_at: snapshot.observedAt,
    served_at: servedAt,
    age_seconds: ageSeconds,
    stale: ageSeconds > snapshot.staleAfterSeconds,
    coverage: snapshot.coverage,
    stale_after_seconds: snapshot.staleAfterSeconds,
    project: { id: "peripheral-battery-monitor", name: "PeriVolt" },
    source: { id: snapshot.source.id, kind: "openbot_table" },
    owner_map: snapshot.ownerMap,
    status_map: snapshot.statusMap,
    tasks: snapshot.tasks,
    evidence: snapshot.evidence,
    digest: snapshot.digest
  };
}
