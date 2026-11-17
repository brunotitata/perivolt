# Proveniência e atribuições propostas

Proposta local sujeita à aprovação final. Não relicencia componentes de terceiros.

## Código próprio

Domínio C++20, parser, dashboard, testes sintéticos e scripts são apresentados pelo coordenador como contribuições próprias do projeto. Os 38 hashes de `review/inclusion.json` conferem. O campo `copied-verbatim` desse manifesto significa cópia da árvore local para o candidato; não é prova de cópia de terceiros nem de autoria própria. A autoridade para licenciar deve ser confirmada pelo responsável. Não foi reconstruído histórico completo de autoria nem realizada busca exaustiva por similaridade.

## Referência documental do parser

Miguel Sanz — attackshark-x11-linux. Copyright (c) 2026 Miguel Sanz. MIT.

- [Documento fixado, seções 4 e 8](https://github.com/MiguelSanzBr/attackshark-x11-linux/blob/e13a99644dcf3538d8cc5908a07832318b49cf2d/docs/PROTOCOL.md)
- [LICENSE do mesmo commit](https://github.com/MiguelSanzBr/attackshark-x11-linux/blob/e13a99644dcf3538d8cc5908a07832318b49cf2d/LICENSE)
- Texto integral preservado: `evidence/protocol-upstream-LICENSE.txt`.

`battery_parser.hpp` já registra a URL fixa. A implementação examinada traduz o prefixo factual `03 55 40`, byte de percentual e limites; não inclui o documento inteiro, código descompilado ou implementação upstream. Essa observação e a declaração do coordenador não certificam cadeia de direitos de todo o histórico. O documento upstream descreve engenharia reversa e declara ausência de código/arte de aplicativo proprietário; não se infere autorização para copiar tais materiais. O aviso MIT foi mantido preventivamente, sem atribuir a Miguel Sanz a autoria do PeriVolt.

Referências GPL mencionadas no contexto do projeto não foram incluídas no recorte como código ou arquivos; inspiração não autoriza copiar implementação. Não há integração Qt nem textos/código Qt no candidato examinado.

## Next.js e CI

Next.js 16.4.6 — Copyright (c) 2025 Vercel, Inc., MIT conforme LICENSE extraído do tarball exato com SHA-512 conferido. O arquivo gerado `dashboard-next/next-env.d.ts` contém referências de tipos; preserva-se o texto integral upstream em `evidence/next-LICENSE.txt`, inclusive por cautela quanto ao pequeno boilerplate gerado. O pacote Next completo e seus componentes compilados não são entregues.

Actions checkout e setup-node: Copyright (c) 2018 GitHub, Inc. and contributors, MIT. São referências remotas por SHA na CI, sem código de Actions copiado; textos consultados em `evidence/checkout-LICENSE.txt` e `evidence/setup-node-LICENSE.txt`.

## caniuse-lite 1.0.30001826 — atribuição preventiva

caniuse-lite, por Ben Briggs (autor na metadata do pacote), projeto Browserslist; dados de compatibilidade do Can I use, por @Fyrd e colaboradores conforme declaração do projeto Can I use. Fontes: [pacote exato](https://registry.npmjs.org/caniuse-lite/1.0.30001826), [Browserslist/caniuse-lite](https://github.com/browserslist/caniuse-lite), [Can I use](https://caniuse.com/), [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).

A licença CC BY 4.0 permite compartilhar/adaptar sob condições de atribuição; não concede endosso. PeriVolt não alterou nem incorporou os dados deste pacote neste candidato. O tarball exato foi lido para verificar licença, sem extrair dados ao candidato; `evidence/caniuse-lite-licenses.json` preserva o texto fornecido.

Se futura distribuição incluir os dados ou derivados, conservar identificação dos criadores e demais avisos efetivamente fornecidos, link à fonte/licença e indicar alterações e adaptações, inclusive minificação/transformação se aplicável. Esta atribuição preventiva não substitui a inspeção dos avisos do artefato redistribuído. A mera referência no lock não redistribui a base nem converte o código próprio em CC BY.

## sharp/libvips

sharp 0.35.5 é Apache-2.0; libvips nos três pacotes de plataforma examinados é 8.18.7. `@img/sharp-libvips-linux-x64` tem versão de pacote 1.3.4; Windows e Wasm usam 0.35.5 e expressões compostas no lock. Não confundir versão do empacotamento com a da biblioteca.

Nenhum desses binários é distribuído. As capturas `evidence/*-licenses.json` guardam licenças/versões dos tarballs examinados, não os binários. Não são um NOTICE completo para futuro bundle: libvips agrega outras bibliotecas, com versões visíveis nos `versions.json`; seus avisos, fontes e condições precisam acompanhar a distribuição concreta quando exigidos.
