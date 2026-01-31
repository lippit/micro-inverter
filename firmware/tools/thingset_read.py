#!/usr/bin/env python3
"""
Minimal ThingSet shell reader.

Examples:
  python tools/thingset_read.py --port /dev/ttyACM1
  python tools/thingset_read.py --port /dev/ttyACM1 --path Measurements/rValues
  python tools/thingset_read.py --port /dev/ttyACM1 --path Live --json-only
"""

import argparse
import re
import sys
import time

try:
    import serial  # type: ignore
except Exception:
    print("ERROR: pyserial is required. Install with: pip install pyserial", file=sys.stderr)
    raise

PROMPT_RE = re.compile(rb"([A-Za-z0-9_-]+):~\$ ")
ANSI_RE = re.compile(r"\x1b\\[[0-9;]*[A-Za-z]")


def read_until_prompt(ser: serial.Serial, timeout: float = 2.0) -> bytes:
    ser.timeout = 0.05
    data = bytearray()
    start = time.time()
    last_rx = start
    while True:
        chunk = ser.read(4096)
        now = time.time()
        if chunk:
            data.extend(chunk)
            last_rx = now
            if PROMPT_RE.search(data):
                break
        if now - last_rx > 0.2 or now - start > timeout:
            break
    return bytes(data)

def read_for(ser: serial.Serial, seconds: float) -> bytes:
    end = time.time() + max(0.0, seconds)
    data = bytearray()
    while time.time() < end:
        chunk = ser.read(4096)
        if chunk:
            data.extend(chunk)
    return bytes(data)


def send_cmd(ser: serial.Serial, cmd: str, timeout: float = 2.0) -> bytes:
    if not (cmd.endswith("\n") or cmd.endswith("\r")):
        cmd += "\r\n"
    ser.write(cmd.encode())
    ser.flush()
    # Read for a fixed duration to catch output even if prompt appears early
    data = read_for(ser, timeout)
    if PROMPT_RE.search(data):
        return data
    # Fallback to prompt-based read if nothing captured
    return read_until_prompt(ser, timeout=timeout)


def extract_json(text: str):
    start = None
    for i, ch in enumerate(text):
        if ch in "[{":
            start = i
            break
    if start is None:
    return None


def clean_text(text: str) -> str:
    text = ANSI_RE.sub("", text)
    # Drop prompt echoes for readability
    text = text.replace("\r", "")
    return text.strip()
    stack = []
    end = None
    for i in range(start, len(text)):
        c = text[i]
        if c in "[{":
            stack.append(c)
        elif c in "]}":
            if not stack:
                break
            top = stack.pop()
            if (top == "{" and c != "}") or (top == "[" and c != "]"):
                return None
            if not stack:
                end = i + 1
                break
    if end is None:
        return None
    blob = text[start:end]
    try:
        import json
        return json.loads(blob)
    except Exception:
        return None


def main() -> int:
    ap = argparse.ArgumentParser(description="Read ThingSet values via Zephyr shell")
    ap.add_argument("--port", required=True, help="Serial port (e.g. /dev/ttyACM1)")
    ap.add_argument("--baud", type=int, default=115200, help="Baud rate (default 115200)")
    ap.add_argument("--path", default="Measurements/rValues", help="ThingSet path to query")
    ap.add_argument("--timeout", type=float, default=2.0, help="Read timeout for each command")
    ap.add_argument("--json-only", action="store_true", help="Print only JSON payload")
    ap.add_argument("--use-thingset-prefix", action="store_true", help="Prefix commands with 'thingset '")
    args = ap.parse_args()

    try:
        ser = serial.Serial(args.port, baudrate=args.baud, timeout=0.2, write_timeout=1.0)
    except Exception as e:
        print(f"ERROR: cannot open {args.port}: {e}", file=sys.stderr)
        return 2

    try:
        try:
            ser.reset_input_buffer()
            ser.reset_output_buffer()
        except Exception:
            pass

        # Wake shell
        ser.write(b"\r\n")
        ser.flush()
        _ = read_until_prompt(ser, timeout=1.0)

        # Enter ThingSet shell
        send_cmd(ser, "select thingset", timeout=args.timeout)
        time.sleep(0.05)
        # Basic probe
        probe_cmd = "thingset ?" if args.use_thingset_prefix else "?"
        out = send_cmd(ser, probe_cmd, timeout=args.timeout)
        text = out.decode(errors="ignore")
        if extract_json(text) is None:
            print("ERROR: ThingSet shell not responding to '?'", file=sys.stderr)
            return 3

        # Query requested path
        path = args.path.strip().lstrip("/")
        cmd = f"?{path}"
        if args.use_thingset_prefix:
            cmd = f"thingset {cmd}"
        out = send_cmd(ser, cmd, timeout=args.timeout)
        text = out.decode(errors="ignore")
        js = extract_json(text)
        if args.json_only and js is not None:
            import json
            print(json.dumps(js))
        else:
            print(clean_text(text))
        return 0
    finally:
        try:
            ser.close()
        except Exception:
            pass


if __name__ == "__main__":
    raise SystemExit(main())
