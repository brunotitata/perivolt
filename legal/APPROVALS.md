# Decisões de licenciamento e alcance — 2026-11-17

## Registro administrativo

Origem: decisões explícitas do titular, obtidas por consulta e repassadas pela coordenação do projeto em 2026-11-17:

1. Pergunta: “Você aprova a licença MIT e o aviso de copyright ‘brunotitata’ para o código próprio do PeriVolt, confirmando que tem autoridade para licenciá-lo?” Resposta: “Aprovar MIT e copyright”.
2. Pergunta: “Você autoriza seguir com Qt vinculado dinamicamente sob LGPL para o aplicativo macOS?” Resposta: “Autorizar Qt sob LGPL”.

Registro do repasse, não verificação independente de autoria. MIT/copyright e autoridade estão resolvidos para o código próprio. Não se afirma identidade civil nem se relicenciam contribuições de terceiros. Não há aprovação de conformidade de DMG ainda não produzido. A autorização de pipeline/publicação é coordenada pelo responsável do projeto, não concedida por este parecer.

## Caminhos para copiar sem alterar conteúdo

Relativos a `publication-candidate/`:

- `LICENSE`: MIT integral, `Copyright (c) 2026 brunotitata`; conteúdo já coincide com a decisão, sem alteração nesta atualização.
- `NOTICE`: decisão formalizada, escopo do recorte de fontes e terceiros. Não usar como inventário de um bundle Qt.
- `legal/APPROVALS.md`, `legal/README.md`, `legal/REVIEW.md`: decisões e limites administrativos.
- `legal/QT-MACOS-DMG-REVIEW.md`: critérios binários, rota Qt 6.8.3 proposta e evidências pendentes.
- `legal/ATTRIBUTIONS.md`, `legal/DEPENDENCIES.md`, `legal/evidence/`: atribuições/fontes do recorte anterior; não presumir que tudo integra o app. Para distribuição, preservar os avisos originais aplicáveis às versões realmente presentes.

Avisos novos Qt/terceiros devem ser concretizados com versão e conteúdo do SDK/bundle fixado. Textos GNU existentes não comprovam inclusão no pacote. DevOps pode copiar os documentos; deve regenerar hashes, manifestos e ZIP e ajustar marcas administrativas fora do escopo de escrita de Licenciamento.

## Incrementos app/ui e platform/macos

Os caminhos `app/ui/` e `app/adapters/attackshark-x11/platform/macos/` estão fora da revisão congelada anterior. A aprovação MIT cobre contribuições próprias autorizadas nesses caminhos; não prova que cada arquivo seja próprio. Inspeção focalizada de nomes e busca de avisos/referências não constitui revisão de proveniência completa nem comparação com upstream.

`platform/macos/README.md` registra referências a headers Apple IOKitUser/IOHIDFamily em commits específicos. Referência a API/documentação não demonstra cópia de implementação; tampouco autoriza remover aviso de material efetivamente copiado. UI usa Qt; os headers/bibliotecas Qt retêm suas licenças. Nenhuma nova libvips/Next foi constatada por esta inspeção, que não alcançou bundle final.

Para fechamento do incremento: DevOps deve fixar commit/PR e lista de arquivos com hashes das fontes recentes, preservando origem dos donos e informando adaptações/cópias de terceiros. A revisão de licenciamento desse conjunto deve confrontar licenças/avisos com o material real antes de concluir cobertura. O snapshot anterior de 38 fontes não pode servir de evidência para arquivos recentes nem substituí-los.

## Gate binário Qt

Build preparatório pode seguir pela rota autorizada; distribuição depende de inventário final, licenças/avisos aplicáveis, fontes correspondentes exatas com patches/receitas, acesso junto ao download e substituição/execução de Qt compatível comprovada. Plugins transitivos e bibliotecas incorporadas entram na revisão; módulos GPL-only não são automaticamente LGPL. Termos MIT próprios não dispensam essas obrigações. Assinatura, notarização, integração e hardware não foram validados por Licenciamento.

Escritas desta atualização: NOTICE e documentos administrativos em legal. LICENSE preservada; sem mudança em código, manifestos, ZIP, pipeline ou publicação.
