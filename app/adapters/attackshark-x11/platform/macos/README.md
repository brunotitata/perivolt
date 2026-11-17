# PeriVolt: primeira fatia experimental macOS Apple Silicon

Implementado: descoberta **somente de metadados** via IORegistry e fronteira
portavel de transporte **somente FAKE**, ligada ao parser C++20 ja aceito.
Sem framework novo, Qt, SDK instalado, driver, configuracao global ou alteracao
de core/Backend/Next. Linux fica para incremento posterior.

## Runtime macOS: apenas discovery

`discovery_macos.cpp` enumera IOHIDDevice com IOServiceMatching,
IOServiceGetMatchingServices e IOIteratorNext. Consulta com
IORegistryEntryCreateCFProperty somente VendorID/ProductID e, para 1d57:fa60,
PrimaryUsagePage/PrimaryUsage/MaxInputReportSize. Libera propriedades CF e
objetos IOKit por RAII. Limita enumeracao a 512 entradas e prazo cooperativo
de dois segundos entre chamadas; nao e garantia de tempo real contra API
bloqueada. Falha/limite/iterador invalidado nao comprova ausencia de dispositivo.

Nao abre dispositivo/colecoes, registra callbacks, solicita permissoes do SO,
le reports, coleta elementos de entrada, usa event taps, escreve Feature ou
Output, envia comandos, emparelha ou altera firmware. Sem serial, MAC, nome,
registry ID, caminho de dispositivo, report descriptor bruto ou payload na saida.

`discovery_main.cpp` produz JSON com fonte, horario recebido UTC em Unix ms,
observed_at null, percentual null, charging/freshness unknown e live false.
Propriedades ausentes permanecem null. O retorno 0 significa enumeracao
completa, nao bateria/suporte; 2 indica incompletude ou argumentos nao aceitos.
O comando nao oferece opcao de leitura. Candidato de metadados 0x0A/0/input5
nao significa colecao exclusiva de bateria nem identidade fisica confirmada.

**Limite de privacidade:** a Apple adverte que PrimaryUsage/Page nao descrevem
todas as application collections de um dispositivo. Nao inferimos exclusao de
teclado/mouse por esses campos. A API de callback de report nao documenta filtro
previo do evento proprietario 0x40. O endpoint multiplexado X11 permanece sem
prova de exclusividade. Por isso **nenhum transporte nativo de report foi ligado**.

## Consentimento e fronteira delimitada

`Consent` comeca false e representa consentimento explicito para leitura de
reports; nao e uma permissao TCC da Apple e nao solicita/contorna permissoes.
`native_read_gate` devolve consent_required sem consentimento; mesmo com ele,
devolve native_validation_required. Nao ha caminho runtime que habilite leitura.
Validacao especifica de endpoint, framing de callbacks, privacidade,
cancelamento e build/hardware macOS e requisito separado do consentimento.
A autorizacao unica Windows foi consumida e nao e reaproveitada aqui.

`run_fake_session` aceita apenas tipo de transporte marcado synthetic_transport.
E uma fronteira preparada/testavel, **nao implementacao de IOHIDDevice**. Exige
consentimento novo, metadados alvo exatos, unicidade e validacao explicitamente
simuladas. Consome o consentimento antes de iniciar; segunda chamada nao le.
Fixa prazo monotonic de 8s e teto 32 reports, scratch de 5 bytes apagado ao sair.
Tempo, cancelamento e erro sao fornecidos pelo FAKE; cancel_close e chamado por
RAII em cada saida apos inicio. O transporte deve parar qualquer acesso ao
buffer antes de retornar de read. Nao ha garantia testada de cancelamento IOKit.

O contrato de frame FAKE e **5 bytes canonicos incluindo report ID**. Isso nao
afirma se o buffer Apple inclui o ID; essa normalizacao nao foi implementada.
Length diferente de 5, ID diferente de 03 ou modelo diferente de 55 abortam.
O parser existente aceita apenas evento 40/percent 0..100; outros eventos ou
percentuais invalidos sao descartados ate teto/prazo. Nao ha retencao de raw.

## Observacao local para integracao posterior

`Observation` nao substitui o contrato de dominio; e um DTO folha deste modulo:
percent optional; charging unknown; origem metadata/synthetic/cache; motivo;
source de aquisicao; URL fixa do protocolo; received_at wall time;
observed_at opcional; freshness unknown e live false. Horario recebido nao
comprova horario da medicao ou confianca de relogio. O dominio devera acrescentar
identidade/epoca/geracao/associacao e decidir frescor conforme seu contrato.

`as_cache` preserva percentual, timestamp de observacao e proveniencia; muda
origem para cache e registra a nova recepcao, sempre freshness unknown/live false.
Nao carrega arquivos, nao importa a amostra Windows de 50%, nao promove cache
para leitura atual e nao gera alertas. Testes usam apenas valores sinteticos.

## Verificacao executada no Windows, somente C++ portavel

Na raiz do projeto:

```powershell
powershell -NoProfile -File app/adapters/attackshark-x11/platform/macos/run-fake-tests.ps1
```

Usa o w64devkit existente, compila adapter_test.cpp + battery_parser.cpp com
C++20 e -Wall -Wextra -Wpedantic -Werror. Nao compila/executa as unidades
macOS nem o candidato Windows. Arquivo gerado: adapter_fake_test.exe (ignorado).

RED observado com stub: 35 checks/30 falhas, exit 1. GREEN: 35 checks/0 falhas,
compile/test exit 0. Cobertura: gates sem leitura, consentimento de uso unico,
0/100, proveniencia/cache sem frescor, lengths invalidos, ID/modelo inesperado,
eventos 10/60/70/80, percent101/255, teto32, timeout com cancelamento pendente
FAKE, cancelamento anterior, erro e resposta no prazo expirado.

## Build macOS preparado para DevOps, NAO EXECUTADO

Somente num Mac arm64 com Apple Command Line Tools/SDK **ja disponiveis**,
na raiz do projeto, build local sem executar o binario:

```sh
clang++ -std=c++20 -arch arm64 -mmacosx-version-min=12.0 -Wall -Wextra -Wpedantic -Werror app/adapters/attackshark-x11/platform/macos/discovery_macos.cpp app/adapters/attackshark-x11/platform/macos/discovery_main.cpp -framework IOKit -framework CoreFoundation -o app/adapters/attackshark-x11/platform/macos/x11-macos-metadata
```

Kernels/SDKs antigos sem kIOMainPortDefault nao sao alvo desta fatia; macOS12+
e alvo de compilacao proposto, nao compatibilidade verificada. Nao instalamos SDK,
nao executamos cross-build com headers simulados, nao acessamos Mac remotamente,
nao disparamos GitHub/CI nem publicamos. DevOps foi avisado dos arquivos/comando.
**Compilacao nativa macOS e teste em hardware nao realizados.** Testes portaveis
nao comprovam nem a disponibilidade das propriedades no dispositivo real.

## Fontes Apple oficiais consultadas

Documentacao/sources consultados apenas como API, sem copiar implementacao:

- [IOKitLib.h oficial](https://github.com/apple-oss-distributions/IOKitUser/blob/323ead896d04424f87184d8f6ff0cce811aab106/IOKitLib.h): descoberta, propriedades e ownership.
- [IOHIDDevice.h oficial](https://github.com/apple-oss-distributions/IOKitUser/blob/323ead896d04424f87184d8f6ff0cce811aab106/hid.subproj/IOHIDDevice.h): contrato de open/callback/report, nao chamado por esta implementacao.
- [IOHIDKeys.h oficial](https://github.com/apple-oss-distributions/IOHIDFamily/blob/777ccd9698845aadf711e32d843c8c9b777431d9/IOHIDFamily/IOHIDKeys.h): limites de PrimaryUsage/Page e usos multiplos.
- [IOHIDDeviceKeys.h oficial](https://github.com/apple-oss-distributions/IOHIDFamily/blob/777ccd9698845aadf711e32d843c8c9b777431d9/IOHIDFamily/IOHIDDeviceKeys.h): nomes das propriedades numericas usadas.

Os headers oficiais estao sob APSL; nenhum arquivo Apple foi incorporado ao
projeto. O build futuro inclui headers do SDK existente. Fonte do protocolo
comunitario MIT e a mesma fixada no parser, sem ampliar sua compatibilidade.
