# PeriVolt — protótipo

Fontes atuais do core C++20, parser experimental X11, UI Qt Widgets e discovery macOS. O entrypoint ainda não liga produtor real à GUI; não há MVP de bateria funcional, instalador ou compatibilidade de hardware certificada.

Dashboard Next.js em dashboard-next é separado do monitor nativo. Não contém dados operacionais. Node24: npm ci, testes existentes, npm run build e npm run typecheck. Consulte dashboard-next/README.md.

Core sem Qt: cmake -S app -B build -DBUILD_TESTING=ON -DPERIVOLT_BUILD_UI=OFF; cmake --build build; ctest --test-dir build --output-on-failure. UI exige SDK Qt6 Widgets; falha se ausente. CI macOS usa SDK Qt6.12.0 público, alvo mínimo de build 14.4, arquitetura observada no runner. Isso não comprova execução no Mac do usuário nem hardware. Consulte app/build-support/MACOS-DELIVERY.md.

MIT e copyright brunotitata aprovados para código próprio; Qt dinâmico LGPL autorizado. Veja LICENSE, NOTICE e legal/APPROVALS.md. Distribuição binária ainda exige inventário/avisos/fontes correspondentes/substituição Qt e integração real. Nenhum DMG ou release de MVP publicado por este import.

Origem e delta: docs/import/current-files.json; fontes recentes preservadas, sem restaurar snapshot congelado, dados privados, diagnósticos HID, tools, builds ou node_modules.
