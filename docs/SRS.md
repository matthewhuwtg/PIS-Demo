# Software Requirements Specification (SRS)

## Passenger Information System (PIS) — Demo

| Document | SRS |
| Version | 1.0 |
| Date | 2026-06-16 |

## 1. Introduction

### 1.1 Purpose
This SRS describes the functional and non-functional requirements for the PIS Demo application, a demonstration program simulating a real-world passenger information system for public transportation.

### 1.2 Scope
The PIS Demo simulates train route progression with configurable stations and timings, provides real-time display of arrival/departure information, supports bilingual announcements, demonstrates IPC via Unix Domain Sockets, integrates with D-Bus, implements a media pipeline, offers a Qt5 GUI, and provides Python-based tooling.

### 1.3 Definitions
- PIS: Passenger Information System
- IPC: Inter-Process Communication
- D-Bus: Desktop Bus
- ICD: Interface Control Document
- SDD: Software Design Document

## 2. Functional Requirements

| ID | Description | Priority |
|----|-------------|----------|
| FR-01 | Load and parse JSON configuration files | High |
| FR-02 | Generate schedule events (DEPARTURE, APPROACHING, NEXT_STOP, ARRIVAL, TERMINUS) | High |
| FR-03 | Display real-time station information on console | High |
| FR-04 | Process announcements through a media pipeline | Medium |
| FR-05 | Broadcast events via Unix Domain Socket | Medium |
| FR-06 | Emit D-Bus signals (with mock fallback) | Medium |
| FR-07 | Qt5 GUI for visual information display | Low |

## 3. Non-Functional Requirements

- **Performance**: Event timing within +-100ms
- **Reliability**: Graceful degradation if D-Bus/GStreamer unavailable
- **Maintainability**: Modular design with separate config/schedule/display/IPC/D-Bus modules
- **Portability**: Linux (Ubuntu 20.04+, WSL) with g++ or clang++

## 4. Use Cases

**UC-01**: Full route simulation — operator launches demo, system loads config, schedule engine runs events sequentially
**UC-02**: External client connection — client connects to Unix socket, receives binary header + JSON payload events
