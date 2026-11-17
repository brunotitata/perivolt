# DMG de teste: preparação, ainda sem artefato

Estado observado em 2026-11-17: o entrypoint abre janela sem produtor; o
repositório remoto contém apenas infraestrutura. Não há MVP macOS integrado,
build Qt/macOS executado, DMG, identidade de assinatura ou notarização comprovada.
Estes scripts não autorizam distribuição e não instalam SDK, executam HID ou
alteram Gatekeeper. A autorização de release de teste não aprova MIT/copyright.

## Build após integração e aquisição do SDK autorizadas

O SDK Qt deve ter patch, URL, hash, módulos, origem dos binários e fontes
correspondentes registrados. O piso de API Qt 6.2 não é seleção de patch/SDK.
Não adicionar Next/Node/libvips ao monitor por conveniência de empacotamento.

Em Mac com CMake >=3.21, Python >=3.11 e SDK Qt dinâmico existente:

```bash
export QTDIR=/caminho/absoluto/do/Qt/aprovado
export PERIVOLT_ARCH=arm64
export PERIVOLT_MACOS_MIN=12.0
bash app/build-support/build-macos.sh
python3 app/build-support/package-macos.py \
  --app .local/macos-arm64/perivolt_monitor.app \
  --qt-root "$QTDIR" \
  --revision COMMIT_COMPLETO_DAS_FONTES \
  --notices /caminho/pacote-legal-revisado \
  --out .local/dmg-candidate-unico
```

Os valores arm64/12.0 são parâmetros de exemplo, não compatibilidade validada.
Selecionar conforme SDK real e Mac de QA. Inspecionar todos os registros Mach-O
`build_version` para determinar o mínimo efetivo, incluindo frameworks/plugins;
o Info.plist isoladamente não prova o mínimo. O script exige host macOS e recusa
sobrescrever diretório de saída. O build usa UI ON, falha se Qt faltar e executa
CTest; esses testes não executam HID nem demonstram integração real da GUI.

O empacotador copia o bundle, inclui notices, executa macdeployqt, verifica Cocoa,
arquiteturas e dependências absolutas externas, gera inventário SHA256 e registros
otool/vtool/codesign/SDK, cria UDZO, verifica e monta readonly e compara o bundle
montado byte a byte. Não inicia a GUI. Não assina, notariza, remove quarentena ou
altera preferências de segurança. Identidades disponíveis são apenas consultadas
no Keychain local; ausência será registrada, nunca substituída por certificado
inventado. Revisar rpaths e a resolução de dependências @rpath com a execução
real: o inventário estático não prova carregamento.

Saídas no diretório candidato: DMG, SHA256SUMS, artifact-evidence.json,
macdeployqt.log e stage/PeriVolt.app. `release_approved` é sempre false.

## Gates antes da publicação autorizada

1. TechLead/Frontend/Hardware entregam integração real identificada por commit.
   Não publicar o entrypoint vazio como MVP. Importação de fontes e termos
   próprios de distribuição devem estar autorizados pelo titular via BIG BOSS.
2. Licenciamento revisa bundle exato/inventário, versão/proveniência Qt, plugins,
   terceiros embutidos, avisos GPL/LGPL e fontes correspondentes das bibliotecas
   junto ao download. Não extrapolar a revisão de fontes do dashboard.
3. Provar substituição/execução real de Qt compatível e relatar condições de
   assinatura/library validation. O registro de codesign não é essa prova.
4. QA registra montar/copiar/abrir/executar no Mac real disponível, arquitetura,
   OS e resultado; runner de build não substitui QA/hardware. Uma prerelease pode
   registrar QA pendente apenas se o escopo de teste for explicitamente aceito;
   nunca anunciar teste executado por inferência.
5. Publicar prerelease associada ao SHA real, com DMG, SHA256SUMS, evidências
   sanitizadas, fontes das bibliotecas e avisos/termos aprovados. Verificar o asset
   baixando novamente e comparando SHA256. Sem decisão de licença própria
   aprovada, não converter MIT proposta em LICENSE definitiva.

A workflow release-draft.yml atual é do bootstrap de fontes e NÃO publica DMG.
Ela não deve ser usada como prova de entrega deste requisito. A automação de
publicação binária será ligada ao contrato/pacote legal concreto, sem contornar
os gates existentes por inputs que aleguem sucesso inexistente.

Referências primárias:
- https://doc.qt.io/qt-6/macos-deployment.html
- https://doc.qt.io/qt-6.8/supported-platforms.html
- Parecer local: publication-candidate/legal/QT-MACOS-DMG-REVIEW.md

