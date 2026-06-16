# Software Design Document (SDD)

## Passenger Information System (PIS) — Demo

## 1. Architecture

The PIS Demo uses a modular, event-driven architecture with a central engine orchestrating independent modules.

```
PISEngine
├── ConfigManager — parses JSON configuration
├── ScheduleEngine — event-driven timer loop
├── DisplayModule — ANSI console output + media pipeline
├── IpcServer — Unix Domain Socket broadcast
└── DBusInterface — D-Bus signal emission (real or mock)
```

## 2. Modules

### ConfigManager
- Custom lightweight JSON parser (no external dependencies)
- Parses system, route, display, IPC, D-Bus, and media configuration

### ScheduleEngine
- Observer pattern with callback registration via std::function
- Worker thread with std::chrono-based timing
- Event types: DEPARTURE, APPROACHING, NEXT_STOP, ARRIVAL, TERMINUS

### DisplayModule
- ANSI color console output
- GStreamer-inspired pipeline: AnnouncementSource -> AudioMixer -> AudioSink

### IpcServer/IpcClient
- Unix Domain Socket (AF_UNIX, SOCK_STREAM)
- Protocol: 24-byte binary header (magic=0x50495331, version, type, len, ts) + JSON payload

### DBusInterface
- Conditional compilation (#ifdef HAS_DBUS)
- Real libdbus-1 when available, mock implementation otherwise
- Interface: com.pis.demo.Interface, Signal: PISEvent

### PISEngine
- Initialization sequence: Config -> Display -> Schedule -> IPC -> D-Bus
- Callback wiring: ScheduleEvent -> Display + IPC + D-Bus

## 3. Data Flow

```
Config JSON -> ConfigManager -> PISConfig
                                    |
ScheduleEngine.run() -> fireEvent() -> DisplayModule (console)
                                   -> IpcServer (socket)
                                   -> DBusInterface (D-Bus)
```
