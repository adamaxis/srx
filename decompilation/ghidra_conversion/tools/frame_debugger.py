#!/usr/bin/env python3
"""Drive the local Qt frame debugger over its newline-delimited JSON protocol.

Examples:
  python frame_debugger.py status
  python frame_debugger.py --port 47800 press a 2 1
  python frame_debugger.py --port 47800 seek --exe game.exe --session run.srxkey --frame 500
  python frame_debugger.py --port 47800 back --exe game.exe --session run.srxkey --frames 1
  python frame_debugger.py quit
  python frame_debugger.py break rng 'bounded\\.bound=6'
  python frame_debugger.py replay-to --exe shadowrun_ghidra_conversion_qt.exe --session run.srxkey --frame 500 --dump frame.bmp
  python frame_debugger.py repl

The client deliberately exposes only commands implemented by the active Qt
runtime. It has no game-state logic and uses only the Python standard library.
"""

from __future__ import annotations

import argparse
import json
import os
import shlex
import socket
import subprocess
import sys
import time
from typing import Any


def build_request(words: list[str]) -> dict[str, Any]:
    if not words:
        raise ValueError("enter a debugger command")

    command, *args = words
    if command in {"status", "pause", "resume", "hash", "list", "help", "quit"}:
        if args:
            raise ValueError(f"{command} takes no arguments")
        return {"cmd": command}
    if command == "step":
        if len(args) > 1:
            raise ValueError("usage: step [frame-count]")
        return {"cmd": "step", "n": int(args[0]) if args else 1}
    if command == "key":
        if len(args) != 2 or args[1] not in {"press", "release"}:
            raise ValueError("usage: key up|down|left|right|a|b|c|start press|release")
        return {"cmd": "key", "key": args[0], "pressed": args[1] == "press"}
    if command == "press":
        if not 1 <= len(args) <= 3:
            raise ValueError("usage: press up|down|left|right|a|b|c|start [hold-frames [release-frames]]")
        return {
            "cmd": "press",
            "key": args[0],
            "frames": int(args[1]) if len(args) > 1 else 1,
            "release": int(args[2]) if len(args) > 2 else 1,
        }
    if command == "inspect":
        if len(args) > 1:
            raise ValueError("usage: inspect [field-prefix]")
        return {"cmd": "inspect", "prefix": args[0] if args else ""}
    if command == "dump":
        if len(args) != 1:
            raise ValueError("usage: dump image-path")
        return {"cmd": "dump", "path": args[0]}
    if command == "events":
        if not args:
            raise ValueError("usage: events recording.srxkey [--from N] [--to N] [--pattern REGEX]")
        request: dict[str, Any] = {"cmd": "events", "path": args.pop(0)}
        while args:
            option = args.pop(0)
            if option not in {"--from", "--to", "--pattern"} or not args:
                raise ValueError("usage: events recording.srxkey [--from N] [--to N] [--pattern REGEX]")
            value = args.pop(0)
            field = {"--from": "from", "--to": "to", "--pattern": "pattern"}[option]
            request[field] = int(value) if field in {"from", "to"} else value
        return request
    if command == "diverge":
        if len(args) != 2:
            raise ValueError("usage: diverge first.srxkey second.srxkey")
        return {"cmd": "diverge", "a": args[0], "b": args[1]}
    if command == "break":
        if not args:
            raise ValueError("usage: break frame|watch|event|rng|scene ...")
        kind, *details = args
        request = {"cmd": "break", "kind": kind}
        if kind == "frame" and len(details) == 1:
            request["frame"] = int(details[0])
        elif kind == "watch" and len(details) == 1:
            request["field"] = details[0]
        elif kind == "event" and len(details) == 1:
            request["pattern"] = details[0]
        elif kind == "rng" and len(details) <= 1:
            if details:
                request["pattern"] = details[0]
        elif kind == "scene" and len(details) <= 1:
            if details:
                request["scene"] = int(details[0], 0)
        else:
            raise ValueError("usage: break frame N | watch FIELD | event REGEX | rng [REGEX] | scene [SCENE-ID]")
        return request
    if command == "delete":
        if len(args) != 1:
            raise ValueError("usage: delete breakpoint-id")
        return {"cmd": "delete", "id": int(args[0])}
    if command == "raw":
        if len(args) != 1:
            raise ValueError("usage: raw '{\"cmd\":\"status\"}'")
        value = json.loads(args[0])
        if not isinstance(value, dict):
            raise ValueError("raw request must be a JSON object")
        return value
    raise ValueError(f"unknown command: {command}")


def send_request(connection: socket.socket, request: dict[str, Any]) -> str:
    stream = connection.makefile("rwb")
    try:
        stream.write((json.dumps(request, separators=(",", ":")) + "\n").encode("utf-8"))
        stream.flush()
        response = stream.readline()
        if not response:
            raise ConnectionError("debugger closed the connection without a response")
        return response.decode("utf-8").rstrip("\r\n")
    finally:
        stream.close()


def replay_to(words: list[str]) -> int:
    """Reconstruct an arbitrary recorded frame from the deterministic launch state."""
    parser = argparse.ArgumentParser(
        prog="frame_debugger.py replay-to",
        description="Rebuild a recorded frame by replaying from the game launch state.",
    )
    parser.add_argument("--exe", required=True, help="Qt game executable")
    parser.add_argument("--session", required=True, help=".srxkey input recording")
    parser.add_argument("--frame", required=True, type=int, help="completed source frame")
    parser.add_argument("--dump", required=True, help="output image path")
    parser.add_argument("--assets", help="asset directory, if it is not auto-detected")
    options = parser.parse_args(words)
    if options.frame < 0:
        parser.error("--frame must be nonnegative")
    command = [
        options.exe,
        "--replay-input",
        options.session,
        "--headless-replay",
        "--replay-to-frame",
        str(options.frame),
        "--dump-frame",
        options.dump,
    ]
    if options.assets:
        command.extend(["--assets", options.assets])
    try:
        return subprocess.run(command, check=False).returncode
    except OSError as error:
        print(f"frame debugger replay-to: {error}", file=sys.stderr)
        return 2


def debug_at(words: list[str], port: int, backwards: bool = False) -> int:
    """Restart the local debugger at a frame reconstructed from a recording.

    The active Qt runtime does not serialize its full C++ and Lua state yet.
    Replaying from the launch state gives the same seek behavior without
    pretending an incomplete snapshot is safe to restore.
    """
    parser = argparse.ArgumentParser(
        prog="frame_debugger.py back" if backwards else "frame_debugger.py seek",
        description=(
            "Restart the headless debugger at a prior recorded frame."
            if backwards
            else "Restart the headless debugger at a recorded frame."
        ),
    )
    parser.add_argument("--exe", required=True, help="Qt game executable")
    parser.add_argument("--session", required=True, help=".srxkey input recording")
    if backwards:
        parser.add_argument("--frames", type=int, default=1,
                            help="number of source frames to move backward")
        parser.add_argument("--from-frame", type=int,
                            help="use this frame if no debugger is running")
    else:
        parser.add_argument("--frame", required=True, type=int,
                            help="completed source frame")
    parser.add_argument("--assets", help="asset directory, if not auto-detected")
    options = parser.parse_args(words)
    if not 1 <= port <= 65535:
        parser.error("--port must be from 1 to 65535")
    if backwards:
        if options.frames < 1:
            parser.error("--frames must be positive")
        if options.from_frame is not None:
            target_frame = max(0, options.from_frame - options.frames)
        else:
            try:
                status = json.loads(send_to_port(port, {"cmd": "status"}))
                if not status.get("ok") or not isinstance(status.get("frame"), int):
                    raise ValueError("debugger status did not include a source frame")
                target_frame = max(0, status["frame"] - options.frames)
            except (OSError, ConnectionError, TimeoutError, ValueError,
                    json.JSONDecodeError) as error:
                print(f"frame debugger back: {error}", file=sys.stderr)
                return 2
    else:
        target_frame = options.frame
    if target_frame < 0:
        parser.error("frame must be nonnegative")

    # Close an existing host cleanly so an active recording flushes its final
    # frame before the same file is reopened for deterministic replay.
    try:
        response = json.loads(send_to_port(port, {"cmd": "status"}))
        if response.get("ok"):
            send_to_port(port, {"cmd": "quit"})
            deadline = time.monotonic() + 10.0
            while time.monotonic() < deadline:
                try:
                    with socket.create_connection(("127.0.0.1", port), timeout=0.2):
                        time.sleep(0.05)
                except OSError:
                    break
            else:
                print("frame debugger: existing host did not release its port",
                      file=sys.stderr)
                return 2
    except (OSError, ConnectionError, TimeoutError, json.JSONDecodeError):
        pass  # No debugger is running; start a new replay host below.

    command = [
        options.exe,
        "--replay-input",
        options.session,
        "--debug-port",
        str(port),
        "--replay-to-frame",
        str(target_frame),
    ]
    if options.assets:
        command.extend(["--assets", options.assets])
    try:
        process = subprocess.Popen(command)
    except OSError as error:
        print(f"frame debugger {parser.prog.rsplit(' ', 1)[-1]}: {error}",
              file=sys.stderr)
        return 2

    deadline = time.monotonic() + 60.0
    while time.monotonic() < deadline:
        if process.poll() is not None:
            print(f"frame debugger: replay host exited with code {process.returncode}",
                  file=sys.stderr)
            return process.returncode or 2
        try:
            response = json.loads(send_to_port(port, {"cmd": "status"}))
            if response.get("ok"):
                print(f"debugger restarted paused at source frame {target_frame} "
                      f"on 127.0.0.1:{port}")
                return 0
        except (OSError, ConnectionError, TimeoutError, json.JSONDecodeError):
            time.sleep(0.05)
    process.terminate()
    try:
        process.wait(timeout=5.0)
    except subprocess.TimeoutExpired:
        process.kill()
    print("frame debugger: replay host did not become ready", file=sys.stderr)
    return 2


def send_to_port(port: int, request: dict[str, Any]) -> str:
    with socket.create_connection(("127.0.0.1", port), timeout=1.0) as connection:
        connection.settimeout(1.0)
        return send_request(connection, request)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "--port", type=int, default=int(os.environ.get("SRXDBG_PORT", "47800")),
        help="local debugger port (default: SRXDBG_PORT or 47800)",
    )
    parser.add_argument("command", nargs=argparse.REMAINDER)
    arguments = parser.parse_args(argv)
    words = arguments.command
    if words and words[0] == "--":
        words = words[1:]
    if not words:
        parser.print_help()
        return 2
    if words[0] == "replay-to":
        return replay_to(words[1:])
    if words[0] in {"seek", "back"}:
        try:
            return debug_at(words[1:], arguments.port, words[0] == "back")
        except (OSError, ConnectionError, TimeoutError, ValueError,
                json.JSONDecodeError) as error:
            print(f"frame debugger {words[0]}: {error}", file=sys.stderr)
            return 2

    try:
        request = None if words == ["repl"] else build_request(words)
        with socket.create_connection(("127.0.0.1", arguments.port), timeout=5.0) as connection:
            connection.settimeout(5.0)
            if words == ["repl"]:
                for line in sys.stdin:
                    if not line.strip():
                        continue
                    try:
                        print(send_request(connection, build_request(shlex.split(line))), flush=True)
                    except (ValueError, json.JSONDecodeError) as error:
                        print(f"error: {error}", file=sys.stderr, flush=True)
                return 0
            assert request is not None
            print(send_request(connection, request))
            return 0
    except (OSError, ConnectionError, TimeoutError, ValueError, json.JSONDecodeError) as error:
        print(f"frame debugger: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
