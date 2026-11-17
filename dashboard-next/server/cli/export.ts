import path from "node:path";
import { exportSnapshotFromFile } from "../export";

function parseArgs(argv: string[]): { input: string; out: string } {
  let input = "";
  let out = path.join(process.cwd(), "data", "snapshot.json");
  for (let index = 0; index < argv.length; index += 1) {
    const key = argv[index];
    const value = argv[index + 1];
    if (key === "--input") {
      if (value === undefined) {
        throw new Error("--input sem valor");
      }
      input = path.resolve(process.cwd(), value);
      index += 1;
    } else if (key === "--out") {
      if (value === undefined) {
        throw new Error("--out sem valor");
      }
      out = path.resolve(process.cwd(), value);
      index += 1;
    } else {
      throw new Error(`argumento desconhecido: ${key ?? ""}`);
    }
  }
  if (input.length === 0) {
    throw new Error("uso: export.ts --input <linhas.json> [--out data/snapshot.json]");
  }
  return { input, out };
}

try {
  const args = parseArgs(process.argv.slice(2));
  const summary = exportSnapshotFromFile(args.input, args.out);
  process.stdout.write(`${JSON.stringify({ ok: true, ...summary }, null, 2)}\n`);
} catch (error) {
  const message = error instanceof Error ? error.message : "falha ao exportar";
  process.stdout.write(
    `${JSON.stringify({ ok: false, error: { code: "export_failed", message } }, null, 2)}\n`
  );
  process.exitCode = 1;
}
