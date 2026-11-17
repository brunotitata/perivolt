# UI/macOS — revisão específica de fontes

Data: 2026-11-17. Commit examinado: `b41e7ec325072cb85aaea405c56675b5540bb368` de `brunotitata/perivolt`.

## Resultado

Não identifiquei impedimento de licenciamento à inclusão das fontes UI/macOS abaixo no projeto sob a MIT própria aprovada. Foram lidos os conteúdos do commit, sem presumir a revisão do snapshot anterior. A conclusão usa a declaração já recebida de autoridade sobre contribuições próprias; não certifica originalidade universal nem título sobre material de terceiros eventualmente não declarado. Não exige nova aprovação MIT/Qt.

As 12 entradas (sete UI, cinco macOS) coincidem byte a byte por SHA-256 com `docs/import/current-files.json` do commit. O manifesto declara origem `current-working-source-verbatim`, que descreve origem operacional, não autoria independente. LICENSE e NOTICE do commit preservam MIT/copyright `2026 brunotitata` e a distinção de terceiros. A revisão não cobre os demais incrementos de domínio/dashboard nem um binário/DMG.

| Grupo / arquivos | Proveniência constatada no conteúdo | Licença / aviso aplicável |
| --- | --- | --- |
| `app/ui/main.cpp`, `monitor_window.cpp`, `monitor_window.hpp` | Aplicação que usa Qt Widgets; nenhuma implementação Qt vendorizada identificada. Ícone “P” é desenhado por QPainter no código; não há imagem/fonte externa embutida nesses arquivos. | Código próprio sob MIT aprovada. Qt retém seus termos; ao distribuir binários observar LGPL/terceiros do Qt real. |
| `app/ui/presenter.cpp`, `presenter.hpp`, `tests/presenter_test.cpp`, `README.md` | Código de apresentação e testes, referências internas ao domínio e biblioteca padrão C++; fixtures sintéticas descritas como tais. Nenhum pacote/dado externo incorporado identificado. | MIT própria aprovada; manter LICENSE e NOTICE do projeto. Não demonstra licença de qualquer implementação futura externa. |
| `app/adapters/attackshark-x11/platform/macos/discovery_macos.cpp`, `discovery_main.cpp` | Uso de APIs IOKit/CoreFoundation e headers do SDK por include; wrappers e lógica do projeto. Não foram encontrados arquivos de implementação/header Apple copiados no recorte. | MIT do código próprio; nomes/chamadas de API não tornam automaticamente o aplicativo APSL. Não redistribuir SDK/frameworks Apple como se cobertos pela MIT. |
| `platform/macos/adapter.hpp`, `adapter_test.cpp`, `README.md` (mesmo prefixo anterior) | Tipos/lógica de fronteira, transporte sintético de teste e uso do parser interno. README fixa fontes primárias Apple e referência comunitária de protocolo. | MIT própria; conservar proveniência do protocolo upstream já atribuída no projeto. Headers Apple referenciados mantêm APSL-2.0; não afirmar que sua licença foi transferida à implementação própria. |

Não há evidência nesse recorte de código copiado de exemplos Qt que exija aviso BSD adicional, nem de libvips/Next empacotado. Isso não prova ausência no bundle futuro. Não alterei os fontes para adicionar cabeçalhos por arquivo: a MIT não obriga inventar autoria individual nem essa forma específica de aviso; LICENSE/NOTICE do projeto devem acompanhar as cópias relevantes.

## Referências e evidências

- [Commit examinado](https://github.com/brunotitata/perivolt/commit/b41e7ec325072cb85aaea405c56675b5540bb368), [manifesto fixado](https://github.com/brunotitata/perivolt/blob/b41e7ec325072cb85aaea405c56675b5540bb368/docs/import/current-files.json).
- README macOS desse commit indica IOKitUser `323ead896d04424f87184d8f6ff0cce811aab106` e IOHIDFamily `777ccd9698845aadf711e32d843c8c9b777431d9`. Conferidos diretamente: [IOKitLib.h](https://github.com/apple-oss-distributions/IOKitUser/blob/323ead896d04424f87184d8f6ff0cce811aab106/IOKitLib.h), [IOHIDKeys.h](https://github.com/apple-oss-distributions/IOHIDFamily/blob/777ccd9698845aadf711e32d843c8c9b777431d9/IOHIDFamily/IOHIDKeys.h), [IOHIDDeviceKeys.h](https://github.com/apple-oss-distributions/IOHIDFamily/blob/777ccd9698845aadf711e32d843c8c9b777431d9/IOHIDFamily/IOHIDDeviceKeys.h), com cabeçalhos APSL-2.0. Não foi feita comparação exaustiva contra todos os exemplos Apple ou Qt existentes.
- [QWidget Qt v6.12.0](https://github.com/qt/qtbase/blob/v6.12.0/src/widgets/kernel/qwidget.h): confirma opção LGPL-3.0-only no header dessa versão, além das alternativas declaradas. Não confirma a licença de todos os módulos/plugins.
- `evidence/ui-macos-b41e7ec.json`: conteúdos obtidos do commit para os 12 arquivos, LICENSE/NOTICE e manifesto; hashes e resultado 12/12. Referências adicionais registram URL, hash e trecho inicial para rastreabilidade. O arquivo é evidência local; o suplemento público pode apontar diretamente para o commit sem duplicar os fontes.

## Qt 6.12.0 — limite desta entrega

DevOps informou SDK `6.12.0-0-202609280346`, SHA-256 qtbase `1e0df66ed17ae97997971ff481983c5c0b7497a52ef52f699725701401fd80ee` e qttools `c503c114fbb96a2c2dad07fae4d195cb800ab498c9deb440c879e657ec14e8f8`. São identificadores repassados, ainda não confrontados aqui com os arquivos SDK ou índices oficiais. Não equivalem aos hashes dos arquivos de fonte correspondente. O header primário v6.12.0 foi consultado independentemente; não foi baixado/revisado o SDK inteiro.

CI preparatório sem distribuição binária pode prosseguir pela rota autorizada. Antes da distribuição, fornecer URLs exatas dos arquivos SDK/fontes, hashes, patches/configuração do fornecedor, inventário real do bundle com todos os plugins e terceiros, avisos incluídos, acesso à fonte correspondente e evidência de substituição/execução com Qt compatível. Qttools usado apenas para build não implica que todo esse pacote seja redistribuído com o aplicativo. Aplicam-se os critérios de `QT-MACOS-DMG-REVIEW.md`; a alternativa 6.8.3 ali descrita é histórica, não SDK escolhido automaticamente. Compatibilidade macOS/arquitetura, assinatura, notarização e hardware ficam fora deste parecer.

Escrita: este suplemento, atribuição complementar e evidência local em legal. Nenhum fonte original, manifesto de importação, commit, PR ou publicação alterado por Licenciamento.

