// Contrato compartilhado dashboard-next. Somente tipos; sem runtime.
// Fonte: dashboard-snapshot@1 + envelope de leitura. Ver CONTRACT.md.

export const NORMALIZED_STATUSES = [
  "pending",
  "in_progress",
  "in_review",
  "blocked",
  "done",
  "excluded",
  "unknown"
] as const;

export type NormalizedStatus = (typeof NORMALIZED_STATUSES)[number];

export interface OwnerMapEntry {
  name: string;
  agent_id?: string;
}

export interface Source {
  id: string;
  kind: string;
  observed_at: string;
  coverage: "complete" | "partial";
  stale_after_seconds: number;
}

export interface StatusMap {
  version: string;
  entries: Record<string, NormalizedStatus>;
}

export interface ExternalRef {
  source: string;
  type: string;
  id: string;
  url: string | null;
}

export interface Evidence {
  id: string;
  task_code: string;
  locator: string;
  kind: string;
  sha256: string | null;
  observed_at: string;
  state: "located" | "missing" | "unreadable";
}

export interface Task {
  task_code: string;
  title: string;
  area: string;
  phase: number | null;
  priority: string;
  status_raw: string;
  status_normalized: NormalizedStatus;
  owner_code: string;
  depends_on_text: string | null;
  requirements_text: string | null;
  acceptance: string | null;
  evidence_text: string | null;
  external_refs: ExternalRef[];
}

export interface SnapshotResponse {
  ok: true;
  schema: "dashboard-snapshot@1";
  version: string;
  generated_at: string;
  observed_at: string;
  served_at: string;
  age_seconds: number;
  stale: boolean;
  coverage: "complete" | "partial";
  stale_after_seconds: number;
  project: { id: "peripheral-battery-monitor"; name: "PeriVolt" };
  source: { id: string; kind: "openbot_table" };
  owner_map: Record<string, OwnerMapEntry>;
  status_map: StatusMap;
  tasks: Task[];
  evidence: Evidence[];
  digest: string;
}

export interface HealthResponse {
  ok: true;
  snapshot: {
    version: string;
    observed_at: string;
    age_seconds: number;
    stale: boolean;
  };
  source: "file";
}

export interface ErrorResponse {
  ok: false;
  error: "snapshot_missing" | "snapshot_invalid";
  served_at: string;
}

export interface StreamSnapshotEvent {
  version: string;
  observed_at: string;
}
