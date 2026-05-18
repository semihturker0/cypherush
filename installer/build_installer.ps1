# Cypherush - Copyright (C) 2026 semihturker0. All Rights Reserved.
# Proprietary software. See LICENSE for terms.
#
# Automates: stage client exe -> gather Qt/MinGW/Crypto++/ICU deps ->
# verify -> compile Inno Setup installer.
# Run from the installer/ directory.

$ErrorActionPreference = "Stop"
Set-Location -Path $PSScriptRoot

# --- Step 1: Inno Setup ---------------------------------------------------
$iscc = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
if (-not (Test-Path $iscc)) {
    Write-Error "Inno Setup 6 not found at: $iscc"
    exit 1
}
Write-Host "[1/13] Inno Setup found: $iscc" -ForegroundColor Green

# --- Step 2: MSYS2 UCRT64 -------------------------------------------------
$msys2Bin = "C:\msys64\ucrt64\bin"
if (-not (Test-Path $msys2Bin)) {
    Write-Error "MSYS2 UCRT64 not found at: $msys2Bin"
    exit 1
}
Write-Host "[2/13] MSYS2 UCRT64 found: $msys2Bin" -ForegroundColor Green

# --- Step 3: windeployqt --------------------------------------------------
$windeployqt = Join-Path $msys2Bin "windeployqt-qt6.exe"
if (-not (Test-Path $windeployqt)) {
    $windeployqt = Join-Path $msys2Bin "windeployqt6.exe"
    if (-not (Test-Path $windeployqt)) {
        $windeployqt = Join-Path $msys2Bin "windeployqt.exe"
    }
}
if (-not (Test-Path $windeployqt)) {
    Write-Error "windeployqt not found in $msys2Bin"
    exit 1
}
Write-Host "[3/13] windeployqt found: $windeployqt" -ForegroundColor Green

# --- Step 4: Build artifact check ----------------------------------------
$exePath = "..\build\bin\cypherush_client.exe"
if (-not (Test-Path $exePath)) {
    Write-Error "Client exe not found. Run 'cmake --build build' first."
    exit 1
}
Write-Host "[4/13] Client exe found: $exePath" -ForegroundColor Green

# --- Step 5: Reset staging -----------------------------------------------
$staging = "staging"
if (Test-Path $staging) {
    Remove-Item -Recurse -Force $staging
}
New-Item -ItemType Directory -Path $staging | Out-Null
Write-Host "[5/13] Staging directory ready" -ForegroundColor Green

# --- Step 6: Copy main exe -----------------------------------------------
Copy-Item $exePath $staging
Write-Host "[6/13] Copied cypherush_client.exe" -ForegroundColor Green

# --- Step 7: windeployqt --------------------------------------------------
Write-Host "[7/13] Running windeployqt..." -ForegroundColor Cyan
$stagedExe = Join-Path $staging "cypherush_client.exe"
try {
    & $windeployqt --release --no-translations --no-system-d3d-compiler `
                   --no-opengl-sw --compiler-runtime $stagedExe
} catch {
    Write-Warning "windeployqt with --compiler-runtime failed; retrying without it"
    & $windeployqt --release --no-translations --no-system-d3d-compiler `
                   --no-opengl-sw $stagedExe
}
Write-Host "[7/13] windeployqt done" -ForegroundColor Green

# --- Step 8: MinGW UCRT64 runtime ----------------------------------------
$mingwDlls = @(
    "libstdc++-6.dll",
    "libgcc_s_seh-1.dll",
    "libwinpthread-1.dll"
)
foreach ($dll in $mingwDlls) {
    $src = Join-Path $msys2Bin $dll
    if (Test-Path $src) {
        Copy-Item $src $staging
        Write-Host "  Copied: $dll" -ForegroundColor Green
    } else {
        Write-Warning "  Not found: $dll"
    }
}
Write-Host "[8/13] MinGW runtime copied" -ForegroundColor Green

# --- Step 9: Crypto++ -----------------------------------------------------
Get-ChildItem -Path $msys2Bin -Filter "libcrypto*.dll" -ErrorAction SilentlyContinue | ForEach-Object {
    Copy-Item $_.FullName $staging
    Write-Host "  Copied: $($_.Name)" -ForegroundColor Green
}
Get-ChildItem -Path $msys2Bin -Filter "libcryptopp*.dll" -ErrorAction SilentlyContinue | ForEach-Object {
    Copy-Item $_.FullName $staging
    Write-Host "  Copied: $($_.Name)" -ForegroundColor Green
}
Write-Host "[9/13] Crypto++ libraries copied" -ForegroundColor Green

# --- Step 10: ICU ---------------------------------------------------------
foreach ($pat in @("icudt*.dll", "icuin*.dll", "icuuc*.dll", "libicu*.dll")) {
    Get-ChildItem -Path $msys2Bin -Filter $pat -ErrorAction SilentlyContinue | ForEach-Object {
        Copy-Item $_.FullName $staging
        Write-Host "  Copied: $($_.Name)" -ForegroundColor Green
    }
}
Write-Host "[10/13] ICU libraries copied" -ForegroundColor Green

# --- Step 11: Dependency verification ------------------------------------
$objdump = Join-Path $msys2Bin "objdump.exe"
if (Test-Path $objdump) {
    $deps = & $objdump -p $stagedExe | Select-String "DLL Name:"
    Write-Host "Required DLLs:" -ForegroundColor Cyan
    foreach ($dep in $deps) {
        $dllName = ($dep -replace ".*DLL Name:\s*", "").Trim()
        $found = Test-Path (Join-Path $staging $dllName)
        $systemDll = Test-Path "C:\Windows\System32\$dllName"
        if ($found) {
            Write-Host "  [OK] $dllName (bundled)" -ForegroundColor Green
        } elseif ($systemDll) {
            Write-Host "  [SYS] $dllName (Windows system)" -ForegroundColor Gray
        } else {
            Write-Host "  [MISSING] $dllName" -ForegroundColor Red
        }
    }
}
Write-Host "[11/13] Dependency check complete" -ForegroundColor Green

# --- Step 12: Compile installer ------------------------------------------
Write-Host "[12/13] Compiling installer..." -ForegroundColor Cyan
& $iscc "Cypherush.iss"
if ($LASTEXITCODE -ne 0) {
    Write-Error "Inno Setup compilation failed"
    exit 1
}

# --- Step 13: Report ------------------------------------------------------
$output = Get-ChildItem -Path "output" -Filter "*.exe" | Select-Object -First 1
if ($output) {
    $sizeMB = [math]::Round($output.Length / 1MB, 2)
    Write-Host ""
    Write-Host "=================================" -ForegroundColor Green
    Write-Host "SUCCESS: Installer built" -ForegroundColor Green
    Write-Host "  Path:    $($output.FullName)" -ForegroundColor White
    Write-Host "  Size:    $sizeMB MB" -ForegroundColor White
    Write-Host "=================================" -ForegroundColor Green
}
