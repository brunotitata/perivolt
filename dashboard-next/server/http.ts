import type { ErrorResponse, HealthResponse, SnapshotResponse } from "../shared/api-types";
import { toSnapshotResponse } from "./snapshot";
import type { SnapshotStore } from "./snapshot-store";

export const NO_STORE_HEADERS = {
  "cache-control": "no-store, max-age=0"
} as const;

export interface HttpResult<T> {
  status: 200 | 503;
  body: T;
}

export async function snapshotHttp(
  store: SnapshotStore,
  now: () => Date = () => new Date()
): Promise<HttpResult<SnapshotResponse | ErrorResponse>> {
  const state = await store.get();
  const servedAt = now().toISOString();
  if (state.kind === "ok" && state.snapshot !== null) {
    return { status: 200, body: toSnapshotResponse(state.snapshot, servedAt) };
  }
  return {
    status: 503,
    body: {
      ok: false,
      error: state.kind === "missing" ? "snapshot_missing" : "snapshot_invalid",
      served_at: servedAt
    }
  };
}

export async function healthHttp(
  store: SnapshotStore,
  now: () => Date = () => new Date()
): Promise<HttpResult<HealthResponse | ErrorResponse>> {
  const state = await store.get();
  const servedAt = now().toISOString();
  if (state.kind === "ok" && state.snapshot !== null) {
    const response = toSnapshotResponse(state.snapshot, servedAt);
    return {
      status: 200,
      body: {
        ok: true,
        snapshot: {
          version: response.version,
          observed_at: response.observed_at,
          age_seconds: response.age_seconds,
          stale: response.stale
        },
        source: "file"
      }
    };
  }
  return {
    status: 503,
    body: {
      ok: false,
      error: state.kind === "missing" ? "snapshot_missing" : "snapshot_invalid",
      served_at: servedAt
    }
  };
}
