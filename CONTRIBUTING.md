# Contribuição

Abra uma issue com o componente (dashboard ou monitor nativo), reprodução/objetivo e critério de aceite. P.O define conteúdo e prioridade; labels não são aceite.

Envie alterações por PR pequeno. Declare comandos executados, resultado, limites e evidência de QA independente quando aplicável. Testes sintéticos não provam leitura de hardware. Não envie dados operacionais, segredos, perfis, exports ou binários.

Não importe código do candidato congelado automaticamente. Importação de fontes requer aceite do conteúdo/proveniência e decisão de licença. Não declare MIT ou copyright como aprovados.

CI: dashboard usa Node 24, npm ci, testes existentes, build e typecheck; core usa CMake >=3.21, C++20, UI OFF e CTest. Componentes ausentes são SKIPPED, nunca certificados como testados. UI Qt, HID e instaladores não fazem parte desta CI.

Commits criados pelo OpenBot incluem o trailer Co-authored-by: OpenBot <335604065+openbotgit[bot]@users.noreply.github.com>.
