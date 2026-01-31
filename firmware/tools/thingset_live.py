#!/usr/bin/env python3
"""
Enable ThingSet live reporting over shell and print incoming reports.

Example:
  python tools/thingset_live.py --port /dev/ttyACM1 --period 1.0
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
ANSI_RE = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")


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


def send_cmd(ser: serial.Serial, cmd: str, timeout: float = 2.0) -> bytes:
    if not (cmd.endswith("\n") or cmd.endswith("\r")):
        cmd += "\r\n"
    ser.write(cmd.encode())
    ser.flush()
    return read_until_prompt(ser, timeout=timeout)


def clean_line(line: str) -> str:
    return ANSI_RE.sub("", line).replace("\r", "").strip()


def main() -> int:
    ap = argparse.ArgumentParser(description="Enable and print ThingSet live reports")
    ap.add_argument("--port", required=True, help="Serial port (e.g. /dev/ttyACM1)")
    ap.add_argument("--baud", type=int, default=115200, help="Baud rate (default 115200)")
    ap.add_argument("--period", type=float, default=1.0, help="Live report period in seconds")
    ap.add_argument("--timeout", type=float, default=2.0, help="Command timeout in seconds")
    ap.add_argument("--raw", action="store_true", help="Print all incoming lines (no filtering)")
    args = ap.parse_args()

    try:
        ser = serial.Serial(args.port, baudrate=args.baud, timeout=0.1, write_timeout=1.0)
    except Exception as e:
        print(f"ERROR: cannot open {args.port}: {e}", file=sys.stderr)
        return 2

    try:
        # Wake shell
        ser.write(b"\r\n")
        ser.flush()
        _ = read_until_prompt(ser, timeout=1.0)

        # Enter ThingSet shell
        send_cmd(ser, "select thingset", timeout=args.timeout)

        # Enable live reporting
        cmd = f"=_Reporting/mLive {{\"sEnable\":true,\"sPeriod_s\":{int(max(1, round(args.period)))} }}"
        send_cmd(ser, cmd, timeout=args.timeout)
        # Confirm state (best-effort)
        state = send_cmd(ser, "?_Reporting/mLive/sEnable", timeout=args.timeout)
        state_txt = clean_line(state.decode(errors="ignore"))
        if state_txt:
            print(state_txt)
        print("Live reporting enabled. Press Ctrl-C to stop.")

        # Stream incoming data
        buf = bytearray()
        while True:
            chunk = ser.read(4096)
            if chunk:
                buf.extend(chunk)
                # Process complete lines
                while b"\n" in buf:
                    line, _, rest = buf.partition(b"\n")
                    buf = bytearray(rest)
                    text = clean_line(line.decode(errors="ignore"))
                    if not text:
                        continue
                    if args.raw:
                        print(text)
                        continue
                    # Print only report-like lines
                    if "mLive" in text or text.startswith(":85") or text.startswith("#mLive"):
                        print(text)
            else:
                time.sleep(0.01)
    except KeyboardInterrupt:
        pass
    finally:
        try:
            # Disable live reporting on exit
            send_cmd(ser, "=_Reporting/mLive {\"sEnable\":false}", timeout=1.0)
        except Exception:
            pass
        try:
            ser.close()
        except Exception:
            pass

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
