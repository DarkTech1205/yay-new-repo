"""Original Geometry-Dash-styled artwork for the mod (procedural; no image generator and no GD assets are used).
Run from the project root:  python3 docs/make_art.py   -> writes resources/gcu-*.png (not the intro frames)

Style rules (taken from GD 2.2 UI): thick near-black outline, vertical light->dark gradients, a bright inner highlight on
the top edge, saturated colour, hard drop shadow. Everything is drawn at 4x and downsampled (LANCZOS) so edges stay crisp,
and every icon keeps a transparent background."""
import math, random
from PIL import Image, ImageDraw, ImageFont, ImageFilter
random.seed(221)
SS = 4
OUT = (10, 6, 24, 255)

def font(sz):
    for p in ("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", "/usr/share/fonts/dejavu/DejaVuSans-Bold.ttf"):
        try: return ImageFont.truetype(p, sz)
        except Exception: pass
    return ImageFont.load_default()

def lerp(a, b, t): return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(len(a)))

def vgrad(w, h, top, bot):
    g = Image.new("RGBA", (w, h)); d = ImageDraw.Draw(g)
    for y in range(h): d.line([(0, y), (w, y)], fill=lerp(top, bot, y / max(1, h - 1)) + (255,))
    return g

def canvas(w, h): return Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))
def finish(im, name, w, h):
    im.resize((w, h), Image.LANCZOS).save(f"resources/{name}.png", optimize=True); print("wrote", name)

def rrect_grad(im, box, r, top, bot, outline=OUT, ow=None, shadow=True, hl=True):
    """GD-style panel: shadow, gradient body, black outline, top highlight."""
    x0, y0, x1, y1 = [int(v) for v in box]; w, h = x1 - x0, y1 - y0
    ow = ow or max(2, int(min(w, h) * .06))
    if shadow:
        sh = Image.new("RGBA", im.size, (0, 0, 0, 0)); ImageDraw.Draw(sh).rounded_rectangle([x0 + ow, y0 + ow * 1.6, x1 + ow, y1 + ow * 1.6], r, fill=(0, 0, 0, 110))
        im.alpha_composite(sh.filter(ImageFilter.GaussianBlur(ow * .6)))
    body = vgrad(w, h, top, bot); m = Image.new("L", (w, h), 0); ImageDraw.Draw(m).rounded_rectangle([0, 0, w - 1, h - 1], r, fill=255)
    layer = Image.new("RGBA", im.size, (0, 0, 0, 0)); layer.paste(body, (x0, y0), m); im.alpha_composite(layer)
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([x0, y0, x1 - 1, y1 - 1], r, outline=outline, width=ow)
    if hl:
        d.rounded_rectangle([x0 + ow * 1.5, y0 + ow * 1.5, x1 - ow * 1.5, y0 + h * .42], max(2, r - ow), outline=(255, 255, 255, 90), width=max(1, ow // 2))
    return d

def poly(im, pts, fill, outline=OUT, ow=6):
    d = ImageDraw.Draw(im); d.polygon(pts, fill=fill)
    d.line(list(pts) + [pts[0]], fill=outline, width=ow, joint="curve")

def tstroke(im, xy, text, sz, fill, stroke=None, anchor="mm", sw=None):
    d = ImageDraw.Draw(im); sw = sw or max(2, sz // 8)
    d.text((xy[0] + sw, xy[1] + sw * 1.2), text, font=font(sz), fill=(0, 0, 0, 120), anchor=anchor, stroke_width=sw, stroke_fill=(0, 0, 0, 120))
    d.text(xy, text, font=font(sz), fill=fill, anchor=anchor, stroke_width=sw, stroke_fill=stroke or OUT)

def spike(d, x, y, w, h, c1, c2, ow=5):
    d.polygon([(x, y), (x + w / 2, y - h), (x + w, y)], fill=c1, outline=OUT)
    d.polygon([(x + w / 2, y - h), (x + w, y), (x + w / 2, y)], fill=c2)
    d.line([(x, y), (x + w / 2, y - h), (x + w, y), (x, y)], fill=OUT, width=ow, joint="curve")

# ------------------------------------------------------------------ currency icons (128x128)
def cash():
    W = 128; im = canvas(W, W) ; s = SS
    d = rrect_grad(im, (10 * s, 30 * s, 118 * s, 100 * s), 12 * s, (120, 240, 110), (28, 150, 60))
    d.rounded_rectangle([20 * s, 40 * s, 108 * s, 90 * s], 8 * s, outline=(255, 255, 255, 120), width=2 * s)
    tstroke(im, (64 * s, 66 * s), "$", 56 * s, (255, 240, 120), sw=4 * s)
    for cx in (28, 100): d.ellipse([(cx - 5) * s, 60 * s, (cx + 5) * s, 72 * s], fill=(20, 120, 50, 255), outline=OUT, width=2 * s)
    finish(im, "gcu-cash", W, W)

def gem():
    W = 128; im = canvas(W, W); s = SS; d = ImageDraw.Draw(im)
    top = [(34, 30), (94, 30), (118, 58), (64, 116), (10, 58)]
    pts = [(x * s, y * s) for x, y in top]
    sh = Image.new("RGBA", im.size, (0, 0, 0, 0)); ImageDraw.Draw(sh).polygon([(x + 8 * s, y + 12 * s) for x, y in pts], fill=(0, 0, 0, 110)); im.alpha_composite(sh.filter(ImageFilter.GaussianBlur(5 * s)))
    d = ImageDraw.Draw(im); d.polygon(pts, fill=(255, 70, 170))
    facets = [([(34, 30), (94, 30), (80, 58), (48, 58)], (255, 170, 225)), ([(34, 30), (48, 58), (10, 58)], (255, 105, 190)), ([(94, 30), (118, 58), (80, 58)], (230, 40, 140)),
              ([(10, 58), (48, 58), (64, 116)], (205, 30, 125)), ([(48, 58), (80, 58), (64, 116)], (255, 80, 175)), ([(80, 58), (118, 58), (64, 116)], (170, 20, 105))]
    for p, c in facets: d.polygon([(x * s, y * s) for x, y in p], fill=c)
    d.line(pts + [pts[0]], fill=OUT, width=6 * s, joint="curve")
    for p, _ in facets: d.line([(x * s, y * s) for x, y in p] + [(p[0][0] * s, p[0][1] * s)], fill=(60, 0, 40, 255), width=2 * s)
    d.polygon([(46 * s, 36 * s), (62 * s, 36 * s), (54 * s, 50 * s)], fill=(255, 255, 255, 190))
    finish(im, "gcu-gem", W, W)

def energy():
    W = 128; im = canvas(W, W); s = SS; d = ImageDraw.Draw(im)
    bolt = [(76, 8), (30, 70), (58, 70), (44, 120), (100, 50), (68, 50), (88, 8)]
    pts = [(x * s, y * s) for x, y in bolt]
    sh = Image.new("RGBA", im.size, (0, 0, 0, 0)); ImageDraw.Draw(sh).polygon([(x + 7 * s, y + 10 * s) for x, y in pts], fill=(0, 0, 0, 120)); im.alpha_composite(sh.filter(ImageFilter.GaussianBlur(4 * s)))
    body = vgrad(W * s, W * s, (255, 250, 150), (255, 170, 20)); m = Image.new("L", im.size, 0); ImageDraw.Draw(m).polygon(pts, fill=255)
    lay = Image.new("RGBA", im.size, (0, 0, 0, 0)); lay.paste(body, (0, 0), m); im.alpha_composite(lay)
    d = ImageDraw.Draw(im); d.line(pts + [pts[0]], fill=OUT, width=7 * s, joint="curve")
    d.line([(74 * s, 16 * s), (42 * s, 62 * s)], fill=(255, 255, 255, 150), width=3 * s)
    finish(im, "gcu-energy", W, W)

def demon():
    W = 96; im = canvas(W, W); s = SS; d = ImageDraw.Draw(im)
    for sx in (-1, 1):   # horns
        poly(im, [((48 + sx * 18) * s, 30 * s), ((48 + sx * 40) * s, 4 * s), ((48 + sx * 32) * s, 40 * s)], (255, 210, 80), ow=4 * s)
    rrect_grad(im, (10 * s, 22 * s, 86 * s, 90 * s), 30 * s, (255, 90, 70), (170, 15, 25), ow=5 * s)
    d = ImageDraw.Draw(im)
    for sx in (-1, 1):
        d.polygon([((48 + sx * 6) * s, 50 * s), ((48 + sx * 28) * s, 44 * s), ((48 + sx * 24) * s, 60 * s)], fill=(255, 255, 255), outline=OUT)
        d.ellipse([(48 + sx * 15 - 3) * s, 50 * s, (48 + sx * 15 + 3) * s, 57 * s], fill=OUT)
    d.rectangle([32 * s, 72 * s, 64 * s, 80 * s], fill=(255, 255, 255), outline=OUT, width=3 * s)
    for i in range(1, 4): d.line([(32 + i * 8) * s, 72 * s, (32 + i * 8) * s, 80 * s], fill=OUT, width=2 * s)
    finish(im, "gcu-demon", W, W)

# ------------------------------------------------------------------ logo, promo, cosmetic
def logo():
    W, H = 512, 192; im = canvas(W, H); s = SS
    rrect_grad(im, (6 * s, 20 * s, 506 * s, 180 * s), 26 * s, (60, 120, 255), (20, 30, 130), ow=9 * s)
    d = ImageDraw.Draw(im)
    for x in range(20, 500, 40): d.line([(x * s, 30 * s), (x * s, 170 * s)], fill=(255, 255, 255, 18), width=2 * s)
    tstroke(im, (306 * s, 78 * s), "UBISOFT DASH", 46 * s, (255, 240, 90), sw=7 * s)
    tstroke(im, (306 * s, 138 * s), "PREMIUM CORPORATE EDITION", 20 * s, (255, 255, 255), sw=4 * s)
    d = ImageDraw.Draw(im); d.rounded_rectangle([28 * s, 52 * s, 86 * s, 110 * s], 8 * s, fill=(255, 220, 40, 255), outline=OUT, width=6 * s)
    d.rectangle([42 * s, 68 * s, 52 * s, 80 * s], fill=OUT); d.rectangle([62 * s, 68 * s, 72 * s, 80 * s], fill=OUT); d.rectangle([42 * s, 92 * s, 72 * s, 98 * s], fill=OUT)
    finish(im, "gcu-logo", W, H)

def promo():
    W = 256; im = canvas(W, W); s = SS; d = ImageDraw.Draw(im); cx = cy = 128 * s
    pts = []
    for i in range(32):
        r = (120 if i % 2 == 0 else 92) * s; a = math.pi * 2 * i / 32 - math.pi / 2
        pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    poly(im, pts, (255, 70, 70), ow=7 * s)
    d = ImageDraw.Draw(im); d.ellipse([cx - 82 * s, cy - 82 * s, cx + 82 * s, cy + 82 * s], fill=(255, 215, 40), outline=OUT, width=6 * s)
    tstroke(im, (cx, cy - 16 * s), "-90%", 56 * s, (255, 255, 255), sw=6 * s)
    tstroke(im, (cx, cy + 36 * s), "TODAY ONLY", 22 * s, (200, 20, 40), stroke=(255, 255, 255), sw=3 * s)
    finish(im, "gcu-promo", W, W)

def premium():
    W = 128; im = canvas(W, W); s = SS
    rrect_grad(im, (16 * s, 24 * s, 112 * s, 120 * s), 12 * s, (190, 120, 255), (90, 40, 200), ow=7 * s)
    d = ImageDraw.Draw(im); 
    for sx in (-1, 1): d.rectangle([(64 + sx * 22 - 8) * s, 60 * s, (64 + sx * 22 + 8) * s, 78 * s], fill=OUT)
    d.rectangle([44 * s, 92 * s, 84 * s, 98 * s], fill=OUT)
    pts = []
    for i in range(10):
        r = (22 if i % 2 == 0 else 9) * s; a = math.pi * 2 * i / 10 - math.pi / 2; pts.append((64 * s + r * math.cos(a), 18 * s + r * math.sin(a)))
    poly(im, pts, (255, 235, 90), ow=4 * s)
    finish(im, "gcu-premium-cosmetic", W, W)

# ------------------------------------------------------------------ backgrounds 512x384 (GD menu look)
def bg(name, top, bot, accent, extra=None):
    W, H = 512, 384; im = vgrad(W, H, top, bot).convert("RGB"); d = ImageDraw.Draw(im, "RGBA")   # RGB so translucent fills really blend
    for _ in range(20):                                  # big soft background squares, like GD's menu backgrounds
        x, y, sz = random.randint(-20, 480), random.randint(-20, 300), random.randint(40, 120)
        d.rectangle([x, y, x + sz, y + sz], fill=accent + (22,), outline=accent + (40,), width=2)
    for x in range(0, W, 32): d.line([(x, 0), (x, H)], fill=(255, 255, 255, 14))
    d.rectangle([0, H - 40, W, H], fill=lerp(bot, (0, 0, 0), .45) + (255,))   # ground
    d.line([(0, H - 40), (W, H - 40)], fill=accent + (255,), width=3)
    for x in range(0, W, 64): d.line([(x, H - 40), (x, H)], fill=(0, 0, 0, 60), width=2)
    if extra: extra(d)
    vg = Image.new("RGBA", (W, H), (0, 0, 0, 0)); vd = ImageDraw.Draw(vg)
    for i in range(30): vd.rectangle([i, i, W - i, H - i], outline=(0, 0, 0, 3 + i // 4), width=1)
    im = im.convert("RGBA"); im.alpha_composite(vg); im.convert("RGB").save(f"resources/{name}.png", optimize=True); print("wrote", name)

def runner(d, x, y, col): d.rectangle([x, y, x + 24, y + 24], fill=col, outline=OUT, width=3)

def make_bgs():
    bg("gcu-shop-bg", (255, 90, 150), (110, 50, 255), (255, 225, 77), lambda d: runner(d, 60, 296, (120, 255, 120)))
    bg("gcu-mod-bg", (20, 40, 110), (8, 12, 40), (111, 209, 255), lambda d: [d.rectangle([0, 0, 512, 20], fill=(255, 210, 63, 220)), d.line([(0, 20), (512, 20)], fill=OUT, width=3)])
    bg("gcu-robtop-bg", (40, 22, 0), (12, 6, 0), (255, 176, 0), lambda d: [d.text((18, 36 + i * 28), "> SUDO MONETIZE --ALL" if i % 2 == 0 else "> LICENSE: $9.99/MO", font=font(18), fill=(255, 176, 0, 140)) for i in range(10)])
    bg("gcu-pass-bg", (150, 70, 255), (255, 90, 170), (255, 225, 77), lambda d: runner(d, 90, 296, (255, 220, 60)))
    bg("gcu-treasure-bg", (60, 28, 90), (30, 12, 40), (255, 210, 63), lambda d: [d.polygon([(x, 344), (x + 18, 318), (x + 36, 344)], fill=(255, 210, 63, 90), outline=OUT) for x in range(20, 500, 90)])

# ------------------------------------------------------------------ level icons and 2.21 pack art
def chains(d, xs, y0, y1, s):
    for x in xs:
        for y in range(int(y0), int(y1), 14 * s):
            d.rounded_rectangle([x - 4 * s, y, x + 4 * s, y + 11 * s], 4 * s, fill=(70, 50, 110, 255), outline=(12, 8, 30, 255), width=2 * s)

def lvl_explorers():
    W = 128; im = canvas(W, W); s = SS
    rrect_grad(im, (6 * s, 6 * s, 122 * s, 122 * s), 16 * s, (95, 40, 170), (30, 10, 70), ow=7 * s)
    d = ImageDraw.Draw(im); chains(d, (30, 98), 14 * s, 112 * s, s)
    for cx, cy, c in ((44, 74, (255, 60, 230)), (86, 50, (60, 255, 130))):      # orbs
        d.ellipse([(cx - 14) * s, (cy - 14) * s, (cx + 14) * s, (cy + 14) * s], fill=c, outline=(255, 255, 255, 255), width=3 * s)
        d.ellipse([(cx - 7) * s, (cy - 7) * s, (cx + 7) * s, (cy + 7) * s], outline=(255, 255, 255, 220), width=2 * s)
    poly(im, [(64 * s, 16 * s), (92 * s, 36 * s), (64 * s, 58 * s), (36 * s, 36 * s)], (230, 20, 90), ow=4 * s)     # skull diamond
    d = ImageDraw.Draw(im); d.rectangle([52 * s, 28 * s, 76 * s, 44 * s], fill=(255, 255, 255)); d.rectangle([56 * s, 31 * s, 61 * s, 37 * s], fill=OUT); d.rectangle([67 * s, 31 * s, 72 * s, 37 * s], fill=OUT)
    for i in range(6): spike(d, (14 + i * 16) * s, 114 * s, 14 * s, 18 * s, (140, 255, 40), (60, 190, 30), 3 * s)
    finish(im, "gcu-lvl-explorers", W, W)

def bricks(d, x0, y0, x1, y1, base, s, line):
    d.rectangle([x0, y0, x1, y1], fill=base)
    row = 0
    for y in range(y0, y1, 8 * s):
        d.line([(x0, y), (x1, y)], fill=line, width=s)
        for x in range(x0 + (row % 2) * 8 * s, x1, 16 * s): d.line([(x, y), (x, min(y + 8 * s, y1))], fill=line, width=s)
        row += 1

def flame(d, cx, cy, s, k=1.0):
    for r, c in ((10, (255, 80, 30)), (7, (255, 160, 30)), (4, (255, 235, 100))):
        d.polygon([(cx - r * s * k, cy), (cx, cy - r * 2 * s * k), (cx + r * s * k, cy), (cx, cy + r * .6 * s * k)], fill=c)

def lvl_firebird():
    W = 128; im = canvas(W, W); s = SS
    rrect_grad(im, (6 * s, 6 * s, 122 * s, 122 * s), 16 * s, (150, 20, 30), (45, 5, 15), ow=7 * s)
    d = ImageDraw.Draw(im); bricks(d, 40 * s, 12 * s, 88 * s, 66 * s, (255, 60, 20, 255), s, (60, 5, 5, 255))
    d.rectangle([40 * s, 62 * s, 88 * s, 70 * s], fill=(255, 215, 60)); d.rectangle([40 * s, 12 * s, 88 * s, 70 * s], outline=OUT, width=4 * s)
    for x in (48, 60, 72): d.line([x * s, 70 * s, x * s, (80 + (x % 7) * 2) * s], fill=(255, 150, 30), width=2 * s)
    for cx in (26, 102):
        d.rectangle([(cx - 5) * s, 50 * s, (cx + 5) * s, 58 * s], fill=(90, 60, 40), outline=OUT, width=2 * s); flame(d, cx * s, 44 * s, s, 1.0)
    for i in range(6): spike(d, (14 + i * 16) * s, 114 * s, 14 * s, 20 * s, (255, 220, 70), (230, 110, 20), 3 * s)
    finish(im, "gcu-lvl-firebird", W, W)

def pack221():
    W = 256; im = canvas(W, W); s = SS
    # frozen-map base (identity of the original 2.21 art): ice-blue frame with crystals
    rrect_grad(im, (8 * s, 8 * s, 248 * s, 248 * s), 26 * s, (110, 205, 255), (25, 90, 200), ow=10 * s)
    d = ImageDraw.Draw(im)
    for x, h in ((34, 40), (58, 62), (196, 70), (222, 46)):
        poly(im, [(x * s, 96 * s), ((x + 10) * s, (96 - h) * s), ((x + 22) * s, 96 * s)], (200, 245, 255), ow=4 * s)
    # two portals: Firebird (left, red/brick/spikes) and Explorers (right, purple/chains/skull)
    for side in (0, 1):
        x0 = 20 + side * 114
        top, bot = ((150, 25, 35), (50, 6, 16)) if side == 0 else ((105, 45, 185), (34, 12, 80))
        rrect_grad(im, (x0 * s, 104 * s, (x0 + 102) * s, 236 * s), 14 * s, top, bot, ow=6 * s, shadow=False)
        d = ImageDraw.Draw(im)
        if side == 0:
            bricks(d, (x0 + 28) * s, 110 * s, (x0 + 74) * s, 158 * s, (255, 70, 25, 255), s, (60, 5, 5, 255))
            d.rectangle([(x0 + 28) * s, 110 * s, (x0 + 74) * s, 158 * s], outline=OUT, width=3 * s)
            for cx in (x0 + 14, x0 + 88): flame(d, cx * s, 150 * s, s, .8)
            for i in range(5): spike(d, (x0 + 6 + i * 18) * s, 230 * s, 16 * s, 26 * s, (255, 220, 70), (230, 110, 20), 3 * s)
        else:
            chains(d, (x0 + 22, x0 + 80), 110 * s, 226 * s, s)
            poly(im, [((x0 + 51) * s, 118 * s), ((x0 + 74) * s, 140 * s), ((x0 + 51) * s, 162 * s), ((x0 + 28) * s, 140 * s)], (230, 20, 90), ow=3 * s)
            d = ImageDraw.Draw(im); d.rectangle([(x0 + 41) * s, 132 * s, (x0 + 61) * s, 146 * s], fill=(255, 255, 255)); d.rectangle([(x0 + 44) * s, 135 * s, (x0 + 49) * s, 141 * s], fill=OUT); d.rectangle([(x0 + 53) * s, 135 * s, (x0 + 58) * s, 141 * s], fill=OUT)
            for cx, cy, c in ((x0 + 30, 186, (255, 60, 230)), (x0 + 72, 172, (60, 255, 130))):
                d.ellipse([(cx - 11) * s, (cy - 11) * s, (cx + 11) * s, (cy + 11) * s], fill=c, outline=(255, 255, 255), width=3 * s)
            for i in range(5): spike(d, (x0 + 6 + i * 18) * s, 230 * s, 16 * s, 24 * s, (140, 255, 40), (60, 190, 30), 3 * s)
    # title
    tstroke(im, (140 * s, 46 * s), "2.21", 56 * s, (255, 235, 70), sw=7 * s)
    tstroke(im, (140 * s, 84 * s), "PACK", 22 * s, (255, 255, 255), sw=4 * s)
    # map scroll badge (top-left corner)
    d = ImageDraw.Draw(im); d.rounded_rectangle([14 * s, 14 * s, 50 * s, 46 * s], 5 * s, fill=(250, 220, 150), outline=OUT, width=4 * s)
    d.line([(20 * s, 36 * s), (30 * s, 26 * s), (36 * s, 34 * s), (44 * s, 22 * s)], fill=(200, 40, 40), width=2 * s)
    finish(im, "gcu-pack-221", W, W)

def ad_panel():
    W, H = 580, 232; im = canvas(W, H); s = SS
    rrect_grad(im, (6 * s, 6 * s, 574 * s, 226 * s), 24 * s, (255, 255, 255), (170, 170, 190), ow=8 * s)     # greyscale: tinted per ad in code
    d = ImageDraw.Draw(im); d.rounded_rectangle([20 * s, 20 * s, 560 * s, 212 * s], 16 * s, outline=(255, 215, 50, 255), width=4 * s)
    for x in range(30, 560, 36): d.line([(x * s, 24 * s), ((x - 24) * s, 208 * s)], fill=(255, 255, 255, 40), width=6 * s)
    finish(im, "gcu-ad-panel", W, H)

if __name__ == "__main__":
    cash(); gem(); energy(); demon(); logo(); promo(); premium(); make_bgs()
    lvl_explorers(); lvl_firebird(); pack221(); ad_panel()
