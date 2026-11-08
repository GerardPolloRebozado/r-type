#!/usr/bin/env python3
"""
Openfront building icon generator (PIL, no external assets).

Crafts chunky pixel-art icons for the placed-in-map buildings at 32x32 logical
pixels (upscaled x2 nearest neighbour to 64x64 for crisp GPU downscaling):

    building_city.png     town: house cluster + tower + flag
    building_port.png     harbor: quay, crane, moored boat
    building_factory.png  plant: sawtooth hall + brick chimneys + smoke
    building_defense.png  bunker: sandbag ring + turret
    building_sam.png      launcher: radar dish + missile rail
    building_silo.png     silo: open hatch + warhead
    trade_train.png       freight convoy used as the animated trade marker

Light comes from the top-left; every icon gets a 1px dark outline and a soft
ground shadow. Regenerate:  python3 gen_icons.py
"""
from PIL import Image, ImageDraw
import os

OUT_DIR = os.path.dirname(os.path.abspath(__file__))
S = 32          # logical size
SCALE = 2       # upscaled factor

# ---------------------------------------------------------------- palette --
OUTLINE   = (34, 34, 46, 255)
# Solid ground pad instead of a translucent shadow: on tinted terrain the
# alpha shadow read like a dark pond under the building.
PAD       = (122, 112, 84, 255)    # trodden earth
PAD_RIM   = (86, 78, 58, 255)
WALL      = (234, 208, 164, 255)
WALL_SH   = (194, 162, 116, 255)
ROOF_R    = (192, 84, 70, 255)     # terracotta
ROOF_RSH  = (148, 56, 48, 255)
ROOF_B    = (98, 112, 140, 255)    # slate
ROOF_BSH  = (66, 78, 104, 255)
WOOD      = (146, 104, 66, 255)
WOOD_SH   = (108, 74, 46, 255)
STEEL     = (172, 182, 196, 255)
STEEL_SH  = (122, 132, 148, 255)
CONCRETE  = (202, 202, 198, 255)
CONCRETE_SH = (152, 156, 156, 255)
GUNMETAL  = (58, 62, 76, 255)
GUNMETAL_L= (92, 98, 114, 255)
BRICK     = (168, 88, 66, 255)
BRICK_SH  = (128, 62, 48, 255)
ACCENT    = (226, 70, 58, 255)     # signal red
ACCENT_SH = (166, 46, 40, 255)
GOLD      = (238, 186, 62, 255)
GOLD_SH   = (178, 132, 36, 255)
OLIVE     = (122, 132, 74, 255)
OLIVE_SH  = (90, 98, 52, 255)
SEA       = (44, 122, 188, 255)
SEA_SH    = (28, 82, 140, 255)
HULL      = (60, 66, 90, 255)
WINDOW    = (255, 222, 140, 255)
WHITE     = (245, 247, 250, 255)
STONE_FIX = (150, 155, 165, 255)
SMOKE     = (238, 240, 244, 255)


def grid():
    img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    dr = ImageDraw.Draw(img)
    return img, dr


def ground_shadow(dr, cx, cy, rx, ry):
    """Solid trodden-earth pad under the structure: rim, then lighter top."""
    dr.ellipse((cx - rx, cy - ry, cx + rx, cy + ry), fill=PAD_RIM)
    dr.ellipse((cx - rx + 1, cy - ry + 1, cx + rx, cy + ry), fill=PAD)


def px_rect(dr, a, color):
    dr.rectangle((a[0], a[1], a[2], a[3]), fill=color)


def outline_and_scale(img):
    """1px dark outline around the opaque silhouette, then x2 nearest."""
    w, h = img.size
    src = img.load()
    outline = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    dst = outline.load()
    for y in range(h):
        for x in range(w):
            if src[x, y][3] > 0:
                dst[x, y] = src[x, y]
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, ny = x + dx, y + dy
                if 0 <= nx < w and 0 <= ny < h and src[nx, ny][3] > 0:
                    dst[x, y] = OUTLINE
                    break
    # outline stays outside the silhouette, so keep art inside (1,1)-(30,30)
    return outline.resize((S * SCALE, S * SCALE), Image.NEAREST)


# --------------------------------------------------------------- city ------
def city():
    img, dr = grid()
    ground_shadow(dr, 16, 26, 12, 4)

    # plaza between houses
    dr.rectangle((13, 21, 19, 23), fill=(180, 172, 148, 255))

    # main house (bottom-left), walls + terracotta gable
    px_rect(dr, (4, 14, 12, 22), WALL)
    px_rect(dr, (4, 20, 12, 22), WALL_SH)
    px_rect(dr, (4, 14, 12, 17), ROOF_R)          # roof face
    px_rect(dr, (4, 14, 4, 22), ROOF_RSH)         # side shade
    dr.polygon([(4, 14), (8, 9), (12, 14)], fill=ROOF_R)
    dr.polygon([(8, 9), (12, 14), (8, 14)], fill=ROOF_RSH)
    dr.rectangle((6, 16, 7, 17), fill=WINDOW)
    dr.rectangle((9, 19, 10, 21), fill=OUTLINE)   # door

    # second house (bottom-right), slate roof
    px_rect(dr, (15, 15, 22, 23), WALL)
    px_rect(dr, (15, 21, 22, 23), WALL_SH)
    px_rect(dr, (15, 15, 22, 17), ROOF_B)
    px_rect(dr, (15, 15, 15, 23), ROOF_BSH)
    dr.polygon([(14, 15), (18, 12), (23, 16)], fill=ROOF_B)
    dr.rectangle((17, 19, 18, 21), fill=WINDOW)
    dr.rectangle((20, 18, 21, 20), fill=WINDOW)

    # small tower top-right with flag
    px_rect(dr, (21, 6, 26, 13), WALL)
    px_rect(dr, (21, 6, 26, 8), ROOF_R)
    px_rect(dr, (21, 8, 21, 13), WALL_SH)
    dr.rectangle((23, 10, 24, 11), fill=WINDOW)
    dr.line((26, 6, 26, 2), fill=OUTLINE)
    dr.rectangle((27, 2, 30, 4), fill=ACCENT)

    # tiny tree in the plaza shadow
    dr.rectangle((15, 12, 15, 14), fill=WOOD_SH)
    dr.ellipse((13, 9, 17, 13), fill=(74, 128, 58, 255))
    dr.ellipse((13, 11, 17, 13), fill=(58, 106, 46, 255))
    return outline_and_scale(img)


# --------------------------------------------------------------- port ------
def port():
    img, dr = grid()
    ground_shadow(dr, 14, 27, 10, 3)

    # boat on the right: hull with pointed bow, striped freeboard
    dr.polygon([(14, 11), (30, 15), (27, 22), (13, 20)], fill=HULL)
    dr.polygon([(14, 11), (30, 15), (29, 18), (14, 16)], fill=(84, 92, 118, 255))
    dr.rectangle((15, 16, 27, 20), fill=HULL)
    dr.line((14, 16, 28, 17), fill=(228, 116, 66, 255))       # boot stripe
    # deckhouse + funnel
    px_rect(dr, (18, 12, 24, 16), WHITE)
    dr.rectangle((19, 13, 23, 13), fill=(120, 170, 210, 255))  # glass band
    px_rect(dr, (24, 10, 27, 12), (232, 180, 60, 255))         # funnel
    dr.point((23, 8), fill=SMOKE)
    dr.point((25, 9), fill=SMOKE)

    # crane: thick mast + jib, hook line (no cargo: keep the silhouette clean)
    dr.rectangle((9, 8, 11, 24), fill=GUNMETAL_L)
    dr.rectangle((10, 7, 22, 10), fill=GUNMETAL)               # jib
    dr.rectangle((9, 6, 12, 9), fill=ACCENT)                   # cab
    dr.line((20, 10, 20, 13), fill=(40, 42, 52, 255))          # cable
    px_rect(dr, (19, 13, 21, 13), GUNMETAL)                    # hook

    # stone quay with bollards along the bottom
    px_rect(dr, (2, 22, 20, 27), CONCRETE)
    px_rect(dr, (2, 26, 20, 27), CONCRETE_SH)
    dr.rectangle((2, 22, 20, 22), fill=(226, 226, 222, 255))
    for bx in (5, 10, 15):
        dr.ellipse((bx - 1, 21, bx + 1, 23), fill=OUTLINE)     # bollard
    return outline_and_scale(img)


# ------------------------------------------------------------- factory -----
def factory():
    img, dr = grid()
    ground_shadow(dr, 15, 26, 12, 4)

    # brick chimney first (behind the hall) with drifting smoke
    px_rect(dr, (22, 10, 25, 26), BRICK)
    px_rect(dr, (22, 10, 22, 26), BRICK_SH)
    px_rect(dr, (22, 10, 25, 12), (110, 58, 44, 255))
    px_rect(dr, (27, 16, 29, 26), BRICK)
    for yy in range(13, 26, 3):
        dr.line((22, yy, 25, yy), fill=BRICK_SH)               # brick courses

    # main hall with low sawtooth roof
    px_rect(dr, (3, 17, 20, 26), STEEL)
    px_rect(dr, (3, 24, 20, 26), STEEL_SH)
    for i in range(3):
        x0 = 3 + i * 6
        dr.polygon([(x0, 17), (x0 + 5, 17), (x0 + 5, 12)], fill=(214, 222, 232, 255))
        dr.line((x0 + 5, 12, x0 + 5, 17), fill=STEEL_SH)
        dr.line((x0, 17, x0 + 5, 17), fill=STEEL_SH)
    # windows band + big rolling door
    for i in range(3):
        dr.rectangle((5 + i * 4, 20, 7 + i * 4, 22), fill=(150, 170, 190, 255))
    px_rect(dr, (16, 22, 19, 26), GUNMETAL)
    for yy in (23, 24, 25):
        dr.line((16, yy, 19, yy), fill=GUNMETAL_L)

    # smoke drifting up-left from the chimney top
    dr.ellipse((24, 6, 28, 9), fill=(SMOKE[0], SMOKE[1], SMOKE[2], 215))
    dr.ellipse((27, 3, 31, 6), fill=(SMOKE[0], SMOKE[1], SMOKE[2], 150))
    return outline_and_scale(img)


# ---------------------------------------------------------- defense post ---
def defense():
    img, dr = grid()
    ground_shadow(dr, 15, 24, 11, 4)

    # sandbag ring
    sacs = [(6, 18), (10, 16), (14, 15), (18, 16), (21, 18),
            (6, 21), (10, 23), (14, 24), (18, 23), (21, 21)]
    for sx, sy in sacs:
        dr.ellipse((sx, sy, sx + 4, sy + 3), fill=OLIVE)
        dr.ellipse((sx, sy + 1, sx + 4, sy + 3), fill=OLIVE_SH)

    # concrete bunker
    dr.ellipse((9, 13, 20, 24), fill=CONCRETE)
    dr.ellipse((10, 14, 19, 23), fill=(170, 172, 170, 255))
    dr.ellipse((12, 16, 17, 21), fill=GUNMETAL)

    # turret with barrel pointing up-right
    dr.ellipse((12, 15, 17, 20), fill=GUNMETAL_L)
    dr.line((15, 17, 22, 10), fill=(40, 42, 52, 255))
    dr.line((21, 9, 24, 10), fill=(40, 42, 52, 255))  # muzzle
    dr.rectangle((18, 17, 21, 19), fill=GUNMETAL_L)

    # pennant
    dr.line((11, 6, 11, 12), fill=OUTLINE)
    dr.rectangle((12, 6, 16, 8), fill=ACCENT)
    return outline_and_scale(img)


# ------------------------------------------------------------- SAM ---------
def sam():
    img, dr = grid()
    ground_shadow(dr, 15, 26, 12, 3)

    # launcher rail: solid angled trunk pointing up-right
    dr.polygon([(9, 20), (22, 7), (26, 9), (13, 22)], fill=GUNMETAL_L)
    dr.rectangle((22, 6, 27, 8), fill=ACCENT)                  # rail end cap

    # two missiles riding the rail, staggered
    for i in range(2):
        mx, my = 10 + i * 6, 19 - i * 6
        dr.line((mx, my, mx + 7, my - 7), fill=WHITE)          # body (2px thick)
        dr.line((mx, my - 1, mx + 7, my - 8), fill=WHITE)
        dr.polygon([(mx + 7, my - 7), (mx + 10, my - 11), (mx + 8, my - 8)],
                   fill=ACCENT)                                # red tip
        dr.line((mx - 1, my + 1, mx + 2, my + 1), fill=STONE_FIX)

    # tracked chassis
    px_rect(dr, (4, 21, 27, 26), GUNMETAL)
    px_rect(dr, (4, 25, 27, 26), (38, 40, 50, 255))
    for wx in (6, 12, 18, 24):
        dr.ellipse((wx, 24, wx + 3, 27), fill=(24, 26, 34, 255))
    px_rect(dr, (4, 21, 27, 21), GUNMETAL_L)

    # radar dish at the front
    dr.rectangle((17, 17, 18, 20), fill=GUNMETAL_L)
    dr.ellipse((13, 11, 19, 17), fill=WHITE)
    dr.ellipse((14, 12 , 18, 16), fill=(210, 216, 224, 255))
    dr.line((15, 14, 19, 14), fill=STONE_FIX)
    return outline_and_scale(img)


# ------------------------------------------------------------- silo --------
def silo():
    img, dr = grid()
    ground_shadow(dr, 15, 25, 12, 3)

    # concrete pad with warning stripes
    px_rect(dr, (4, 22, 27, 26), CONCRETE)
    for i in range(0, 24, 6):
        dr.rectangle((4 + i, 24, 7 + i, 26), fill=GOLD)
    px_rect(dr, (4, 25, 27, 26), CONCRETE_SH)

    # bunker ring
    dr.ellipse((6, 6, 25, 25), fill=CONCRETE)
    dr.ellipse((8, 8, 23, 23), fill=(176, 178, 176, 255))

    # open hatch with warhead peeking
    dr.ellipse((11, 9, 20, 18), fill=(58, 62, 76, 255))
    dr.ellipse((12, 10, 19, 17), fill=(44, 46, 58, 255))
    dr.polygon([(15, 13), (14, 16), (17, 16)], fill=ACCENT)   # warhead tip
    px_rect(dr, (15, 16, 15, 18), WHITE)
    dr.line((13, 17, 18, 17), fill=(120, 124, 136, 255))
    px_rect(dr, (13, 18, 18, 20), CONCRETE_SH)                 # hatch rim

    # small dish on the rim
    dr.line((24, 12, 24, 15), fill=GUNMETAL_L)
    dr.ellipse((22, 9, 27, 14), fill=WHITE)
    return outline_and_scale(img)


# ------------------------------------------------------- trade train -------
def trade_train():
    img = Image.new("RGBA", (30, 14), (0, 0, 0, 0))
    dr = ImageDraw.Draw(img)
    # rail shadow
    dr.line((1, 12, 28, 12), fill=PAD_RIM)
    # locomotive
    px_rect(dr, (1, 4, 8, 11), ACCENT)
    px_rect(dr, (1, 10, 8, 11), ACCENT_SH)
    px_rect(dr, (2, 5, 4, 7), (150, 200, 230, 255))
    dr.polygon([(8, 6), (11, 4), (11, 11), (8, 11)], fill=ACCENT_SH)
    # coal car
    px_rect(dr, (12, 6, 17, 11), GUNMETAL)
    px_rect(dr, (13, 4, 16, 6), (30, 30, 40, 255))
    # freight wagon
    px_rect(dr, (18, 5, 24, 11), WOOD)
    px_rect(dr, (18, 9, 24, 11), WOOD_SH)
    px_rect(dr, (6, 11, 7, 12), OUTLINE)
    for wx in (3, 6, 14, 20, 22):
        dr.ellipse((wx, 10, wx + 2, 12), fill=(28, 30, 38, 255))
    # wheels highlight
    return outline_and_scale(img)


def main():
    icons = {
        "building_city.png": city(),
        "building_port.png": port(),
        "building_factory.png": factory(),
        "building_defense.png": defense(),
        "building_sam.png": sam(),
        "building_silo.png": silo(),
        "trade_train.png": trade_train(),
    }
    for name, im in icons.items():
        path = os.path.join(OUT_DIR, name)
        im.save(path)
        print(f"wrote {path} ({im.size[0]}x{im.size[1]})")


if __name__ == "__main__":
    main()
