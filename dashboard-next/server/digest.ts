import { createHash } from "node:crypto";

/** JSON canonico do contrato: chaves ordenadas, sem espacos, inteiros. */
export function canonicalize(value: unknown): string {
  if (value === null) {
    return "null";
  }
  if (typeof value === "string") {
    return JSON.stringify(value);
  }
  if (typeof value === "boolean") {
    return value ? "true" : "false";
  }
  if (typeof value === "number") {
    if (!Number.isFinite(value)) {
      throw new TypeError("numero nao finito");
    }
    if (!Number.isInteger(value)) {
      throw new TypeError("contrato aceita apenas inteiros em campos numericos");
    }
    return String(value);
  }
  if (Array.isArray(value)) {
    return `[${value.map((item) => canonicalize(item)).join(",")}]`;
  }
  if (typeof value === "object") {
    const record = value as Record<string, unknown>;
    const keys = Object.keys(record).sort();
    return `{${keys.map((key) => `${JSON.stringify(key)}:${canonicalize(record[key])}`).join(",")}}`;
  }
  throw new TypeError("tipo nao suportado");
}

export function sha256Hex(text: string | Uint8Array): string {
  return createHash("sha256").update(text).digest("hex");
}

export function computeSnapshotDigest(snapshot: Record<string, unknown>): string {
  const withoutDigest: Record<string, unknown> = Object.create(null) as Record<string, unknown>;
  for (const key of Object.keys(snapshot)) {
    if (key !== "digest") {
      withoutDigest[key] = snapshot[key];
    }
  }
  return `sha256:${sha256Hex(canonicalize(withoutDigest))}`;
}
