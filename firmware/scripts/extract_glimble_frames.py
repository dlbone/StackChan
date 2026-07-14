#!/usr/bin/env python3
"""Extract the Glimble frames used by StackChan from a Codex v2 pet atlas."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


ATLAS_SIZE = (1536, 2288)
CELL_SIZE = (192, 208)
ANIMATIONS = {
    "idle": (0, 6),
    "ready": (3, 4),
    "blocked": (5, 8),
    "needs_input": (6, 6),
    "running": (7, 6),
}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("atlas", type=Path)
    parser.add_argument("output_directory", type=Path)
    args = parser.parse_args()

    with Image.open(args.atlas) as atlas:
        if atlas.size != ATLAS_SIZE:
            raise ValueError(f"expected a {ATLAS_SIZE[0]}x{ATLAS_SIZE[1]} Codex v2 atlas, got {atlas.size}")
        atlas = atlas.convert("RGBA")
        args.output_directory.mkdir(parents=True, exist_ok=True)
        expected_names: set[str] = set()
        for animation_name, (row, frame_count) in ANIMATIONS.items():
            for column in range(frame_count):
                frame_name = f"glimble_{animation_name}_{column}.png"
                expected_names.add(frame_name)
                left = column * CELL_SIZE[0]
                top = row * CELL_SIZE[1]
                frame = atlas.crop((left, top, left + CELL_SIZE[0], top + CELL_SIZE[1]))
                frame.save(args.output_directory / frame_name, format="PNG", optimize=True)

    for existing_frame in args.output_directory.glob("glimble_*.png"):
        if existing_frame.name not in expected_names:
            existing_frame.unlink()


if __name__ == "__main__":
    main()
