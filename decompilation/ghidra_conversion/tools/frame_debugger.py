#!/usr/bin/env python3
"""Drive the local Qt frame debugger over its newline-delimited JSON protocol.

Examples:
  python frame_debugger.py status
  python frame_debugger.py --port 47800 press a 2 1
  python frame_debugger.py break rng 'bounded\\.bound=6'
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
import sys
from typing import Any


def build_request(words: list[str]) -> dict[str, Any]:
    if not words:
        raise ValueError("enter a debugger command")

    command, *args = words
    if command in {"status", "pause", "resume", "hash", "list", "help"}:
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
