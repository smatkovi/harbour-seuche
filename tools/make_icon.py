#!/usr/bin/env python3
"""Generate the application icon.

A dark globe with the four disease colours on it — placeholder artwork, but it
gives the package a real icon in every size Sailfish asks for.

Usage:  python3 tools/make_icon.py
"""
import os

from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SIZES = (86, 108, 128, 172, 256)
SUPERSAMPLE = 4

SEA = (16, 24, 32, 255)
LAND = (38, 54, 62, 255)
DISEASE = [(76, 143, 212, 255), (221, 180, 58, 255), (138, 138, 150, 255), (212, 84, 76, 255)]


def draw_icon(size):
    big = size * SUPERSAMPLE
    image = Image.new("RGBA", (big, big), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)

    margin = big * 0.06
    draw.ellipse((margin, margin, big - margin, big - margin), fill=SEA)

    # two land bands, just enough to read as a globe
    draw.ellipse((big * 0.16, big * 0.22, big * 0.62, big * 0.50), fill=LAND)
    draw.ellipse((big * 0.38, big * 0.52, big * 0.86, big * 0.80), fill=LAND)

    # one cube per disease, placed like the four regions of the board
    cube = big * 0.15
    spots = ((0.27, 0.30), (0.33, 0.62), (0.62, 0.32), (0.66, 0.63))
    for colour, (x, y) in zip(DISEASE, spots):
        left, top = big * x - cube / 2, big * y - cube / 2
        draw.rectangle((left, top, left + cube, top + cube), fill=colour,
                       outline=(0, 0, 0, 180), width=max(1, int(big * 0.006)))

    return image.resize((size, size), Image.LANCZOS)


def main():
    out = os.path.join(ROOT, "sailfish/icons")
    os.makedirs(out, exist_ok=True)
    for size in SIZES:
        path = os.path.join(out, "icon-%d.png" % size)
        draw_icon(size).save(path)
        print("wrote", path)


if __name__ == "__main__":
    main()
