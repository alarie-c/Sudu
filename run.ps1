param (
    [switch]$c
)

if (-not (Test-Path -Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

Set-Location -Path "build"

if ($c) {
    Write-Host "Compiling..."
    cmake -G "MinGW Makefiles" .. ; make
    Write-Host "Compilation complete."
}

Start-Process -NoNewWindow -Wait -FilePath "./cladis.exe"
