# Entrega e limites

Estado inicial: somente administração, sem fontes do produto e sem licença aprovada. A CI identifica a ausência de cada componente e registra SKIP explícito; sucesso administrativo não é sucesso de build do produto.

Após importação autorizada das fontes atuais (nunca substituir pelo snapshot antigo): dashboard Next separado do monitor, npm ci/testes/build/typecheck em Ubuntu; core CMake/CTest em Ubuntu e macOS, PERIVOLT_BUILD_UI=OFF. Nenhum resultado certifica Qt, HID ou hardware. Não instalar Qt nesta etapa.

Release: workflow_dispatch prepara somente rascunho, sem assets e sem publicação final. Gates obrigatórios: LICENSE aprovada presente, declaração de aceite de licença e QA, ambos componentes presentes, CI do mesmo SHA com sucesso. Registrar evidências/restrições no rascunho; publicação final exige decisão posterior. Não há deploy de dashboard ou instalador.

Proteção recomendada para main após bootstrap: PR obrigatório, conversa resolvida, sem force push/deleção, administration requerido. Adicionar checks de produto como obrigatórios somente após fontes importadas e primeira execução real bem-sucedida. Não exigir aprovação de terceiro indisponível para o bootstrap. API de proteção requer permissão Administration de escrita; GITHUB_TOKEN de workflows não a possui. Não afirmar proteção ativa sem leitura de confirmação.

Labels são taxonomia administrativa; prioridade e conteúdo das issues pertencem ao P.O.

Payload revisável: .github/branch-protection.json. Aplicação com sessão administrativa própria: `gh api --method PUT repos/brunotitata/perivolt/branches/main/protection --input .github/branch-protection.json`. Depois conferir `gh api repos/brunotitata/perivolt/branches/main/protection`. Este arquivo não ativa a proteção sozinho.
