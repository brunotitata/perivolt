import { readFileSync, statSync } from "node:fs";
import path from "node:path";
import {
  SnapshotValidationError,
  parseSnapshotText,
  validateSnapshotDocument,
  type ValidatedSnapshot
} from "./snapshot";
import { SNAPSHOT_LIMITS } from "./limits";

export type StoreKind = "ok" | "missing" | "invalid";

export interface SnapshotFileInfo {
  name: string;
  size: number | null;
  mtime: string | null;
}

export interface SnapshotState {
  kind: StoreKind;
  snapshot: ValidatedSnapshot | null;
  error: "snapshot_missing" | "snapshot_invalid";
  checkedAt: string;
  file: SnapshotFileInfo;
  /** Motivo interno, sem caminho absoluto nem conteudo; nao vai para a API. */
  reason: string | null;
}

export interface SnapshotStoreOptions {
  paths?: string[];
  now?: () => Date;
}

export function defaultSnapshotPaths(cwd = process.cwd()): string[] {
  const explicit = process.env.PERIVOLT_SNAPSHOT_PATH;
  if (explicit !== undefined && explicit.trim().length > 0) {
    return [path.resolve(cwd, explicit.trim())];
  }
  return [path.join(cwd, "data", "snapshot.json"), path.join(cwd, "data", "project.json")];
}

function iso(value: number): string {
  return new Date(value).toISOString();
}

function basename(filePath: string): string {
  return path.basename(filePath);
}

export class SnapshotStore {
  private readonly paths: string[];
  private readonly now: () => Date;
  private state: SnapshotState;
  private lastSignature: string | null = null;
  private inflight: Promise<SnapshotState> | null = null;

  constructor(options: SnapshotStoreOptions = {}) {
    this.paths = options.paths ?? defaultSnapshotPaths();
    this.now = options.now ?? (() => new Date());
    this.state = {
      kind: "missing",
      snapshot: null,
      error: "snapshot_missing",
      checkedAt: this.now().toISOString(),
      file: { name: basename(this.paths[0] ?? "snapshot.json"), size: null, mtime: null },
      reason: "no_candidate_file"
    };
  }

  get pathsForTests(): readonly string[] {
    return this.paths;
  }

  async get(): Promise<SnapshotState> {
    await this.refresh();
    return this.state;
  }

  async refresh(): Promise<SnapshotState> {
    if (this.inflight !== null) {
      return this.inflight;
    }
    this.inflight = this.load().finally(() => {
      this.inflight = null;
    });
    return this.inflight;
  }

  private async load(): Promise<SnapshotState> {
    const checkedAt = this.now().toISOString();
    let selected: string | null = null;
    let info: { size: number; mtimeMs: number } | null = null;
    for (const candidate of this.paths) {
      try {
        const stat = statSync(candidate);
        if (stat.isFile()) {
          selected = candidate;
          info = { size: stat.size, mtimeMs: stat.mtimeMs };
          break;
        }
      } catch {
        /* candidato ausente; tenta o proximo */
      }
    }
    if (selected === null || info === null) {
      this.lastSignature = "missing";
      this.state = {
        kind: "missing",
        snapshot: null,
        error: "snapshot_missing",
        checkedAt,
        file: { name: basename(this.paths[0] ?? "snapshot.json"), size: null, mtime: null },
        reason: "file_not_found"
      };
      return this.state;
    }
    const signature = `${selected}|${info.mtimeMs}|${info.size}`;
    if (signature === this.lastSignature && this.state.kind !== "missing") {
      this.state = { ...this.state, checkedAt };
      return this.state;
    }
    const file: SnapshotFileInfo = {
      name: basename(selected),
      size: info.size,
      mtime: iso(info.mtimeMs)
    };
    if (info.size > SNAPSHOT_LIMITS.maxBytes) {
      this.lastSignature = signature;
      this.state = {
        kind: "invalid",
        snapshot: null,
        error: "snapshot_invalid",
        checkedAt,
        file,
        reason: "file_too_large"
      };
      return this.state;
    }
    try {
      const raw = readFileSync(/* turbopackIgnore: true */ selected, "utf8").replace(/^\uFEFF/, "");
      const parsed = parseSnapshotText(raw);
      const snapshot = validateSnapshotDocument(parsed);
      this.lastSignature = signature;
      this.state = {
        kind: "ok",
        snapshot,
        error: "snapshot_invalid",
        checkedAt,
        file,
        reason: null
      };
      return this.state;
    } catch (error) {
      const reason = error instanceof SnapshotValidationError ? error.reason : "read_or_parse_failed";
      this.lastSignature = signature;
      this.state = {
        kind: "invalid",
        snapshot: null,
        error: "snapshot_invalid",
        checkedAt,
        file,
        reason
      };
      return this.state;
    }
  }
}

interface GlobalWithStore {
  __perivoltSnapshotStore?: SnapshotStore;
}

export function getSnapshotStore(): SnapshotStore {
  const globalRef = globalThis as GlobalWithStore;
  if (globalRef.__perivoltSnapshotStore === undefined) {
    globalRef.__perivoltSnapshotStore = new SnapshotStore();
  }
  return globalRef.__perivoltSnapshotStore;
}

/** Apenas para testes: descarta o singleton e permite outro caminho. */
export function resetSnapshotStoreForTests(): void {
  delete (globalThis as GlobalWithStore).__perivoltSnapshotStore;
}
