# Cypherush

**End-to-End Encrypted Messaging**

> _"Speak in cipher, hear in hush."_

## What's in a name?

**Cypherush** is a three-layered word blend:

- **Cypher** — _cipher_, the encryption at the core of every message.
- **Hush** — _silence_, the privacy guaranteed by end-to-end encryption.
- **Rush** — _flow_, the fast, real-time delivery of messages.

Together: secrets that travel quickly and stay quiet.

## Technologies

- **C++17**
- **Qt 6** (Core, Network, Widgets)
- **Crypto++** (AES, RSA, hashing)
- **CMake** + **Ninja**
- **MSYS2 / MinGW** (UCRT64 toolchain) on Windows

## Architecture

| Component           | Description                                            |
| ------------------- | ------------------------------------------------------ |
| `cypherush_common`  | Shared static library: models, crypto, data, network.  |
| `cypherush_server`  | Headless relay server (`QCoreApplication`, `QTcpServer`). |
| `cypherush_client`  | Desktop GUI client (`QApplication`, `QTcpSocket`).     |

## Build

From the project root:

```sh
cmake -B build -G Ninja
cmake --build build
```

All executables are written to `build/bin/`.

## Run

Start the server first, then launch two clients to chat between them:

```sh
# Terminal 1 — server
./build/bin/cypherush_server

# Terminal 2 — first client
./build/bin/cypherush_client

# Terminal 3 — second client
./build/bin/cypherush_client
```

## Custom Server Address

Server address is configured via command-line setup. After building, run
once:

```sh
cypherush_client.exe --setup <SERVER_IP>
```

This writes an obfuscated config to `%APPDATA%/Cypherush/config.dat`
(Linux: `~/.local/share/Cypherush/config.dat`). The address is hidden
from end users; not visible in the UI. Defaults to `127.0.0.1` if no
config exists.
