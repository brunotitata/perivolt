# PeriVolt — contrato mínimo do dashboard-next

**Status:** decisão Tech Lead para implementação paralela. **Módulo novo:** `dashboard-next/`; `dashboard/` legado preservado. **Fonte:** tabela OpenBot `peripheral_battery_tasks`, projeto `PeripheralBatteryMonitor`; somente leitura. **Next.js:** versão estável compatível com Node 24 fixada por DevOps.

## Dados

Reutilizar `dashboard-snapshot@1` e o exportador existente. O export atual tem 32 linhas e mojibake; a tabela tem 33. **Não usar o arquivo congelado como verdade**: P.O gera novo snapshot e corrige UTF-8. `project.id = "peripheral-battery-monitor"`; `project.name = "PeriVolt"`; filtro da tabela usa `project = "PeripheralBatteryMonitor"`.

`tasks[]` mínimo: `task_code`, `title`, `area`, `phase`, `priority`, `status_raw`, `status_normalized`, `owner_code`, `depends_on_text`, `requirements_text`, `acceptance`, `evidence_text`, `external_refs`. `owner_map`/`status_map` vêm no snapshot; sem mapa, código cru/`unknown`, nunca heurística. O dashboard não escreve na tabela.

## API

Somente leitura, `Cache-Control: no-store`; sem DB, LLM ou credencial. Tipos: `dashboard-next/shared/api-types.ts`.

| Rota | Contrato |
| --- | --- |
| `GET /api/snapshot` | `200 SnapshotResponse`; `503 {"ok":false,"error":"snapshot_missing"|"snapshot_invalid","served_at":"..."}` |
| `GET /api/health` | `200 {"ok":true,"snapshot":{"version","observed_at","age_seconds","stale"},"source":"file"}` |
| `GET /api/stream` | opcional SSE `event: snapshot` com `{version,observed_at}` + heartbeat 20 s; polling 2 s é o caminho inicial |

`SnapshotResponse` = `ok:true`, `schema:"dashboard-snapshot@1"`, `version`/`digest` (mesmo hash), `generated_at`, `observed_at`, `served_at`, `age_seconds`, `stale`, `coverage`, `stale_after_seconds`, `project`, `source`, `owner_map`, `status_map`, `tasks[]`, `evidence[]`.

Backend lê `dashboard-next/data/snapshot.json`; ausente/inválido devolve 503 e a UI mantém o último estado bom. `age_seconds`/`stale` calculados no servidor.

## Ponte e atualização

Sem API HTTP programática documentada do OpenBot, não acessar SQLite/banco interno nem inventar endpoint de escrita. Ponte por evento de orquestração, dona P.O:

1. P.O/agente altera a tabela pelas ferramentas OpenBot.
2. No mesmo fluxo, publica `dashboard-next/data/snapshot.json` por escrita atômica (`tmp` + rename), com `observed_at` da leitura.
3. Backend serve o novo snapshot; Frontend faz polling de 2 s e atualiza sem refresh/importação manual do usuário.

**Limitação:** não é push do OpenBot. Se o P.O não publicar, a UI mostra idade/`stale` e a última observação, nunca “tempo real”. Zero-toque exigiria API/evento documentado do OpenBot, inexistente nesta etapa.

## Frontend

App Router em `app/{page,layout,globals.css}`, `components/**`; consome só a API. Polling 2 s com `AbortController`, preserva o último snapshot bom em erro/503; exibe fonte, `observed_at`, idade, `stale`, cobertura e erro. Sem fixtures na tela operacional, sem escrita/importação, sem refresh obrigatório. Links externos só `http(s)`; GitHub público opcional não bloqueia.

## Ownership

| Dono | Responsabilidade |
| --- | --- |
| P.O | Tabela/estados/backlog e publicação do snapshot; corrigir UTF-8 e regenerar com a tabela real |
| Backend | `app/api/**`, leitura/validação e envelope/erro |
| Frontend | `app/{page,layout,globals.css}`, `components/**`, polling e estados |
| DevOps | versão Next/Node 24, `package.json/config`, scripts, execução/parada, porta local |
| QA | testes independentes e E2E; não implementa |
| Tech Lead | contrato, integração e revisão curta; sem gate por arquivo |

## Aceite E2E

Alteração real e rastreável na tabela + publicação do P.O deve aparecer em `/api/snapshot` e na UI em até 2 s, sem refresh/importação manual, com `observed_at`, `version` e idade novos. Falha de publicação mantém o último estado e mostra `stale`; nunca zero/sucesso inventado. GitHub não é requisito.

**Riscos impeditivos:** ponte depende do P.O, não é push; export atual congelado/desatualizado e com mojibake; versão Next/scripts são DevOps; sem servidor rodando não há atualização automática.
