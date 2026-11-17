# Revisão de licenciamento — MIT aprovada; recorte histórico

**Atualização administrativa 2026-11-17:** o titular aprovou MIT/copyright brunotitata e confirmou autoridade, conforme repasse do BIG BOSS. Autorizou também Qt dinâmico sob LGPL. Ver `APPROVALS.md`. As referências abaixo à decisão pendente descrevem o estado da revisão original e estão superadas por esse registro; não repetir a aprovação. A revisão técnica do recorte permanece limitada às fontes e versões ali examinadas, sem aprovação de um DMG futuro ou de incrementos recentes.

Data: 2026-11-17. Escopo: primeiro candidato GitHub PeriVolt, fontes de protótipo + lockfile, sem dependências vendorizadas, bundle, instalador, binários ou dados operacionais.

## Conclusão prática

**Revisão local concluída para o recorte descrito; proposta pronta para decisão do usuário, sem autorização de publicação.** Não identifiquei no conteúdo examinado uma obrigação de relicenciar o código próprio sob GPL/LGPL/CC BY. As obrigações de terceiros não desaparecem: estão delimitadas nos avisos e no inventário. A conclusão pressupõe verdadeira a declaração de que o código próprio foi produzido com direitos suficientes para licenciá-lo; hashes não demonstram autoria.

Recomendo **MIT**: adequada ao protótipo gratuito de código aberto, simples para reutilização, modificação e integração com componentes permissivos e bibliotecas LGPL sem impor copyleft ao código próprio. MIT permite também uso comercial; “gratuito” aqui não significa proibir venda por terceiros. Proibir uso comercial deixaria de atender ao objetivo OSS.

Apache-2.0 também seria opção permissiva válida e acrescentaria concessão expressa de patentes e regras próprias de NOTICE/alterações; é útil se essa política for desejada. GPL-3.0-or-later priorizaria reciprocidade nas distribuições de obras cobertas, com obrigações de disponibilizar fonte, mas não é necessária pelas dependências presentes. Nenhuma opção sana falta de direitos ou substitui licenças de terceiros. MIT e Apache-2.0 do aplicativo podem coexistir com LGPL-3.0 da biblioteca quando cumpridas suas condições; não se afirma compatibilidade de qualquer combinação futura não examinada.

`../LICENSE` contém MIT integral com aviso **proposto** `Copyright (c) 2026 brunotitata`. O handle foi fornecido pelo coordenador; não se inventou pessoa jurídica, nome civil ou contato. A confirmação necessária é autoridade para licenciar e adequação do aviso, não fornecimento obrigatório de nome civil. `../NOTICE` deixa explícito o estado de proposta e o escopo; não altera as condições MIT. A publicação só pode ocorrer depois da aprovação final do usuário, coordenada pelo BIG BOSS.

## Evidências do recorte

- Recebidos 52 arquivos; `review/inclusion.json` fixa 38 entradas de fonte/configuração, todas com SHA-256 conferido antes das alterações legais. “copied-verbatim” descreve cópia da árvore local para o candidato, não comprova origem intelectual.
- Lock SHA-256: `8fe85539404cc44a25e1022dfadf595d1bc3d4bc606bec7606c9766b6e78cf28`. As 60 entradas coincidem com `legal/npm-lock-inventory.json` e metadata das versões exatas obtida diretamente de registry.npmjs.org; zero divergências de licença/integridade. Ver `DEPENDENCIES.md` e `evidence/registry-exact.json`.
- Diretas: Next 16.4.6, React/React DOM 19.3.1. Desenvolvimento direto: TypeScript 5.9.3, @types/node 24.19.2, @types/react e @types/react-dom 19.3.0. O inventário inclui transitivas e opcionais por plataforma; não confunde todas as 60 com pacotes instalados em uma máquina.
- Nenhum node_modules, .next, binário de produto, asset externo de imagem/fonte tipográfica ou base caniuse foi encontrado na listagem do recorte. CSS usa fontes de sistema por nome, sem redistribuir arquivos tipográficos. A biblioteca padrão/compiladores não são entregues como arquivos.
- Parser e sua referência fixa foram lidos; metadados, imports e referências do restante do recorte foram examinados para inventário/proveniência. Não se realizou comparação exaustiva com todos os repositórios públicos nem certificação de titularidade. A afirmação de originalidade do coordenador é declaração, não prova independente.
- A referência documental attackshark-x11-linux possui MIT no commit fixado; aviso integral preservado. Não foi encontrada implementação upstream copiada no parser examinado; o conteúdo usa campos factuais de protocolo. Ver `ATTRIBUTIONS.md`.
- `next-env.d.ts` é boilerplate gerado que referencia tipos, sem implementar Next. Por cautela, o aviso MIT do pacote exato foi preservado em `evidence/next-LICENSE.txt`, verificado por integridade do tarball. Os componentes internos compilados de Next não foram inventariados integralmente porque não estão distribuídos.
- Actions checkout/setup-node foram verificadas nos SHAs constantes da CI; MIT em ambas. São referências de execução remota, não redistribuição desses programas. Qt e implementações GPL de referência não aparecem como arquivos no recorte; não se atribui licença a uma futura versão Qt não escolhida.

## O que cada forma de entrega exige

### Este candidato: fontes próprias + manifestos + avisos

Manter LICENSE após aprovação, NOTICE, atribuições e avisos preservados. A inclusão de nome/versão/URL/integridade no lock não distribui o conteúdo dos pacotes nem copia bases de dados. Portanto, este candidato não aciona por si a entrega de fontes de libvips ou atribuição por redistribuição de caniuse; a atribuição preventiva já está redigida. Os textos de licença em evidence mantêm seus próprios termos e não são relicenciados sob MIT.

### Instalação para uso local

`npm ci` obtém pacotes e opcionais apropriados à plataforma a partir do registro. Uso privado, sem repassar pacotes, não equivale à distribuição pública de binários. Um lock multiplataforma não prova quais pacotes de SO foram instalados. Não executei instalação. Hospedar apenas funcionalidade no servidor não equivale automaticamente a entregar bibliotecas; JavaScript, dados e assets enviados ao navegador são distribuídos e precisam de exame do build.

### Futura entrega de node_modules, bundle, imagem de container ou instalador

Revisar o artefato concreto e todos os componentes incorporados, inclusive os empacotados dentro de Next/SWC/sharp/libvips, antes de liberar. Conservar copyrights, textos e avisos exigidos; MIT/ISC/BSD exigem preservação de avisos nas condições aplicáveis; BSD-3-Clause também impede endosso pelo nome; Apache-2.0 §4 exige licença, avisos relevantes, indicação de arquivos modificados e NOTICE upstream quando fornecido. A MIT do PeriVolt não se sobrepõe a isso.

**sharp/libvips:** sharp 0.35.5 é Apache-2.0. @img/sharp-libvips-* 1.3.4 declara LGPL-3.0-or-later; variantes Win 0.35.5 combinam Apache-2.0 AND LGPL-3.0-or-later; Wasm 0.35.5 acrescenta MIT. Os tarballs Linux x64, Win x64 e Wasm examinados por SHA-512 registram libvips **8.18.7** em versions.json. Essa versão não foi presumida para todas as variantes. As snapshots incluem outras bibliotecas incorporadas; isso demonstra por que a metadata de uma dependência npm não é um inventário completo de um binário.

LGPLv3 incorpora GPLv3. Ao distribuir a biblioteca/obra combinada, conservar avisos e cópias das duas licenças; disponibilizar fonte correspondente da biblioteca, modificações e material necessário segundo GPLv3 §6 quando aplicável. Para download, uma via concreta é oferecer a fonte exata com acesso equivalente e instruções junto ao binário (§6d); depender apenas de link genérico a upstream não garante cumprir disponibilidade. LGPLv3 §4 exige permitir modificação da biblioteca e engenharia reversa para depurar tais mudanças, e usar mecanismo adequado de biblioteca compartilhada substituível (§4d1), ou entregar fonte mínima correspondente e código da aplicação em forma apta a recombinação/relink (§4d0). Fornecer informações de instalação quando §4e/GPLv3 §6 as exigirem. Vinculação estática/Wasm requer atenção à possibilidade efetiva de recombinar; abrir apenas a fonte do aplicativo não resolve automaticamente todas as obrigações. Não basta mencionar LGPL, e não é obrigatório tornar todo aplicativo LGPL só por usar a biblioteca sob as permissões cumpridas.

**caniuse-lite 1.0.30001826:** metadata e LICENSE do tarball exato confirmam CC-BY-4.0. Se dados ou adaptações forem compartilhados, CC BY §3 pede identificação/avisos fornecidos, link à licença/material quando praticável e indicação de alterações; §4 trata dos direitos de banco de dados quando aplicáveis. Não sugerir endosso e não impor restrições adicionais ao material coberto. `ATTRIBUTIONS.md` traz texto preventivo; adaptar à saída final e aos avisos realmente fornecidos. CC BY dos dados não se aplica automaticamente à implementação própria.

Os cinco tarballs sharp/caniuse/plataformas e o tarball Next foram baixados só para leitura em memória; não se instalou/executou seu código nem se guardaram binários. Foram preservados apenas textos de licença e versões. Os textos extraídos com decodificação substitutiva no caniuse não são fonte canônica para redistribuição byte a byte: usar o LICENSE original do tarball indicado. A revisão de binários futuros permanece fora da aprovação deste recorte.

## Pendências reais e encaminhamento

1. **Usuário:** aprovar MIT e confirmar autoridade sobre contribuições próprias e aviso proposto com handle. Caso haja terceiros com direitos não resolvidos, identificar os arquivos e autorizações antes da publicação. Sem essa confirmação, não tratar LICENSE como licença final adotada.
2. **BIG BOSS / dono da montagem:** após decisão, regenerar hashes, inventários de arquivos e eventual ZIP e ajustar os textos públicos de “pendente/proposta” para o estado efetivamente aprovado. Os antigos 52 arquivos/hashes não descrevem as adições legais desta entrega. Não alterei manifestos ou ZIP.
3. **Fora deste recorte:** revisão de licenças de artefato que incorpore dependências/binários/dados; não é bloqueio adicional para publicar somente as fontes aqui descritas após os itens anteriores.

## Fontes primárias consultadas em 2026-11-17

- [MIT / OSI](https://opensource.org/license/mit): concessão permissiva, aviso e disclaimer.
- [Apache-2.0, §§3–4](https://www.apache.org/licenses/LICENSE-2.0): patentes e redistribuição.
- [LGPLv3](https://www.gnu.org/licenses/lgpl-3.0.txt) e [GPLv3](https://www.gnu.org/licenses/gpl-3.0.txt): textos completos obtidos diretamente via HTTPS e preservados em evidence. O leitor web apresentou timeout; a consulta direta teve sucesso, sem contornar bloqueio de segurança.
- [CC BY 4.0 legalcode, §§3–4](https://creativecommons.org/licenses/by/4.0/legalcode): condições de atribuição/bancos de dados.
- Cada URL npm de versão exata em `DEPENDENCIES.md`, com captura em `evidence/registry-exact.json`; tarballs/integrais indicados nas seis capturas de evidência.
- [sharp 0.35.5 LICENSE](https://github.com/lovell/sharp/blob/v0.35.5/LICENSE), [instalação oficial](https://sharp.pixelplumbing.com/install/) e [empacotamento sharp-libvips](https://github.com/lovell/sharp-libvips): contexto técnico. O conteúdo exato de versão prevalece sobre páginas mutáveis.
- [Protocolo e LICENSE no commit fixado](https://github.com/MiguelSanzBr/attackshark-x11-linux/tree/e13a99644dcf3538d8cc5908a07832318b49cf2d), `ATTRIBUTIONS.md`.
- [checkout LICENSE no SHA](https://github.com/actions/checkout/blob/11d5960a326750d5838078e36cf38b85af677262/LICENSE), [setup-node LICENSE no SHA](https://github.com/actions/setup-node/blob/49933ea5288caeca8642d1e84afbd3f7d6820020/LICENSE).
- [caniuse-lite README](https://github.com/browserslist/caniuse-lite#license) e [Can I use](https://caniuse.com/): origem e atribuição preventiva. Páginas atuais são contexto; versão/licença pinned foram verificadas no tarball.

Não houve QA, teste de aplicativo, build, instalação, commit, push ou publicação nesta revisão. Limites: não certifica autoria histórica, conformidade de futuros binários ou suporte de hardware. Verificação documental final em `evidence/verification.json`.
