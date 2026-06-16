# PIS (Passenger Information System) Demo

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)

A comprehensive Passenger Information System (PIS) demo written in **C++23**, demonstrating event-driven architecture, Unix/Linux IPC (Sockets, D-Bus), GStreamer-inspired media pipeline, Qt5 GUI, and professional software documentation.

## Features

| Feature | Technology |
|---------|-----------|
| **Event-Driven Core** | C++23 callbacks & threads |
| **JSON Configuration** | Custom lightweight parser (no external deps) |
| **Terminal Display** | ANSI-color, bilingual (EN/ZH) |
| **Unix Socket IPC** | AF_UNIX binary header + JSON payload |
| **D-Bus Integration** | Real libdbus-1 with mock fallback |
| **Media Pipeline** | GStreamer-inspired Source-Processor-Sink |
| **Qt5 GUI** | Dark-themed real-time dashboard |
| **Python Tooling** | Config generator + test runner |
| **Bash Automation** | Build/test/dependency-check scripts |
| **Documentation** | SRS, SDD, ICD (English) |

## Quick Start

```bash
# Check dependencies
./scripts/build.sh --check

# Build and run
make clean && make
./bin/main
```

## Requirements

- **C++23 compiler** (g++ 11+ or clang++ 14+)
- **GNU Make**, **Python 3.x**
- **POSIX-compliant OS** (Linux, WSL)

Optional: libdbus-1-dev, libgstreamer1.0-dev, qtbase5-dev
