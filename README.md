# SmartClip

An ESP32 weather station that serves its own live web dashboard over Wi-Fi — no cloud, no router, no app. Power it up, join the `SmartClip` access point, and open `http://smartclip.local`.

Temperature and humidity come from a DHT22; rain is detected with an analog rain sensor. Readings are sampled every 5 seconds and the last 5 minutes are kept in a rolling buffer for the dashboard's sparkline charts.

![status](https://img.shields.io/badge/platform-ESP32-informational) ![framework](https://img.shields.io/badge/framework-Arduino-teal) ![build](https://img.shields.io/badge/build-PlatformIO-orange)

## Features

- **Standalone Wi-Fi access point** — the ESP32 *is* the network, so it works anywhere with power
- **Live dashboard** at `/` — animated rain background, current conditions, temperature and humidity sparklines
- **JSON API** at `/api/data` for scripting or integrating with something else
- **mDNS + captive DNS** so `smartclip.local` resolves without knowing the IP
- **5-minute rolling history** (60 samples at 5-second intervals)
- **Graceful sensor failures** — a missed DHT22 read renders as `--` instead of breaking the page
- Serial log of every reading for debugging without a browser

## Hardware

| Component | Notes |
|---|---|
| ESP32 devkit (`esp32doit-devkit-v1`) | Any ESP32 with an exposed GPIO34 works |
| DHT22 / AM2302 | Temperature and humidity |
| Analog rain sensor module | e.g. FC-37 / YL-83 with a comparator board |
| 10 kΩ resistor | Pull-up for the DHT22 data line |
| 3.3 V supply | Sensors are powered from the board's 3V3 pin |

### Wiring

| Sensor pin | ESP32 pin | Notes |
|---|---|---|
| DHT22 `VCC` | `3V3` | |
| DHT22 `DATA` | `GPIO4` | 10 kΩ pull-up from `DATA` to `3V3` |
| DHT22 `GND` | `GND` | |
| Rain sensor `VCC` | `3V3` | |
| Rain sensor `GND` | `GND` | |
| Rain sensor `AO` | `GPIO34` | Analog output — this is what the firmware reads |
| Rain sensor `DO` | `GPIO27` | Configured as input but currently unused |

`GPIO34` is input-only and has no internal pull-up, which is fine for the analog output. The firmware reads the raw 12-bit value (0–4095) and treats anything **below 2500 as raining**; a dry board reads near 4095. That threshold is the one number you will most likely need to tune for your sensor — see [Configuration](#configuration).

## Getting started

Requires [PlatformIO](https://platformio.org/install) (CLI or the VS Code extension).

```bash
git clone https://github.com/zapds/smart_clip.git
cd smart_clip

pio run                 # compile
pio run --target upload # flash
pio device monitor      # serial log at 115200 baud
```

`platformio.ini` pins the upload and monitor port to `/dev/ttyUSB0`. On macOS or Windows, either change `upload_port` / `monitor_port` or delete both lines and let PlatformIO auto-detect.

### Using it

1. Flash the board and open the serial monitor — it prints the AP name and IP on boot.
2. On your phone or laptop, join the Wi-Fi network **`SmartClip`** (password `smartclip123`).
3. Open **http://smartclip.local** — or the IP printed on the serial console, usually `192.168.4.1`.

The dashboard polls `/api/data` every 5 seconds and updates in place, so you can leave it open as a wall display.

## API

### `GET /api/data`

```json
{
  "current": {
    "raining": false,
    "rain_ao": 4087,
    "temp": 24.6,
    "hum": 51.2
  },
  "history": [
    { "temp": 24.5, "hum": 51.0 },
    { "temp": 24.6, "hum": 51.2 }
  ]
}
```

- `raining` — boolean, derived from `rain_ao < 2500`
- `rain_ao` — raw 12-bit analog reading, `4095` = dry
- `temp` / `hum` — `null` when the DHT22 read fails (bad checksum, sensor unplugged, read too soon after boot)
- `history` — oldest to newest, at most 60 entries, temperature and humidity only

`GET /` returns the dashboard HTML. There are no other routes.

## Configuration

All tunables are `constexpr` at the top of [`src/main.cpp`](src/main.cpp):

| Constant | Default | Purpose |
|---|---|---|
| `AP_SSID` | `SmartClip` | Access point name |
| `AP_PASS` | `smartclip123` | Access point password (min 8 chars) |
| `MDNS_HOST` | `smartclip` | Resolves as `smartclip.local` |
| `DHT_PIN` | `4` | DHT22 data pin |
| `RAIN_AO_PIN` | `34` | Rain sensor analog pin |
| `RAIN_ANALOG_THRESHOLD` | `2500` | Below this = raining |
| `LOG_INTERVAL_MS` | `5000` | Sampling interval |
| `HISTORY_SIZE` | `60` | Samples retained (60 × 5 s = 5 min) |

> **Note:** the AP password is committed in source. That is acceptable for a local demo, but if you plan to deploy this anywhere real, move the credentials into a gitignored `secrets.h`.

## Project structure

```
smart_clip/
├── platformio.ini      # board, framework, library dependencies
└── src/
    ├── main.cpp        # Wi-Fi AP, DNS/mDNS, HTTP server, sensor sampling, JSON API
    └── dashboard.h     # dashboard HTML/CSS/JS served from flash (PROGMEM)
```

The dashboard is a single self-contained page with no external assets — charts are hand-drawn inline SVG and the rain effect is a canvas animation, so it works with zero internet access.

## Limitations

- **Access point only.** It does not join an existing network, so you lose internet while connected to it on most devices.
- **History is in RAM** and resets on reboot.
- **Rain is binary**, not a rate or accumulation measurement.
- **One client at a time** is the practical limit for the ESP32's soft AP and the Arduino `WebServer`.

## License

No license specified yet. Add one before reusing this code — until then, default copyright applies.
