export const SNAPSHOT_LIMITS = {
  maxBytes: 20 * 1024 * 1024,
  maxTasks: 5000,
  maxEvidence: 10000,
  validationMs: 5000,
  json: {
    maxDepth: 32,
    maxValues: 200_000,
    maxStringLength: 100_000
  }
} as const;

export const PROJECT_DEFAULTS = {
  id: "peripheral-battery-monitor",
  name: "PeriVolt",
  source_id: "openbot.peripheral_battery_tasks",
  source_kind: "openbot_table",
  stale_after_seconds: 86_400
} as const;

export const STATUS_MAP_DEFAULT = {
  version: "2026-11-17.1",
  entries: {
    PLANEJADO: "pending",
    EM_ANDAMENTO: "in_progress",
    EM_REVISAO: "in_review",
    BLOQUEADO: "blocked",
    CONCLUIDO: "done",
    CANCELADO: "excluded",
    NAO_PLANEJADO: "excluded"
  }
} as const;
