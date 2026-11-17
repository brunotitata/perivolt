# Monitor nativo — primeira fatia UI

`main.cpp` abre `MonitorWindow` vazio e sem callbacks. Nenhum dispositivo é
criado pela UI. Consentimento inicia OFF e controles de leitura/importação ficam
indisponíveis até ligação real. Bandeja é opcional; sem suporte, a janela continua
disponível. Fechar para bandeja exige opção explícita, desmarcada inicialmente.

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

CMake pertence a DevOps. Não há build/executável Qt, validação visual nem hardware
validado por esta entrega. Qt/macOS/Linux não foram executados. Main standalone
permanece vazio até Backend/Hardware conectarem um produtor real.
