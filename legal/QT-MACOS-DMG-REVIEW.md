# Parecer candidato — monitor Qt6 / DMG macOS

Data: 2026-11-17. Destinatários: BIG BOSS e DevOps. Não autoriza publicação.

**Atualização administrativa:** BIG BOSS repassou aprovação expressa do titular para MIT/copyright brunotitata com autoridade e para Qt dinâmico sob LGPL. Ver `APPROVALS.md`. Referências abaixo à decisão pendente do titular estão superadas; não repetir essas aprovações. Permanecem versão/inventário, proveniência, fontes correspondentes, avisos e substituição/execução no pacote final. Nenhum DMG inexistente está aprovado.

**Qt 6.8.3 proposto pelo DevOps:** há fontes públicas oficiais em https://download.qt.io/archive/qt/6.8/6.8.3/single/ (consultado em 2026-11-17). Não identifiquei impedimento de licenciamento para preparar build Widgets dinâmico dessa release pública sob LGPL, condicionado aos módulos/terceiros exatos. Registrar URL, hash SHA-256, configuração e patches; fontes vanilla não bastam se o SDK tiver alterações de fornecedor. Incluir dependências incorporadas e scripts necessários à construção. Disponibilidade pública não comprova suporte atual ou QA. macdeployqt usado como ferramenta de build não impõe por si GPL à saída; redistribuir a ferramenta exigiria análise própria. A versão ainda é proposta, não SDK/bundle examinado.

## Decisão atual

Existe caminho de distribuição com Qt Widgets dinâmico sob LGPLv3, sem obrigar automaticamente a aplicação própria a adotar MIT ou divulgar seu código. **O DMG concreto ainda não está aprovado:** faltam artefato, versão exata, proveniência, inventário e termos próprios autorizados pelo titular. Prerelease gratuito para terceiros também é distribuição. A revisão anterior do Next não cobre o monitor.

Inspeção local: `app/CMakeLists.txt` pede `find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)`, liga `Qt6::Widgets` e cria `MACOSX_BUNDLE`. Isso é um piso, não versão fixada nem prova de ligação dinâmica. `app/ui/main.cpp` abre uma janela sem produtor/import handler; não é arquivo vazio. `app/build-support/validation.md` registra ausência de SDK Qt e nenhuma execução macOS. Não encontrei DMG/dylib nos caminhos `app` e `publication-candidate` pesquisados. Não inspecionei host remoto nem artefato fora desses caminhos. Não executei build/QA.

## Aquisição permitida proposta

Escolher explicitamente uma destas rotas antes do build distribuível:

1. **Open source/LGPL:** adquirir release público oficial Qt elegível, via distribuição open source da Qt ou fontes oficiais; fixar patch, arquitetura, URL, hash, termos e configuração do SDK. Preferir QtBase dinâmico com Widgets e somente módulos/plugins necessários. Se usar pacote de terceiro (por exemplo Homebrew/aqt), registrar também receita/versão/patches e origem real dos binários: licença do gerenciador não licencia o Qt. Não presumir elegibilidade de release comercial/LTS restrito nem de avaliação. O repositório público oficial de releases é fonte possível, não seleção de versão já feita.
2. **Comercial:** titular providencia licença Qt válida cobrindo desenvolvedores, aquisição e distribuição pretendidas; revisar o contrato concreto e suas restrições. Não basta compra futura presumida ou trial. Licenças de terceiros embutidos continuam exigindo revisão.

Recomendação para este desenho: rota LGPLv3 dinâmica, condicionada à elegibilidade de cada componente e possibilidade real de substituição. Não comprar, instalar ou aceitar termos em nome do usuário por este parecer.

## Pacote legal necessário na rota LGPL

| Item | Condição para distribuição |
| --- | --- |
| QtCore, QtGui e QtWidgets | São componentes esperados do desenho Widgets, ainda não inventário observado. Fixar versão e textos da release efetiva; conferir cada biblioteca incluída. |
| Plugins | Enumerar `Contents/PlugIns`, inclusive plataforma Cocoa, estilos e formatos de imagem que existirem. `macdeployqt` pode copiar plugins e dependências adicionais; sua execução não comprova conformidade. |
| Terceiros do Qt | Usar metadados de licença/SBOM da build exata e fontes correspondentes; identificar também componentes compilados dentro dos frameworks, não só dylibs externas. Preservar avisos e textos de cada licença aplicável. |
| Aviso LGPL | Incluir aviso destacado de uso de Qt, versões/componentes e cobertura LGPL; cópias integrais de LGPLv3 e GPLv3; copyrights reais do upstream. Se a execução exibir copyrights, incluir os do Qt e referência aos textos, conforme LGPL §4(c). |
| Fonte correspondente das bibliotecas | Entregar fontes exatas do Qt e demais componentes cobertos, incluindo patches e scripts/configuração necessários à reconstrução. Qt inalterado também exige fonte correspondente ao binário redistribuído. Não confundir com fonte própria do aplicativo. |
| Acesso às fontes | Para download de DMG, propor GPLv3 §6(d): arquivo de fontes junto à mesma página de download, acesso equivalente e sem custo adicional, instrução clara ao lado do binário. Pode haver servidor de terceiro com acesso equivalente, mas a responsabilidade de disponibilidade permanece do distribuidor. Link genérico para página inicial do Qt ou simples oferta de três anos não substitui automaticamente essa modalidade de distribuição. |
| Substituição/relink | Usar mecanismo dinâmico adequado que execute biblioteca modificada com interface compatível: LGPL §4(d)(1). Nessa rota não é automaticamente necessário distribuir objetos/fontes próprios para relink. Se o mecanismo não satisfizer isso ou houver ligação estática, reavaliar §4(d)(0): fonte mínima correspondente e código objeto e/ou fonte da aplicação em formato e termos que permitam recombinar/relinkar. Não prosseguir silenciosamente. |
| Termos próprios | Não podem restringir modificação das partes LGPL nem engenharia reversa para depurar essas modificações. Direitos sobre o aplicativo precisam vir do titular; LGPL não concede esses direitos próprios. |
| macOS/assinatura | Documentar procedimento viável de substituir frameworks/plugins e executar a cópia modificada. Verificar caminhos de carga, assinatura, hardened runtime e library validation da build real. Não alegar que toda assinatura impede LGPL nem que assinar garante atendimento. Informação de instalação é exigida quando presentes as condições de LGPL §4(e)/GPL §6; não presumir transferência de User Product apenas pelo download do DMG. Não entregar chaves privadas do distribuidor: resolver execução da cópia modificada por procedimento apropriado e comprovado. |

Destino proposto dos avisos no bundle: `Contents/Resources/licenses/`, com índice acessível e aviso Qt. Para fontes grandes, pacote separado acessível junto ao DMG é alternativa à inclusão no DMG. Os arquivos `legal/evidence/GPL-3.0.txt` e `LGPL-3.0.txt` existentes não provam que foram incorporados ao bundle.

Não transportar automaticamente libvips/sharp/caniuse/Node/Next para esta lista: só entram se o bundle ou pacote de distribuição os contiver. Ausência na declaração CMake não prova ausência no produto final. Se encontrados, inventariar versões e obrigações próprias. Módulo GPL-only não pode ser tratado como LGPL por estar no mesmo SDK; remover componente desnecessário ou submeter compatibilidade/rota comercial ao titular.

## Evidências que DevOps deve entregar para fechamento

- DMG e `.app` finais identificados por SHA-256, commit de origem, arquitetura e deployment target; versão exata do Qt, SDK/provedor, URLs/hashes e registro de alterações.
- Lista recursiva com hashes de executáveis, Frameworks, PlugIns e Resources; vínculos `otool -L` de todos os Mach-O, incluindo plugins; configuração/saída de `macdeployqt` e build que distinga bibliotecas compartilhadas de estáticas. Esses registros são insumos de licenciamento, não execução atribuída a esta revisão.
- SBOM/metadados de terceiros da build exata (Qt disponibiliza SBOM a partir de 6.8), textos originais de licença e fonte correspondente com patches e instruções. Para versões sem SBOM, produzir inventário equivalente a partir do SDK/fontes/build.
- Textos de distribuição próprios aprovados, avisos efetivamente incluídos, localização dos pacotes de fonte e demonstração documentada de substituição/execução de Qt compatível no macOS de destino. Registrar entitlements/assinatura usados para avaliar obstáculos.

## Decisão concreta a encaminhar ao titular pelo BIG BOSS

Proposta revisável, ainda NÃO aprovada: “Autorizo preparar a distribuição do monitor PeriVolt usando Qt elegível sob LGPLv3, com ligação dinâmica substituível, entrega dos avisos/licenças e fontes correspondentes das bibliotecas. Confirmo quem detém os direitos do código/ativos próprios e quais termos autorizo para distribuir o binário. A publicação do código próprio e a adoção da MIT dependem de decisão separada. A aprovação final será dada sobre o DMG identificado por hash e seu pacote legal.”

O titular pode escolher licença permissiva para seu código, termos de distribuição apenas do binário com ressalvas LGPL, ou rota comercial Qt. Não inventar nome de copyright nem usar LICENSE MIT candidato como autorização. Pendências técnicas e inventário permanecem mesmo após a decisão do titular. BIG BOSS coordena a aprovação final; este parecer não libera release.

## Fontes primárias e limites

Consultadas em 2026-11-17; documentação corrente não identifica a release instalada:

- https://doc.qt.io/qt-6/licensing.html — opções, módulos GPL-only, terceiros e SBOM.
- https://doc.qt.io/qt-6/qtwidgets-index.html — licença do módulo Widgets e uso CMake.
- https://www.qt.io/development/open-source-lgpl-obligations — orientação oficial Qt sobre fontes, relink e LGPL.
- https://doc.qt.io/qt-6/macos-deployment.html — bundle, frameworks e ferramenta de deployment.
- https://download.qt.io/official_releases/qt/ — distribuição pública oficial; nenhuma versão selecionada neste parecer.
- https://raw.githubusercontent.com/qt/qtbase/dev/LICENSES/LGPL-3.0-only.txt — texto LGPL lido diretamente do upstream Qt, §§3–4. URL `dev` serve ao texto da licença, não fixa versão do SDK.
- `legal/evidence/GPL-3.0.txt`, §6 — texto integral local já existente, lido para modalidades de entrega de fontes e informação de instalação. As consultas diretas a gnu.org falharam por timeout; nenhum bloqueio de certificado foi contornado.

Escrita desta tarefa: somente este parecer local. Sem alterações de código, LICENSE/NOTICE anterior, publicação, compra ou instalação.
