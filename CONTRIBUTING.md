# Contribuição

Abra issue com componente (dashboard/monitor nativo), reprodução ou objetivo e critério de aceite. P.O define conteúdo e prioridade; labels não são aceite.

Envie PR pequeno com comandos/resultados, limites e QA independente aplicável. Não envie dados operacionais, segredos, perfis, exports ou binários sem fluxo de distribuição revisado.

MIT/copyright brunotitata para código próprio e Qt dinâmico LGPL foram aprovados pelo titular: legal/APPROVALS.md. Não substituir fontes recentes pelo candidato congelado. Proveniência de código/terceiros continua necessária; licença própria não substitui avisos ou obrigações Qt.

CI: dashboard Node24/npm ci/testes/build/typecheck; core CMake/C++20/CTest sem Qt em Ubuntu24.04 e macOS15. Build preparatório Qt macOS15 provisiona SDK efêmero e verifica hashes oficiais. Componentes ausentes são explicitamente SKIPPED; build/runner não provam GUI/hardware. DMG público exige integração real, pacote legal e gates de distribuição; não publicar janela vazia como MVP.

Commits OpenBot incluem Co-authored-by: OpenBot <335604065+openbotgit[bot]@users.noreply.github.com>.
