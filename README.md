# MineTheRouter (MVP)

Minimal client-server Minecraft-like prototype with an authoritative server and a custom binary network protocol.

The project is implemented from scratch without using existing networking engines or game frameworks.  
The main goal is to build a clear and controllable networking architecture with server-authoritative logic and a custom message protocol.

Current development focus is on infrastructure:

- binary serialization
- message protocol
- TCP message framing
- basic handshake (Join / Ping)
- foundation for server world simulation


## Overview

Project architecture:

- **Authoritative server** — all game logic runs on the server
- **Thin client** — input, rendering and networking
- **Custom binary protocol over TCP**
- **Shared protocol module** used by both client and server

The project is intentionally kept relatively simple.  
In some places more straightforward implementations are preferred over complex abstractions to keep the networking logic explicit and easy to reason about.


## Project Structure

Solution: `minetherouter.sln`

Projects:

- `minetherouter-client` — game client (networking, input, rendering)
- `minetherouter-server` — authoritative server (game logic, world simulation)
- `minetherouter-common` — shared code (protocol, serialization, message types)

### Common module responsibilities

- ByteWriter / ByteReader (binary serialization)
- message definitions
- protocol contract
- shared network structures


## Tech Stack

- Language: **C++20**
- Build system: **CMake**
- IDE: **Visual Studio 2022**
- Networking: **custom TCP implementation**
- Platform (current): **Windows**
- Target (future): **Linux dedicated server + Windows client**
- Serialization: **custom little-endian binary protocol**


## Build

The project is built using **CMake**.

Helper scripts are provided:


build_debug.bat
build_release.bat


They perform:

1. CMake project generation
2. Visual Studio solution generation
3. project build
4. placing binaries into the `build` directory

After build the following binaries are available:

minetherouter-tests.exe
minetherouter-server.exe
minetherouter-client.exe



## Tests

Unit tests are located in the `minetherouter-common` module.

After building the project run:
build/bin/minetherouter-tests.exe


Tests cover:

- binary serialization
- message framing
- protocol message parsing
- handshake logic


## Network Protocol (MVP)

The project uses a custom binary protocol over TCP.

### Endianness

All numeric types are serialized in **little-endian** format.

Examples:

- `u16`
- `u32`
- `i32`
- `f32`

Least significant byte goes first.


### Message Format

Each network message contains a fixed-size header:

| Field        | Type | Size |
|--------------|------|------|
| MessageType  | u16  | 2 bytes |
| PayloadSize  | u32  | 4 bytes |
| Payload      | N    | N bytes |

Header size: **6 bytes**

This is required because **TCP is a streaming protocol and does not preserve message boundaries**.


## Networking (planned)

- network I/O in a separate thread
- thread-safe queues for incoming / outgoing messages
- clear separation between networking layer and simulation


## Possible Extensions

Potential future improvements:

- alternative transport support (TCP / UDP)
- world state streaming
- message compression / optimization