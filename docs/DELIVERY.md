# Entrega e limites

MIT/copyright brunotitata e rota Qt dinâmico LGPL aprovados em 2026-11-17: legal/APPROVALS.md. Fontes atuais importadas por allowlist e manifestadas em docs/import; fontes/data de operação, diagnósticos HID, tools e builds locais excluídos. Nunca restaurar snapshot congelado sobre incremento recente.

Dashboard Next.js é separado do monitor C++/Qt. CI real: Next Node24 npm ci/testes/build/typecheck; core CMake/CTest Ubuntu24.04/macOS15 UI OFF; workflow macOS preparatório com UI ON e SDKQt6.12.0 público por hashes oficiais em runner padrão macOS15. macOS14 foi rejeitado pelo serviço como configuração não suportada. SDK alvo de build macOS14.4+, arquitetura comprovada no runner e em cada Mach-O; não prometer compatibilidade no Mac do usuário por inferência.

Release-draft.yml continua preparando somente rascunho de fontes, sem assets; não entrega o requisito DMG. DMG de teste público é autorizado, mas depende de integração real, inventário binário/avisos/fontes exatas/substituição Qt revisados. app/build-support/MACOS-DELIVERY.md descreve evidências. Runner não substitui QA Mac/hardware. Não alegar assinatura/notarização quando ausentes.

main possui proteção aplicada: PR, check administration de GitHub Actions, atualização com base e conversas resolvidas, sem force push/deleção. Administradores mantêm exceção conforme bootstrap. Os checks de produto não são exigidos por proteção até sua execução real estabilizada; falhas reais impedem declarar incremento aprovado. Payload .github/branch-protection.json é referência, estado efetivo deve ser relido no GitHub.

Taxonomia existente é apenas verificada em modo read-only; P.O controla nomes/escopo dos milestones e conteúdo/prioridade das issues.
