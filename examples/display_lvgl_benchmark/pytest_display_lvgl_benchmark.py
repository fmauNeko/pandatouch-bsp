# SPDX-FileCopyrightText: 2025 fmauNeko
# SPDX-License-Identifier: CC0-1.0

import datetime
import json
import os
import urllib.error
import urllib.request
from pathlib import Path

import pytest
from pytest_embedded import Dut

BOARD = "pandatouch"
BENCHMARK_RELEASES_URL = (
    "https://github.com/fmauNeko/pandatouch-bsp/releases/download/benchmark-latest"
)


def _write(ext: str, text: str) -> None:
    with open(f"benchmark_{BOARD}{ext}", "a") as f:
        f.write(text)


def _load_previous_json() -> dict:
    try:
        url = f"{BENCHMARK_RELEASES_URL}/benchmark_{BOARD}.json"
        with urllib.request.urlopen(url) as resp:
            return json.load(resp)
    except (urllib.error.HTTPError, urllib.error.URLError, json.JSONDecodeError):
        return {}


def _find_previous(prev: dict, name: str) -> dict:
    if not prev:
        return {}
    for t in prev.get("tests", []):
        if t["Name"] == name:
            return t
    return {}


def _diff(current: dict, previous: dict, field: str, higher_is_better: bool) -> str:
    if os.getenv("GITHUB_REF_NAME") == "main":
        return ""
    if not previous or not current.get(field) or not previous.get(field):
        return ""
    cur = int(current[field].replace("%", ""))
    prv = int(previous[field].replace("%", ""))
    delta = cur - prv
    if delta == 0:
        return ""
    if higher_is_better:
        color = "green" if delta > 0 else "red"
    else:
        color = "red" if delta > 0 else "green"
    sign = "+" if delta > 0 else ""
    return f'*<span style="color:{color}"><sub>({sign}{delta})</sub></span>*'


@pytest.mark.pandatouch
@pytest.mark.parametrize("target", ["esp32s3"])
def test_lvgl_benchmark(dut: Dut) -> None:
    date = datetime.datetime.now()

    Path(f"benchmark_{BOARD}.md").unlink(missing_ok=True)
    Path(f"benchmark_{BOARD}.json").unlink(missing_ok=True)

    dut.expect_exact("benchmark: Running LVGL benchmark", timeout=120)

    output: dict = {
        "date": date.strftime("%d.%m.%Y %H:%M"),
        "board": BOARD,
    }

    if os.getenv("GITHUB_REF_NAME") == "main":
        _write(".md", "## LVGL Benchmark\n\n")
    else:
        _write(".md", f"# Benchmark for BOARD {BOARD}\n\n")
    _write(".md", f"**DATE:** {date.strftime('%d.%m.%Y %H:%M')}\n\n")

    match = dut.expect(r"Benchmark Summary \(([\d.]+)\s*\)", timeout=200)
    lvgl_version = match[1].decode().strip()
    output["LVGL"] = lvgl_version
    _write(".md", f"**LVGL version:** {lvgl_version}\n\n")

    dut.expect(
        r"Name, Avg\. CPU, Avg\. FPS, Avg\. time, render time, flush time", timeout=30
    )
    _write(
        ".md", "| Name | Avg. CPU | Avg. FPS | Avg. time | render time | flush time |\n"
    )
    _write(
        ".md", "| ---- | :------: | :------: | :-------: | :---------: | :--------: |\n"
    )

    prev_json = _load_previous_json()
    output["tests"] = []

    for _ in range(17):
        m = dut.expect(
            r"([\w \.]+),[ ]?(\d+%),[ ]?(\d+),[ ]?(\d+),[ ]?(\d+),[ ]?(\d+)",
            timeout=200,
        )
        entry = {
            "Name": m[1].decode(),
            "Avg. CPU": m[2].decode(),
            "Avg. FPS": m[3].decode(),
            "Avg. time": m[4].decode(),
            "Render time": m[5].decode(),
            "Flush time": m[6].decode(),
        }
        output["tests"].append(entry)

        prev = _find_previous(prev_json, entry["Name"])
        _write(
            ".md",
            f"| {entry['Name']} "
            f"| {entry['Avg. CPU']} {_diff(entry, prev, 'Avg. CPU', False)} "
            f"| {entry['Avg. FPS']} {_diff(entry, prev, 'Avg. FPS', True)} "
            f"| {entry['Avg. time']} {_diff(entry, prev, 'Avg. time', False)} "
            f"| {entry['Render time']} {_diff(entry, prev, 'Render time', False)} "
            f"| {entry['Flush time']} {_diff(entry, prev, 'Flush time', False)} |\n",
        )

    _write(".md", "\n")
    if os.getenv("GITHUB_REF_NAME") != "main":
        _write(".md", "***\n\n")

    _write(".json", json.dumps(output, indent=4))
