// Hook somente para execucao direta com Node (CLI/testes): resolve imports
// relativos sem extensao para .ts, como o bundler do Next faz. Nao altera PATH.
import { existsSync } from "node:fs";
import { registerHooks } from "node:module";
import { fileURLToPath } from "node:url";

registerHooks({
  resolve(specifier, context, nextResolve) {
    const relative = specifier.startsWith("./") || specifier.startsWith("../");
    const hasExtension = /\.[A-Za-z0-9]+$/.test(specifier);
    if (relative && !hasExtension && context.parentURL !== undefined) {
      const base = new URL(specifier, context.parentURL);
      for (const extension of [".ts", ".tsx", ".mts", ".js"]) {
        const candidate = new URL(base.href + extension);
        if (existsSync(fileURLToPath(candidate))) {
          return { url: candidate.href, shortCircuit: true };
        }
      }
    }
    return nextResolve(specifier, context);
  }
});
