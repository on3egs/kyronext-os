# Test QEMU isolé : fichiers ISO/firmware uniquement, aucun disque physique.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$IsoPath,
    [ValidateSet('uefi', 'bios')][string]$Mode = 'uefi',
    [ValidateSet('std', 'virtio')][string]$Video = 'std',
    [string]$QemuPath,
    [string]$OvmfCodePath,
    [string]$OvmfVarsPath,
    [string]$ManifestPath,
    [ValidateSet('tcg', 'whpx')][string]$Acceleration = 'whpx',
    [ValidateRange(0, 65535)][int]$HostSshPort = 0
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot

function Get-LocalRegularFile([string]$Path) {
    # Refuser notamment \\.\PhysicalDriveN, \\?\Volume{}, UNC et chemins de périphériques.
    if ($Path -notmatch '^[A-Za-z]:[\\/]' -or $Path.Contains(',')) {
        throw "Un chemin absolu local de fichier, sans virgule, est requis : $Path"
    }
    $item = Get-Item -LiteralPath $Path -Force
    if ($item.PSProvider.Name -ne 'FileSystem' -or $item.PSIsContainer -or
        ($item.Attributes -band [IO.FileAttributes]::ReparsePoint)) {
        throw "Fichier régulier requis (pas de dossier ou lien) : $Path"
    }
    return $item.FullName
}

$iso = Get-LocalRegularFile $IsoPath
if ([IO.Path]::GetExtension($iso) -ine '.iso' -or (Get-Item -LiteralPath $iso).Length -eq 0) {
    throw 'Une image ISO non vide est requise.'
}
if (-not $QemuPath) {
    $installedQemu = 'C:\Program Files\qemu\qemu-system-x86_64.exe'
    $portableQemu = Join-Path $projectRoot 'build\tools\qemu\qemu-system-x86_64.exe'
    $QemuPath = if (Test-Path -LiteralPath $installedQemu -PathType Leaf) { $installedQemu } else { $portableQemu }
}
$qemu = Get-LocalRegularFile $QemuPath
if (-not $ManifestPath) { $ManifestPath = Join-Path (Split-Path -Parent $iso) 'SHA256SUMS' }
$manifest = Get-LocalRegularFile $ManifestPath
$isoName = [IO.Path]::GetFileName($iso)
$matchingHashes = @(
    foreach ($line in Get-Content -LiteralPath $manifest) {
        if ($line -match '^([A-Fa-f0-9]{64})[ \t]+\*?(.+)$') {
            if ([string]::Equals($Matches[2], $isoName, [StringComparison]::Ordinal)) {
                $Matches[1]
            }
        }
    }
)
if ($matchingHashes.Count -ne 1) { throw 'ISO absente du manifeste SHA-256 ou entrée ambiguë.' }
$hash = (Get-FileHash -LiteralPath $iso -Algorithm SHA256).Hash.ToLowerInvariant()
if ($hash -ine $matchingHashes[0]) { throw 'Empreinte SHA-256 incorrecte : test refusé.' }

$port = if ($Mode -eq 'uefi') { 4444 } else { 4445 }
$listener = [Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback, $port)
try { $listener.Start() } catch { throw "Port QMP local $port occupé : aucune nouvelle instance lancée." }
finally { $listener.Stop() }
if ($HostSshPort -ne 0) {
    if ($HostSshPort -lt 1024) { throw 'Le port SSH local doit être supérieur ou égal à 1024.' }
    $sshListener = [Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback, $HostSshPort)
    try { $sshListener.Start() } catch { throw "Port SSH local $HostSshPort occupé : aucune nouvelle instance lancée." }
    finally { $sshListener.Stop() }
}

$stamp = (Get-Date).ToUniversalTime().ToString('yyyyMMddTHHmmssfffZ')
$run = Join-Path $projectRoot "logs\test-windows-$Mode-$stamp"
if ($run.Contains(',')) { throw 'Le chemin du projet ne doit pas contenir de virgule pour QEMU.' }
New-Item -ItemType Directory -Path $run | Out-Null
$accelerationArgument = if ($Acceleration -eq 'whpx') { 'whpx' } else { 'tcg,thread=single' }
$nicArgument = 'user,model=virtio-net-pci'
if ($HostSshPort -ne 0) {
    $nicArgument += ",hostfwd=tcp:127.0.0.1:$HostSshPort-:22"
}
$arguments = @('-machine', 'q35', '-cpu', 'qemu64', '-accel', $accelerationArgument,
               '-m', '3072', '-smp', '1', '-display', 'none',
               '-qmp', "tcp:127.0.0.1:$port,server=on,wait=off",
               '-serial', "file:$(Join-Path $run 'serial.log')",
               '-cdrom', $iso, '-boot', 'd', '-nic', $nicArgument, '-vga', $Video)
if ($Mode -eq 'uefi') {
    if (-not $OvmfCodePath) { $OvmfCodePath = Join-Path $projectRoot 'build\builder\OVMF_CODE_4M.fd' }
    if (-not $OvmfVarsPath) { $OvmfVarsPath = Join-Path $projectRoot 'build\builder\OVMF_VARS_4M.fd' }
    $code = Get-LocalRegularFile $OvmfCodePath
    $vars = Get-LocalRegularFile $OvmfVarsPath
    $varsCopy = Join-Path $run 'vars.fd'
    Copy-Item -LiteralPath $vars -Destination $varsCopy
    $arguments += @('-drive', "if=pflash,format=raw,readonly=on,file=$code",
                    '-drive', "if=pflash,format=raw,file=$varsCopy")
}

# Start-Process joint ArgumentList en une chaîne : protéger chaque argument.
$quotedArguments = @(
    foreach ($argument in $arguments) {
        if ($argument.Contains('"')) { throw 'Guillemet interdit dans un argument QEMU.' }
        '"' + $argument + '"'
    }
)
$stdout = Join-Path $run 'qemu.stdout.log'
$stderr = Join-Path $run 'qemu.stderr.log'
$process = Start-Process -FilePath $qemu -ArgumentList $quotedArguments -WindowStyle Hidden `
    -RedirectStandardOutput $stdout -RedirectStandardError $stderr -PassThru
$process.Id | Set-Content -LiteralPath (Join-Path $run 'qemu.pid') -Encoding ascii
$sshForward = if ($HostSshPort -eq 0) { $null } else { "127.0.0.1:$HostSshPort -> guest:22" }
$record = [ordered]@{
    started_utc = $stamp; pid = $process.Id; mode = $Mode; video = $Video; iso = $iso; sha256 = $hash
    qemu = $qemu; acceleration = $Acceleration; host_ssh_forward = $sshForward
    arguments = $arguments; qmp = "tcp:127.0.0.1:$port"
    status = 'Processus lancé ; démarrage et bureau non encore validés.'
}
$record | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $run 'launch.json') -Encoding utf8
Write-Output "QEMU PID $($process.Id) — preuves : $run"
Write-Output "QMP : tcp:127.0.0.1:$port"
if ($HostSshPort -ne 0) { Write-Output "SSH invité (clé requise, local uniquement) : 127.0.0.1:$HostSshPort" }
