export const statuses = ["pending", "in_progress", "in_review", "blocked", "done", "excluded", "unknown"] as const;
export type Status = typeof statuses[number];
export const statusLabels: Record<Status, string> = { pending: "Pendente", in_progress: "Em andamento", in_review: "Em revisão", blocked: "Bloqueado", done: "Concluído", excluded: "Fora do fluxo", unknown: "Triagem" };
export type { Task } from "../shared/api-types";
import type { Task, SnapshotResponse as Snapshot } from "../shared/api-types";
export type { SnapshotResponse as Snapshot } from "../shared/api-types";
export interface Filters { query: string; status: string; owner: string; phase: string; area: string }
export const emptyFilters: Filters = { query: "", status: "", owner: "", phase: "", area: "" };
export function filterTasks(tasks: Task[], filters: Filters): Task[] {
  const query = filters.query.trim().toLocaleLowerCase("pt-BR");
  return tasks.filter(task => (!filters.status || task.status_normalized === filters.status)
    && (!filters.owner || task.owner_code === filters.owner)
    && (!filters.area || task.area === filters.area)
    && (!filters.phase || String(task.phase ?? "none") === filters.phase)
    && (!query || [task.task_code, task.title, task.area, task.owner_code].join(" ").toLocaleLowerCase("pt-BR").includes(query)));
}
export function sourceIsStale(snapshot: Pick<Snapshot, "stale" | "age_seconds" | "stale_after_seconds">, elapsedSeconds: number): boolean {
  return snapshot.stale || snapshot.age_seconds + elapsedSeconds > snapshot.stale_after_seconds;
}
export function safeExternalUrl(value: string | null): string | null {
  if (!value) return null;
  try { const url = new URL(value); return ["http:", "https:"].includes(url.protocol) ? url.href : null; }
  catch { return null; }
}
