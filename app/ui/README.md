# Monitor nativo — primeira fatia UI

**Preview técnico / não lê bateria / não cumpre MVP.**

`main.cpp` conecta Monitor da camada application. No macOS, inicia discovery
registry-only em worker; timer GUI de 250 ms recebe snapshot e metadados.
Presenca/completude/erro/contagens aparecem separados da lista de bateria.
Metadados nao comprovam identidade, conexao ou bateria; nao criam linhas reais.
Consentimento inicia OFF e capacidade de leitura permanece indisponivel.
Sair chama shutdown, invalidando conclusoes pendentes. Sem HID ou bateria nativa.
Bandeja opcional; fechar para bandeja exige escolha explicita.

## Integração

- C++17 no presenter; Qt6 Widgets na janela/entrypoint, sem Q_OBJECT/moc próprio.
- `setIntents(Intents)` conecta solicitações de leitura e importação. Não há I/O,
  domínio de alertas, parser de arquivo, HID ou servidor nesta camada.
- `setSnapshot(Snapshot)` recebe lista completa real, `State` do core, nome,
  fonte, idade opcional calculada pelo produtor e `ReadControl`. A lista ativa
  aceita apenas `present` com origem de sessão. Consentimento/permissão/capacidade
  são independentes. Clique envia intenção e espera novo snapshot, sem conceder
  consentimento por conta própria. Não há persistência de preferências nesta fatia.
- Métodos da janela devem executar no thread GUI. Produtor externo usa entrega
  enfileirada com contexto/lifetime da janela; não chamar QWidget de outra thread.
- Importação só emite intenção após escolha explícita de arquivo. Serviço retorna
  `completeHistoricalImport(request_id, readings, error)` no thread GUI. IDs antigos,
  inexistentes ou repetidos são recusados. Dados são marcados históricos e nunca
  entram no snapshot ativo, nem disparam `observe`/avaliação de alertas. Idade
  permanece a fornecida pelo produtor, sem relógio/import time substituto.
- Strings externas são texto simples. Seleção da tabela usa `(device,component)`.
  Frescor de nível/carga/conexão aparece separadamente. Nenhum timer fabrica idade.

## Verificação focalizada

Na raiz do projeto: `powershell -NoProfile -File app/ui/run-tests.ps1`.
Usa w64devkit local; escreve `app/ui/tests/presenter_test.exe`. Fixtures sintéticas
existem apenas em `tests/presenter_test.cpp`. Casos: 0/100, ausência/NaN,
qualitativo, stale preservado, idade, remoção, consentimento/capacidade/permissão,
isolamento e correlação da importação histórica, status de monitoramento.

CMake pertence a DevOps. Build Qt/macOS deste lote deve ser comprovado no runner;
nao foi executado neste host Windows. Testes locais do presenter nao equivalem
a GUI, hardware, suporte macOS ou aceite MVP. O produtor ligado e apenas discovery
de metadados; nao ha transporte de bateria.
