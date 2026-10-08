param([switch]$Elevated, [string]$TestRoot)
$ErrorActionPreference = 'Stop'
try {
  $support = $PSScriptRoot
  if (!$TestRoot -and !$Elevated) {
    $arguments = '-NoProfile -ExecutionPolicy Bypass -File "' + $PSCommandPath + '" -Elevated'
    $child = Start-Process powershell.exe -Verb RunAs -ArgumentList $arguments -Wait -PassThru
    exit $child.ExitCode
  }
  Write-Host 'Toyland KB-09 installation' -ForegroundColor Cyan
  Write-Host 'Close your music software before continuing.'
  if (!$TestRoot) { Read-Host 'Press Enter when ready' | Out-Null }
  $manifest = Get-Content -LiteralPath (Join-Path $support 'install-files.json') -Raw | ConvertFrom-Json
  foreach ($item in $manifest) {
    $file = Join-Path $support $item.path
    if ((Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash -ne $item.sha256) { throw "Damaged download: $($item.path). Extract the ZIP again." }
  }
  if ($TestRoot) {
    $vstBase = Join-Path $TestRoot 'VST3'
    $appBase = Join-Path $TestRoot 'Toyland/KB-09'
  } else {
    $vstBase = Join-Path ${env:CommonProgramFiles} 'VST3'
    $appBase = Join-Path ${env:ProgramFiles} 'Toyland/KB-09'
  }
  $backup = Join-Path $appBase ('Backups/' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
  $targets = @(@{source='VST3/Toyland KB-09.vst3'; dest=(Join-Path $vstBase 'Toyland KB-09.vst3')}, @{source='Standalone';dest=(Join-Path $appBase 'Standalone')}, @{source='User Guide';dest=(Join-Path $appBase 'User Guide')}, @{source='Licenses';dest=(Join-Path $appBase 'Licenses')}, @{source='Factory Presets';dest=(Join-Path $appBase 'Factory Presets')})
  $changed = @()
  try {
    foreach ($target in $targets) {
      $destination = $target.dest
      $stage = $destination + '.kb09-staging'
      if (Test-Path -LiteralPath $stage) { throw "Previous unfinished installation at $stage. Contact Kevin for help." }
      New-Item -ItemType Directory -Path (Split-Path $destination) -Force | Out-Null
      Copy-Item -LiteralPath (Join-Path $support $target.source) -Destination $stage -Recurse
      $old = Join-Path $backup $target.source
      if (Test-Path -LiteralPath $destination) {
        New-Item -ItemType Directory -Path (Split-Path $old) -Force | Out-Null
        Move-Item -LiteralPath $destination -Destination $old
      }
      $changed += @{dest=$destination;old=$old;stage=$stage}
      Move-Item -LiteralPath $stage -Destination $destination
    }
  } catch {
    [array]::Reverse($changed)
    foreach ($entry in $changed) {
      if (Test-Path -LiteralPath $entry.dest) { Remove-Item -LiteralPath $entry.dest -Recurse -Force }
      if (Test-Path -LiteralPath $entry.old) { Move-Item -LiteralPath $entry.old -Destination $entry.dest }
    }
    throw
  }
  if (!$TestRoot) {
    $menu = Join-Path ([Environment]::GetFolderPath('CommonPrograms')) 'Toyland KB-09'
    New-Item -ItemType Directory -Path $menu -Force | Out-Null
    $shell = New-Object -ComObject WScript.Shell
    foreach ($link in @(@{name='Play KB-09';file='Standalone/Toyland KB-09.exe'}, @{name='KB-09 User Guide';file='User Guide/index.html'})) {
      $shortcut = $shell.CreateShortcut((Join-Path $menu ($link.name + '.lnk')))
      $shortcut.TargetPath = Join-Path $appBase $link.file
      $shortcut.Save()
    }
  }
  Write-Host 'Installed! Open your music software and look for Toyland KB-09 under Instruments.' -ForegroundColor Green
  Write-Host 'To play without music software: Start menu > Toyland KB-09 > Play KB-09.'
  if (!$TestRoot) { Read-Host 'Press Enter to close' | Out-Null }
  exit 0
} catch {
  Write-Host ('Installation could not finish: ' + $_.Exception.Message) -ForegroundColor Red
  Write-Host 'Close your music software and try again. If it still fails, send this message to Kevin.'
  if (!$TestRoot) { Read-Host 'Press Enter to close' | Out-Null }
  exit 1
}
