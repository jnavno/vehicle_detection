#!/usr/bin/env python3
"""
vehicle_mesh_bridge.py

Reads structured event lines from the Waveshare vehicle detector over USB serial
and sends Meshtastic text messages through a RAK node using the Meshtastic CLI.

Working architecture:
  Waveshare USB serial -> this bridge -> RAK USB API -> Meshtastic LoRa text

Expected Waveshare event lines:
  veh:cal,...
  veh:1,ALARM,...
  veh:0,CLEAR,...

Example messages sent:
  VEHICLE SENSOR READY
  VEHICLE ALERT
  VEHICLE CLEAR
  VEHICLE SENSOR ERROR
"""

import re
import subprocess
import time
from datetime import datetime

import serial


# -----------------------------
# User configuration
# -----------------------------

WAVESHARE_PORT = "/dev/ttyACM1"   # Waveshare USB serial port
WAVESHARE_BAUD = 115200

RAK_PORT = "/dev/ttyACM0"         # RAK Meshtastic USB API port
MESHTASTIC_BIN = "/home/anda/.local/bin/meshtastic"

COOLDOWN_SECONDS = 10


# Only match structured detector event lines.
# This avoids duplicate sends from debug lines like "[MESH-OUT] VEH_ALARM".
EVENT_PATTERNS = [
    ("VEH_ALARM", re.compile(r"^veh:1,ALARM")),
    ("VEH_CLEAR", re.compile(r"^veh:0,CLEAR")),
    ("VEH_READY", re.compile(r"^veh:cal")),
    ("MAG_FAIL", re.compile(r"\bMAG_.*FAIL\b|Sensor read fail|MLX90393 not found")),
]


EVENT_MESSAGES = {
    "VEH_ALARM": "VEHICLE ALERT",
    "VEH_CLEAR": "VEHICLE CLEAR",
    "VEH_READY": "VEHICLE SENSOR READY",
    "MAG_FAIL": "VEHICLE SENSOR ERROR",
}


last_sent = {}


def now_stamp() -> str:
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def send_mesh_text(text: str) -> bool:
    print(f"[{now_stamp()}] SEND -> {text}", flush=True)

    try:
        result = subprocess.run(
            [MESHTASTIC_BIN, "--port", RAK_PORT, "--sendtext", text],
            text=True,
            capture_output=True,
            timeout=30,
            check=False,
        )

        if result.stdout.strip():
            print(result.stdout.strip(), flush=True)

        if result.stderr.strip():
            print(result.stderr.strip(), flush=True)

        if result.returncode != 0:
            print(
                f"[{now_stamp()}] ERROR: meshtastic exited with code {result.returncode}",
                flush=True,
            )
            return False

        return True

    except subprocess.TimeoutExpired:
        print(f"[{now_stamp()}] ERROR: meshtastic command timed out", flush=True)
        return False

    except FileNotFoundError:
        print(f"[{now_stamp()}] ERROR: meshtastic binary not found: {MESHTASTIC_BIN}", flush=True)
        return False

    except Exception as exc:
        print(f"[{now_stamp()}] ERROR: {exc}", flush=True)
        return False


def maybe_send(event_name: str, raw_line: str) -> None:
    now = time.time()
    previous = last_sent.get(event_name, 0)

    if now - previous < COOLDOWN_SECONDS:
        print(f"[{now_stamp()}] cooldown skip {event_name}", flush=True)
        return

    last_sent[event_name] = now

    msg = EVENT_MESSAGES.get(event_name, event_name)

    ok = send_mesh_text(msg)

    if ok:
        print(f"[{now_stamp()}] SENT OK event={event_name}", flush=True)
    else:
        print(f"[{now_stamp()}] SENT FAILED event={event_name} raw={raw_line}", flush=True)


def classify_event(line: str) -> str | None:
    for event_name, pattern in EVENT_PATTERNS:
        if pattern.search(line):
            return event_name

    return None


def main() -> None:
    print(f"[{now_stamp()}] Vehicle Mesh Bridge starting", flush=True)
    print(f"[{now_stamp()}] Waveshare: {WAVESHARE_PORT} @ {WAVESHARE_BAUD}", flush=True)
    print(f"[{now_stamp()}] RAK:       {RAK_PORT}", flush=True)
    print(f"[{now_stamp()}] Meshtastic CLI: {MESHTASTIC_BIN}", flush=True)
    print(f"[{now_stamp()}] Cooldown: {COOLDOWN_SECONDS}s", flush=True)

    while True:
        try:
            with serial.Serial(WAVESHARE_PORT, WAVESHARE_BAUD, timeout=1) as ser:
                print(f"[{now_stamp()}] Opened Waveshare serial", flush=True)

                while True:
                    raw = ser.readline()

                    if not raw:
                        continue

                    line = raw.decode("utf-8", errors="replace").strip()

                    if not line:
                        continue

                    print(f"[{now_stamp()}] WS: {line}", flush=True)

                    event_name = classify_event(line)

                    if event_name:
                        maybe_send(event_name, line)

        except serial.SerialException as exc:
            print(f"[{now_stamp()}] SERIAL ERROR: {exc}", flush=True)
            print(f"[{now_stamp()}] Retrying in 5 seconds...", flush=True)
            time.sleep(5)

        except KeyboardInterrupt:
            print(f"\n[{now_stamp()}] Stopped by user", flush=True)
            return

        except Exception as exc:
            print(f"[{now_stamp()}] ERROR: {exc}", flush=True)
            print(f"[{now_stamp()}] Retrying in 5 seconds...", flush=True)
            time.sleep(5)


if __name__ == "__main__":
    main()