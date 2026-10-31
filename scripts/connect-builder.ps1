# Accès uniquement au constructeur dédié créé pour ce projet.
param([string]$Command)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$builderRoot = Join-Path $projectRoot 'build\builder'
$keyPath = Join-Path $builderRoot 'id_ed25519'
$knownHosts = Join-Path $builderRoot 'known_hosts'
if (-not (Test-Path -LiteralPath $keyPath)) { throw 'Clé du constructeur absente.' }
$arguments = @('-o', "UserKnownHostsFile=$knownHosts", '-i', $keyPath, '-p', '2222', 'builder@127.0.0.1')
if ($Command) { $arguments += $Command }
& ssh @arguments
exit $LASTEXITCODE
