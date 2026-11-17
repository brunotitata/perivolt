# PeriVolt: decodificador experimental X11

Parser original C++20, sem Qt, API de SO, dependencias adicionais ou acesso a
dispositivos. Nao descobre, abre, consulta ou envia comandos HID. Este artefato
**nao e suporte X11 nem leitura real de bateria**.

## Contrato local para integracao posterior

`perivolt::attackshark_x11::parse(std::span<const std::uint8_t>) noexcept`
recebe bytes ja obtidos pelo chamador, incluindo report ID. Retorna percentual
opcional (0 e valido), `Charging::unknown`, motivo tipado e URL de proveniencia
fixada. Nenhum estado anterior e mantido; erro retorna opcional vazio.

O prefixo documentado tem cinco bytes: report ID no indice 0, modelo no 1,
evento no 2, estado no 3 e percentual no 4. Apenas `03 55 40` e aceito.
Modelos SE/PRO sao rejeitados; compartilhar receptor nao comprova protocolo.
Comprimento menor que cinco e truncado, inclusive vazio. Bytes posteriores sao
ignorados: isto e um decoder de prefixo, nao um validador do tamanho HID total,
padding ou checksum. A precedencia dos motivos e tamanho, report, modelo,
evento, faixa. Valores 101..255 sao fora de faixa; **nao ha sentinela documentada**
nessa fonte, portanto 255 nao recebe significado especial.

O estado nao e interpretado: a fonte descreve comportamentos diferentes para
cabo e receptor, e a API deliberadamente nao recebe transporte. Nao inferir
carregamento, conectado, dormindo, firmware ou identidade fisica pelo resultado.
Proveniencia descreve a regra documental, nao a origem dos bytes do chamador.

## Fonte primaria do protocolo comunitario

[PROTOCOL.md, secoes 4 e 8](https://github.com/MiguelSanzBr/attackshark-x11-linux/blob/e13a99644dcf3538d8cc5908a07832318b49cf2d/docs/PROTOCOL.md),
commit `e13a99644dcf3538d8cc5908a07832318b49cf2d`, projeto MIT.
Implementacao original a partir do layout documentado; nenhum codigo de terceiro
copiado. A secao 4 documenta o heartbeat e a faixa; a secao 8 registra uma
observacao de terceiro em hardware. Nao reproduzimos essa observacao.

Contexto local lido: `app/spec/hardware-candidates.md`, `protocol-candidates.md`
e `local-hardware-identification.md`. O Windows observou receptor `1d57:fa60`,
mas a variante exata permanece inconclusiva. Nenhuma validacao de hardware,
macOS ou Linux foi feita nesta entrega.

## Executar testes no Windows

Na raiz do projeto:

```powershell
powershell -NoProfile -File app/adapters/attackshark-x11/run-tests.ps1
```

Usa apenas `tools/w64devkit/bin/g++.exe` existente, C++20 e warnings como erros.
Gera `battery_parser_test.exe` neste diretorio (ignorado pelo Git); nenhum PATH
global e alterado. Para outros compiladores C++20, compilar os dois `.cpp` juntos.

Todos os frames dos testes sao **SINTETICOS**, sem capturas ou eventos de entrada.
Cobrem todos os 256 valores do byte percentual (incluindo 0/100, 101/255), todos
os prefixos truncados, report/modelo/evento incorretos, todos os estados sem
inferir charging e bytes adicionais. Checks usam retorno de processo, nao assert,
e continuam ativos com NDEBUG. O teste nao comprova compatibilidade de dispositivo.
