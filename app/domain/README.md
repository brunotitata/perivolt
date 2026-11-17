# PeriVolt F0b - terceira fatia: primeiro grupo fonte/permissao/tempo

API C++20 `battery.hpp` / `battery.cpp`, namespace `perivolt`. Somente dominio: sem Qt, SO, HID, I/O, relogio global ou dependencia externa. Duas fatias anteriores aceitas como subconjunto por QA em `qa-review.md`; este grupo novo ainda depende de revisao independente. Nao e F0b/MVP completo.

## API implementada

- `Battery(ttl_ms)`, `observe`, `project`: nivel percentual inteiro finito 0..100 sem clamp; invalid/unavailable/qualitativo; frescor separado por campo, TTL inclusivo, cache sem renovar, desconexao com valor retido. `Level.percent` so tem significado para kind=reported_percent.
- `register_device(Device)`: registra metadados declarados de um ID canonico (confidence, parent, slot). Nao resolve serial, nao associa por nome/VID/PID/endpoint, nao funde dispositivos. Registro conflitante do mesmo ID e recusado; registro identico e idempotente. Endpoint vem da observacao e pode mudar mantendo ID. Cada `(device,component)` tem estado independente; slots de aparelhos distintos nao compartilham estado mesmo com endpoint igual.
- `remove(source,device,component,generation,source_epoch)`: grava tombstone sem apagar barreira de geracao. Igual/menor e rejeitado; maior pode reabrir. Sem source_epoch explicita usa somente epoca ja conhecida; antes da primeira observacao e obrigatorio informar epoca. Evento de remocao atrasado de epoca antiga e recusado quando traz epoca explicita. Remocao sem epoca e um evento confiavel da epoca ATUAL; caller deve incluir epoca em eventos assincronos.
- `change_source_epoch(source,old,new)`: transicao explicita; exige old corrente. Valor anterior permanece historico com frescor unknown. Observacao da nova epoca reinicia geracao por componente, inclusive 0; respostas de epocas antigas nao substituem estado. Tombstone de epoca anterior nao bloqueia nova epoca. Nao infere mudanca pelo timestamp.
- `sleep(now_ms)` invalida frescor de todos os componentes e tentativas pendentes de entrega. `resume(now_ms)` nao renova observacoes. Nova observacao so recupera frescor se nao estiver dormindo e tiver timestamp de observacao >= ultima fronteira de sono/resume; resposta com geracao maior mas timestamp anterior continua invalidada. Tempos sao monotônicos fornecidos pelo chamador na mesma epoca; evento global de mudanca de epoca monotônica ainda nao implementado.
- `State.rejected_ids`: diagnostico de observacoes rejeitadas por geracao/epoca para chaves conhecidas. Chave desconhecida recebida numa epoca nao corrente e rejeitada sem criar registro nem identidade fantasma.
- `source_permission(source, granted|denied)` registra evento de permissao da FONTE, separado da permissao de notificacao. Negacao invalida a leitura anterior e cancela tentativa pendente, sem afirmar desconexao. Grant nao renova cache: nova observacao aceita e necessaria. Repetir mesmo estado e idempotente; episodio entregue nao e limpo. Estados nao informados significam apenas ausencia de negacao explicita no core tipado, nao comprovacao de permissao do SO. O chamador e responsavel por enviar eventos reais.
- `Observation.charging_observed_ms` / `connection_observed_ms` opcionais permitem base temporal REAL independente por campo, na mesma monotonic_epoch da observacao. Ausencia usa a base comum observed_ms, sem inventar timestamp de recebimento. Sem ambas, campo unknown. Cada base respeita TTL, epoch e cutoff de sono. O harness NAO sintetiza esses campos a partir de expectativas.
- Versao incompat?vel precede interpretacao do nivel: invalid/unsupported_version; carga/conexao do payload ficam unknown. Diagnostico invalido explicito de versao suportada e preservado. Geracao/epoch rejeitadas sao tratadas ANTES do payload e nao sobrescrevem o estado.
- `configure` / `set_threshold` mudam parametros sem avaliar retroativo. `evaluate(State)` aplica politica somente quando chamado pelo consumidor numa leitura elegivel. `delivery(State,outcome)` recebe resultado externo SINCRONO; apenas delivered com tentativa pendente marca episodio. `episode` / `restore_episode` permitem ao futuro caller persistir/restaurar flag. Nao existe persistencia/serializacao neste modulo.
- Remocao, reconexao, sleep/resume e mudanca de epoca NAO limpam episodio entregue. Rearme somente por leitura elegivel >= min(limiar+histerese,100). Entrega assincrona correlacionada por token NAO implementada: nao entregar resultado antigo apos observe/evaluate/lifecycle; mudancas lifecycle cancelam pending.
- Precedencia atual de politica: policy_pending, ineligible, alerts_disabled, permission_denied, rearme, limiar/histerese, charging, already_notified. P2-05 do contrato ainda precisa ratificacao global (carga na faixa intermediaria); esta fatia nao cria politica nova.

`observed_ms` e instante REAL de observacao monotônica. Adaptador so usa recebimento quando receipt_is_observation=true. Cache consulta project, nao observe como dado novo. Sem timestamp/epoca comparavel, frescor unknown; idade <= TTL fresh, > TTL stale. Fonte diferente por chave e excecao explicita: multi-fonte continua unsupported, nunca ignorado/mesclado.

## Executar e resultado real (17/11/2026)

Da raiz do projeto:

```powershell
python app/domain/tests/run_fixtures.py
```

Harness usa somente JSON da biblioteca padrao Python para traduzir as fixtures ORIGINAIS em chamadas C++ e asserts literais. Codigo real `battery.cpp` e linkado aos testes. Nao implementa regras de dominio nem altera as fixtures/flags historicos delas; evento nao suportado interrompe a geracao. Entrega de notificacao e resultado externo explicitamente SIMULADO, sem notificacao OS.

Toolchain existente `tools/w64devkit/bin/g++.exe`; `-std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror`. Sem novos downloads/dependencias/instalacoes.

- F08/F10/F11 e 13 regress?es PASS nos campos selecionados. F10 can?nica corrigida pelo TechLead (somente expectativas/rationale); removida toda exce??o DIVERGENCE do runner. Compara??o direta de freshness.connection=unknown em e2/e4; core inalterado.
- Execu??o estrita: 16 fixtures SINT?TICAS, 71 checkpoints, 1065 compara??es can?nicas de campos selecionados. compile_exit=0 / run_exit=0. N?o significa conformidade integral das 16 fixtures; campos n?o implementados continuam descritos abaixo.
- Regressao: 26 checks diretos anteriores e 36 lifecycle PASS; compilacoes/execucoes exit 0.
- Novo `tests/source_time_test.cpp`: 31 checks PASS, compilacao/execucao exit 0. Negacao/grant sem renovar cache, isolamento de fonte/episodio, versao2 com -1/255/NaN preserva unsupported_version, diagnostico explicito suportado, geracao rejeitada nao sobrescreve, conexao com base temporal independente nao renova nivel, TTL/epoch, fonte sem tempo, sono por campo e ausencia com episodio salvo/ineligible.
- Nova regressao falhou antes do ajuste: nivel pos-resume com conexao de timestamp pre-resume tratava conexao como stale em vez de unknown; corrigido cutoff por campo. Reexecutado comando completo acima com exit 0. Nenhuma entrada wall-clock e usada no nucleo; runner reconhece clock_changed como evento sem efeito monot?nico, nao recalcula idade pela parede.

Campos comparados nas fixtures visitadas: present; level.kind/value/reason; charging/connection; frescor dos tres campos; alert_eligible/suppression; metadados identity_confidence/endpoint/parent/slot; rejected_observation_ids quando fornecidos em expected; requested/blocked_reason/rearm_applied e episode_notified apos resultado de entrega simulado. Campos de estado ausente comparados somente quando declarados na fixture.

NAO comparados/implementados: candidates/conflict/uncertainty, associacao multi-fonte, delivery como projecao completa. F01 valida preservacao dos IDs DECLARADOS e troca de endpoint, nao um resolvedor de identidade a partir de serial. Portanto nao se declara conformidade integral das fixtures visitadas.

## Pendentes e limites

F09/F14/F15/F19 NAO executadas; associacao/candidatos/conflito/uncertainty e politica de intervalo sao a proxima fatia, seguindo adendo P2. Esta entrega e o PRIMEIRO GRUPO funcional autorizado (F08/F10/F11), nao fechamento de todo o restante. F10 n?o tem mais diverg?ncia nos campos selecionados ap?s corre??o can?nica do TechLead; core continua sem produzir frescor sem base.

Restantes: identidade derivada/associacoes, conflitos/tolerancia/candidates, eventos parciais por campo (sem nova observacao de nivel), epoca monotônica global, persistencia e entrega assincrona correlacionada. P2-01 intervalo versus alerta permanece fora do escopo (multi-fonte recusada); P2-02 teste direto preserva episodio em stale/invalido e lifecycle, sem conflito; P2-03 ausencia aceita metadados de politica e jamais alerta: teste direto com politica ratificada confirma ineligible/flag preservado; P2-04 multi-fonte nao implementada. Nenhuma lacuna e apresentada como MVP pronto.

Executado apenas Windows/w64devkit. Sem hardware, macOS/Linux, Qt, HID ou declaracao de compatibilidade. Nenhuma alteracao em Next, Hardware/adapters, fixtures/spec, tabela ou Git/index/branch. Artefatos locais gerados em `app/domain/build/*.cpp|exe`; remover somente esta subpasta para limpar. Hashes das fontes e contrato lido em hashes.txt.
