# Installer Build Guide

## Prerequisites

- Inno Setup 6 installed at the standard location
  (`C:\Program Files (x86)\Inno Setup 6\ISCC.exe`)
- MSYS2 UCRT64 environment with Qt 6 and Crypto++ installed
- The project must be built first
  (`build/bin/cypherush_client.exe` must exist)

## Build the installer

From the project root in PowerShell:

```powershell
cd installer
.\build_installer.ps1
```

The script will:

1. Verify dependencies are available
2. Stage the client executable
3. Gather Qt dependencies via `windeployqt`
4. Copy MinGW runtime, Crypto++, and ICU libraries
5. Verify all DLLs are bundled or system-provided
6. Compile with Inno Setup

## Output

The installer will be at:
`installer/output/CypherushSetup-v1.0.0.exe`

## After installation

The user must configure the server address once after install:

```powershell
cypherush_client --setup <SERVER_IP>
```

## Testing the installer

Test in a clean Windows VM or a different machine to ensure all
dependencies are bundled. The application should launch without
missing-DLL errors.
