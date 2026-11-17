/**
 * Parser JSON estrito do backend (porta do validador do dashboard legado).
 * Recusa chave duplicada, chave perigosa de merge, profundidade, volume e string longa.
 * Nao usa eval/Function. Saida com objetos sem prototipo.
 */

export interface JsonLimits {
  maxDepth: number;
  maxValues: number;
  maxStringLength: number;
}

export interface JsonStats {
  values: number;
  maxDepth: number;
}

export class JsonStrictError extends Error {
  readonly code: "syntax" | "duplicate_key" | "forbidden_key" | "depth" | "value_limit" | "string_limit";

  constructor(
    code: JsonStrictError["code"],
    message: string
  ) {
    super(message);
    this.name = "JsonStrictError";
    this.code = code;
  }
}

const FORBIDDEN_KEYS = new Set(["__proto__", "constructor", "prototype"]);
const NUMBER_TOKEN = /-?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)?/y;

export function parseJsonStrict(text: string, limits: JsonLimits): { value: unknown; stats: JsonStats } {
  let index = 0;
  let values = 0;
  let maxDepthSeen = 0;
  const length = text.length;

  const fail = (code: JsonStrictError["code"], message: string): never => {
    throw new JsonStrictError(code, message);
  };

  const skipWhitespace = (): void => {
    while (index < length) {
      const code = text.charCodeAt(index);
      if (code === 0x20 || code === 0x09 || code === 0x0a || code === 0x0d) {
        index += 1;
      } else {
        break;
      }
    }
  };

  const countValue = (depth: number): void => {
    values += 1;
    if (values > limits.maxValues) {
      fail("value_limit", "limite de valores JSON excedido");
    }
    if (depth > maxDepthSeen) {
      maxDepthSeen = depth;
    }
  };

  const parseString = (): string => {
    const start = index;
    index += 1;
    while (index < length) {
      const char = text[index];
      if (char === '"') {
        index += 1;
        const raw = text.slice(start, index);
        let decoded: unknown;
        try {
          decoded = JSON.parse(raw);
        } catch {
          return fail("syntax", "string JSON invalida");
        }
        if (typeof decoded !== "string") {
          return fail("syntax", "string JSON invalida");
        }
        if (decoded.length > limits.maxStringLength) {
          return fail("string_limit", "string acima do limite configurado");
        }
        return decoded;
      }
      if (char === "\\") {
        index += 2;
        continue;
      }
      if (text.charCodeAt(index) < 0x20) {
        return fail("syntax", "caractere de controle dentro de string");
      }
      index += 1;
    }
    return fail("syntax", "string JSON nao terminada");
  };

  const parseNumber = (): number => {
    NUMBER_TOKEN.lastIndex = index;
    const match = NUMBER_TOKEN.exec(text);
    if (match === null) {
      return fail("syntax", "numero JSON invalido");
    }
    index += match[0].length;
    const parsed = Number(match[0]);
    if (!Number.isFinite(parsed)) {
      return fail("syntax", "numero JSON fora de faixa");
    }
    return parsed;
  };

  const parseArray = (depth: number): unknown[] => {
    countValue(depth);
    index += 1;
    const result: unknown[] = [];
    skipWhitespace();
    if (text[index] === "]") {
      index += 1;
      return result;
    }
    for (;;) {
      result.push(parseValue(depth));
      skipWhitespace();
      const char = text[index];
      if (char === ",") {
        index += 1;
        continue;
      }
      if (char === "]") {
        index += 1;
        return result;
      }
      return fail("syntax", "array JSON malformado");
    }
  };

  const parseObject = (depth: number): Record<string, unknown> => {
    countValue(depth);
    index += 1;
    const result: Record<string, unknown> = Object.create(null) as Record<string, unknown>;
    const keys = new Set<string>();
    skipWhitespace();
    if (text[index] === "}") {
      index += 1;
      return result;
    }
    for (;;) {
      skipWhitespace();
      if (text[index] !== '"') {
        return fail("syntax", "chave JSON esperada");
      }
      const key = parseString();
      if (FORBIDDEN_KEYS.has(key)) {
        return fail("forbidden_key", "chave proibida no documento");
      }
      if (keys.has(key)) {
        return fail("duplicate_key", "chave duplicada no mesmo objeto");
      }
      keys.add(key);
      skipWhitespace();
      if (text[index] !== ":") {
        return fail("syntax", "separador de objeto esperado");
      }
      index += 1;
      result[key] = parseValue(depth);
      skipWhitespace();
      const char = text[index];
      if (char === ",") {
        index += 1;
        continue;
      }
      if (char === "}") {
        index += 1;
        return result;
      }
      return fail("syntax", "objeto JSON malformado");
    }
  };

  const parseValue = (parentDepth: number): unknown => {
    skipWhitespace();
    if (index >= length) {
      return fail("syntax", "fim de arquivo inesperado");
    }
    const char = text[index];
    if (char === "{" || char === "[") {
      const depth = parentDepth + 1;
      if (depth > limits.maxDepth) {
        return fail("depth", "profundidade JSON acima do limite");
      }
      return char === "{" ? parseObject(depth) : parseArray(depth);
    }
    if (char === '"') {
      const value = parseString();
      countValue(parentDepth);
      return value;
    }
    if (char === "t" && text.startsWith("true", index)) {
      index += 4;
      countValue(parentDepth);
      return true;
    }
    if (char === "f" && text.startsWith("false", index)) {
      index += 5;
      countValue(parentDepth);
      return false;
    }
    if (char === "n" && text.startsWith("null", index)) {
      index += 4;
      countValue(parentDepth);
      return null;
    }
    if (char === "-" || (char >= "0" && char <= "9")) {
      const value = parseNumber();
      countValue(parentDepth);
      return value;
    }
    return fail("syntax", "valor JSON inesperado");
  };

  const value = parseValue(0);
  skipWhitespace();
  if (index !== length) {
    fail("syntax", "conteudo apos o documento JSON");
  }
  return { value, stats: { values, maxDepth: maxDepthSeen } };
}
