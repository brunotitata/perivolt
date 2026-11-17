param([ValidateSet('start','stop','restart','health')][string]$Action = 'health')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$state = Join-Path $root '.local'
$pidFile = Join-Path $state 'server.json'
$url = 'http://127.0.0.1:3210'
function Stop-Local {
  if (!(Test-Path $pidFile)) { return }
  $saved = Get-Content $pidFile -Raw | ConvertFrom-Json
  $proc = Get-CimInstance Win32_Process -Filter "ProcessId=$($saved.pid)"
  if ($proc) {
    if ($proc.CommandLine -notlike "*$root*node_modules*next*") { throw 'PID identity mismatch; refusing to stop.' }
    Stop-Process -Id $saved.pid
  }
  Remove-Item -LiteralPath $pidFile
}
if ($Action -eq 'stop' -or $Action -eq 'restart') { Stop-Local; if ($Action -eq 'stop') { return } }
if ($Action -eq 'start' -or $Action -eq 'restart') {
  if (Test-Path $pidFile) { throw 'Server state exists. Use health or stop before start.' }
  if (!(Test-Path (Join-Path $root '.next/BUILD_ID'))) { throw 'Run npm run build first.' }
  if (Get-NetTCPConnection -LocalPort 3210 -State Listen -ErrorAction SilentlyContinue) { throw 'Port 3210 already in use.' }
  New-Item -ItemType Directory -Force $state | Out-Null
  $node = (Get-Command node.exe).Source
  $cli = Join-Path $root 'node_modules/next/dist/bin/next'
  $quotedCli = [char]34 + $cli + [char]34
  $server = Start-Process -FilePath $node -ArgumentList @($quotedCli,'start','--hostname','127.0.0.1','--port','3210') -WorkingDirectory $root -WindowStyle Hidden -RedirectStandardOutput (Join-Path $state 'server.out.log') -RedirectStandardError (Join-Path $state 'server.err.log') -PassThru
  @{pid=$server.Id;url=$url;startedAt=(Get-Date -Format o)} | ConvertTo-Json | Set-Content $pidFile
  Write-Output "PID=$($server.Id) URL=$url"
}
$healthy = $false
for ($attempt=0; $attempt -lt 15; $attempt++) {
  try {
    $response = Invoke-WebRequest $url -UseBasicParsing -TimeoutSec 5
    if ($response.StatusCode -eq 200) { $healthy=$true; break }
  } catch { if ($Action -eq 'health') { throw }; Start-Sleep -Milliseconds 500 }
}
if (!$healthy) { throw 'HTTP health failed. Inspect .local/server.*.log' }
Write-Output "HTTP=$($response.StatusCode) URL=$url"
if (Test-Path $pidFile) { Get-Content $pidFile }

