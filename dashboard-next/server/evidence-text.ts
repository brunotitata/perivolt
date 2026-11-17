export function sanitizeEvidenceText(value: string | null): string | null {
  if (value === null) {
    return null;
  }
  const trimmed = value.trim();
  if (trimmed.length === 0) {
    return null;
  }
  // Politica conservadora: omitir todo o campo quando houver caminho privado.
  // URLs e locators relativos continuam legiveis; isto nao e filtro de segredos.
  const absolute = /(?:^|[\s\[\]{}<>`("'=,:])(?:[A-Za-z]:[\\/]|\\\\|\/(?!\/)[^\s])/;
  const traversal = /(?:^|[\s\[\]{}<>`("'=,:\\/])\.\.[\\/]/;
  if (absolute.test(trimmed) || traversal.test(trimmed)) {
    return "[caminho nao exibido]";
  }
  return trimmed.slice(0, 2000);
}
