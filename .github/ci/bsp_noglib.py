#!/usr/bin/env python
#
# SPDX-FileCopyrightText: 2026 fmauNeko
# SPDX-License-Identifier: MIT

"""
Remove esp_lvgl_port from BSP's dependencies
"""

import os
import shutil
import sys
import argparse
import yaml
from pathlib import Path
from idf_component_tools.manager import ManifestManager
from update_readme_dependencies import check_bsp_readme

DEFINE_NOGLIB_OFF = "#define BSP_CONFIG_NO_GRAPHIC_LIB 0"
DEFINE_NOGLIB_ON = "#define BSP_CONFIG_NO_GRAPHIC_LIB 1"
ESP_REGISTRY_URL = "https://components.espressif.com/components/"
README_NOGLIB_NOTICE = "> :warning: This is **No Graphical Library version** of {} BSP. If you want to use this BSP with LVGL use [{}]({}) component.\n"


def select_bsp_config_no_graphic_lib(bsp_path):
    config_path = bsp_path / "include" / "bsp" / "config.h"
    try:
        with open(config_path, encoding="utf-8", mode="r") as header:
            content = header.read()
        original_content = content
        content = content.replace(DEFINE_NOGLIB_OFF, DEFINE_NOGLIB_ON)
        if content == original_content:
            # No replacement occurred - DEFINE_NOGLIB_OFF not found
            print(
                "{}: could not find {} in config.h".format(
                    str(bsp_path), DEFINE_NOGLIB_OFF
                )
            )
            return 1
        with open(config_path, encoding="utf-8", mode="w", newline="\n") as header:
            header.write(content)
        return 0
    except FileNotFoundError:
        print("{}: could not modify config.h".format(str(bsp_path)))
        return 1


def remove_esp_lvgl_port(bsp_path):
    manager = ManifestManager(bsp_path, "bsp")
    deps = manager.manifest.dependencies
    try:
        del deps["espressif/esp_lvgl_port"]
    except KeyError:
        print("{}: could not remove esp_lvgl_port".format(str(bsp_path)))
        return 1
    try:
        del deps["lvgl/lvgl"]
    except KeyError:
        print("{}: no lvgl dependency found".format(str(bsp_path)))

    manager.manifest.description = (
        manager.manifest.description + " with no graphical library"
    )

    # Add 'noglib' tag
    tags = manager.manifest.tags or []
    if "noglib" not in tags:
        tags.append("noglib")
    manager.manifest.tags = tags

    manager.dump()
    return 0


def update_examples(bsp_path):
    manager = ManifestManager(bsp_path, "bsp")
    bsp_name = bsp_path.name

    examples_dir = bsp_path.parent / "examples"

    if not examples_dir.exists():
        print(
            "{}: examples directory not found at {}".format(
                str(bsp_path), str(examples_dir)
            )
        )
        return 0

    compatible_examples = []

    for example_dir in sorted(examples_dir.iterdir()):
        if not example_dir.is_dir():
            continue

        manifest_path = example_dir / "main" / "idf_component.yml"
        if not manifest_path.exists():
            print(
                "{}: skipping {} (no main/idf_component.yml)".format(
                    str(bsp_path), example_dir.name
                )
            )
            continue

        try:
            with open(manifest_path, "r", encoding="utf-8") as f:
                example_manifest = yaml.safe_load(f) or {}

            dependencies = example_manifest.get("dependencies", {})

            if bsp_name in dependencies:
                relative_path = "../examples/{}".format(example_dir.name)
                compatible_examples.append({"path": relative_path})
                print(
                    "{}: found compatible example: {}".format(
                        str(bsp_path), example_dir.name
                    )
                )
        except (yaml.YAMLError, IOError, OSError) as e:
            print(
                "{}: error parsing {}: {}".format(str(bsp_path), manifest_path, str(e))
            )
            continue

    if compatible_examples:
        manager.manifest.examples = compatible_examples
    else:
        try:
            del manager.manifest.examples
        except (AttributeError, KeyError):
            pass

    manager.dump()
    return 0


def add_notice_to_readme(bsp_path):
    readme_path = bsp_path / "README.md"
    bsp_name = bsp_path.parts[-1].removesuffix("_noglib")

    # Namespace for Espressif Component Registry (may differ from GitHub org)
    namespace = os.environ.get("BSP_REGISTRY_NAMESPACE", "fmauneko")

    try:
        with open(readme_path, encoding="utf-8", mode="r") as readme:
            content = readme.readlines()

        content.insert(
            0,
            README_NOGLIB_NOTICE.format(
                bsp_name, bsp_name, ESP_REGISTRY_URL + namespace + "/" + bsp_name
            ),
        )
        with open(readme_path, encoding="utf-8", mode="w", newline="\n") as readme:
            readme.writelines(content)
        return 0
    except FileNotFoundError:
        print("{}: could not modify README.md".format(bsp_name))
        return 1


def bsp_no_glib_all(bsp_names):
    ret = 0
    for bsp in bsp_names:
        bsp_path = Path(bsp)
        if not bsp_path.exists():
            print("[Error] Argument {} does not point to existing BSP".format(bsp))
            raise FileNotFoundError(f"BSP path does not exist: {bsp}")

        # 1. Move/copy the BSP to BSP_noglib
        noglib_path = Path(bsp + "_noglib")
        if os.environ.get("CI"):
            bsp_path = bsp_path.rename(noglib_path)
        else:
            if noglib_path.exists():
                shutil.rmtree(noglib_path)
            shutil.copytree(bsp_path, noglib_path)
            bsp_path = noglib_path

        # 2. Modify the configuration, dependencies and README
        ret += select_bsp_config_no_graphic_lib(bsp_path)
        ret += remove_esp_lvgl_port(bsp_path)
        ret += update_examples(bsp_path)
        ret += add_notice_to_readme(bsp_path)
        try:
            check_bsp_readme(bsp_path)
        except (ValueError, IOError, OSError) as e:
            # README.md was modified by the noglib generation, which is expected here
            print(f"[DEBUG] check_bsp_readme: {type(e).__name__}: {e}")
    return ret


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("bsp_names", nargs="*", help="BSP names to process")
    args = parser.parse_args()
    sys.exit(bsp_no_glib_all(args.bsp_names))
