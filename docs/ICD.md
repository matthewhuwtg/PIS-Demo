# Interface Control Document (ICD)

## Passenger Information System (PIS) — Demo

## 1. Unix Domain Socket Interface

- **Type**: AF_UNIX, SOCK_STREAM
- **Default path**: /tmp/pis_demo.sock

### Message Format

**Header (24 bytes)**:
```
Offset  Size  Field         Type
 0       4     magic        uint32 (0x50495331 = "PIS1")
 4       4     version      uint32 (1)
 8       4     msg_type     uint32 (0=DEPARTURE..4=TERMINUS)
12       4     payload_len  uint32
16       8     timestamp_ms uint64
```

**Payload**: JSON object
```json
{"type":"ARRIVAL","station_id":"S03","station_name_en":"University","elapsed_sec":300,"message":"Arriving at University"}
```

## 2. D-Bus Interface

| Parameter | Value |
|-----------|-------|
| Bus | Session Bus |
| Service | com.pis.demo |
| Object | /com/pis/demo |
| Interface | com.pis.demo.Interface |

**Signal**: PISEvent(string type, string station, string message, int32 elapsed_sec)

## 3. Configuration Schema

```json
{
  "system": { "name": "string", "version": "string" },
  "route": { "id": "string", "stations": [{ "id": "string", "name_en": "string", "arrival_sec": "int" }] },
  "display": { "refresh_interval_ms": "int", "color_scheme": "string" },
  "ipc": { "socket_path": "string", "enabled": "bool" },
  "dbus": { "service_name": "string", "object_path": "string", "enabled": "bool" },
  "media": { "announcement_enabled": "bool", "audio_backend": "string" }
}
```

## 4. Python Script Interfaces

### generate_config.py
- `-n, --stations N` — number of stations
- `-r, --route NAME` — route name
- `-o, --output FILE` — output file

### run_tests.py
- `--no-build` — skip build step
- `--verbose, -v` — detailed output
