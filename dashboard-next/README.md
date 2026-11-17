# PeriVolt dashboard Next.js local
Requer Node 24.x e npm. Dependencias exatas em package-lock.json.
Instalar: npm ci
Desenvolvimento (loopback): npm run dev
Producao local: npm run build e npm run local:start
URL: http://127.0.0.1:3210
Verificar HTTP da pagina: npm run local:health
Parar: npm run local:stop
Reiniciar o build existente: npm run local:restart
Depois de mudar codigo: parar, npm run build, iniciar novamente.
PID/data/URL: .local/server.json
Logs: .local/server.out.log e .local/server.err.log
Processo Node em segundo plano (Start-Process Hidden); independe do terminal.
O health operacional confirma HTTP 200 da pagina; aceite funcional pertence a QA.
npm run typecheck verifica TypeScript.
Scripts start/dev limitam binding a 127.0.0.1:3210; sem LAN/tunel.
Nao editar app/components/server pelo bootstrap. Legado ../dashboard preservado.
Versoes conferidas no npm: Next 16.4.6, React 19.3.1, TypeScript 5.9.3.
Fonte dos requisitos Next: https://nextjs.org/docs/app/getting-started/installation
