"""
StormBreaker Splash Screen Generator
Generates cinematic dark splash screens for Unreal Engine 5.8
"""

import math
import random
import os
from PIL import Image, ImageDraw, ImageFont, ImageFilter

random.seed(42)

OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))


def lerp_color(c1, c2, t):
    return tuple(int(c1[i] + (c2[i] - c1[i]) * t) for i in range(len(c1)))


def draw_gradient_bg(draw, w, h):
    """Dark navy-to-black diagonal gradient"""
    top_left = (10, 15, 35)
    top_right = (5, 8, 20)
    bot_left = (3, 5, 12)
    bot_right = (0, 0, 5)

    for y in range(h):
        ty = y / max(1, h - 1)
        for x in range(w):
            tx = x / max(1, w - 1)
            top = lerp_color(top_left, top_right, tx)
            bot = lerp_color(bot_left, bot_right, tx)
            c = lerp_color(top, bot, ty)
            draw.point((x, y), fill=c)


def draw_noise(img, w, h, intensity=8):
    """Subtle noise texture"""
    pixels = img.load()
    for y in range(h):
        for x in range(w):
            r, g, b = pixels[x, y][:3]
            noise = random.randint(-intensity, intensity)
            r = max(0, min(255, r + noise))
            g = max(0, min(255, g + noise))
            b = max(0, min(255, b + noise))
            pixels[x, y] = (r, g, b)


def draw_light_streaks(draw, w, h, count=5):
    """Diagonal light streaks for storm feel"""
    for _ in range(count):
        x_start = random.randint(0, w)
        y_start = 0
        length = random.randint(h // 2, h)
        angle = random.uniform(1.1, 1.4)  # near-vertical diagonal
        opacity = random.randint(6, 18)

        for t in range(length):
            x = int(x_start + t * math.cos(angle) * 0.3)
            y = int(y_start + t * math.sin(angle))
            if 0 <= x < w and 0 <= y < h:
                streak_width = random.randint(1, 3)
                for dx in range(-streak_width, streak_width + 1):
                    px = x + dx
                    if 0 <= px < w:
                        fade = max(0, opacity - abs(dx) * 4)
                        draw.point((px, y), fill=(140, 180, 255, fade))


def draw_vignette(img, w, h, strength=0.7):
    """Dark vignette edges"""
    pixels = img.load()
    cx, cy = w / 2, h / 2
    max_dist = math.sqrt(cx * cx + cy * cy)

    for y in range(h):
        for x in range(w):
            dist = math.sqrt((x - cx) ** 2 + (y - cy) ** 2)
            factor = 1.0 - (dist / max_dist) * strength
            factor = max(0.0, min(1.0, factor))
            r, g, b = pixels[x, y][:3]
            pixels[x, y] = (int(r * factor), int(g * factor), int(b * factor))


def draw_grid(draw, w, h, spacing=60, opacity=10):
    """Faint grid texture"""
    grid_color = (80, 120, 180, opacity)
    for x in range(0, w, spacing):
        draw.line([(x, 0), (x, h)], fill=grid_color, width=1)
    for y in range(0, h, spacing):
        draw.line([(0, y), (w, y)], fill=grid_color, width=1)


def draw_accent_line(draw, cx, y, length, thickness=2):
    """Electric blue accent line"""
    x1 = cx - length // 2
    x2 = cx + length // 2
    # Main line
    draw.rectangle([x1, y, x2, y + thickness], fill=(30, 140, 255))
    # Glow
    for i in range(1, 4):
        alpha = max(0, 60 - i * 20)
        draw.rectangle([x1 + i, y - i, x2 - i, y + thickness + i],
                        fill=(30, 140, 255, alpha))


def get_font(size, bold=True):
    """Get the best available font"""
    font_paths = [
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/calibrib.ttf",
    ]
    if not bold:
        font_paths = [
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/segoeui.ttf",
            "C:/Windows/Fonts/calibri.ttf",
        ]

    for path in font_paths:
        if os.path.exists(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def draw_metallic_text(img, draw, text, font, cx, cy, glow_color=(30, 100, 200)):
    """Draw text with metallic silver-blue gradient and glow"""
    # Get text bounding box
    bbox = font.getbbox(text)
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]
    tx = cx - tw // 2
    ty = cy - th // 2

    # Glow layer (draw text in glow color, blur it)
    glow_layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow_layer)
    glow_draw.text((tx, ty), text, font=font, fill=(*glow_color, 120))
    glow_layer = glow_layer.filter(ImageFilter.GaussianBlur(radius=8))
    img.paste(Image.alpha_composite(img.convert("RGBA"), glow_layer).convert("RGB"))

    # Gradient text: create text mask then fill with gradient
    text_layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    text_draw = ImageDraw.Draw(text_layer)

    # Draw character by character with vertical gradient
    for i, char in enumerate(text):
        char_bbox = font.getbbox(char)
        char_w = char_bbox[2] - char_bbox[0]

        # Vertical gradient per character: silver top → blue-silver bottom
        char_img = Image.new("RGBA", (char_w + 10, th + 10), (0, 0, 0, 0))
        char_draw = ImageDraw.Draw(char_img)
        char_draw.text((0, 0), char, font=font, fill=(255, 255, 255, 255))

        # Apply gradient tint
        pixels = char_img.load()
        for py in range(char_img.height):
            t = py / max(1, char_img.height - 1)
            top_color = (220, 230, 245)  # Silver
            bot_color = (140, 170, 220)  # Blue-silver
            tint = lerp_color(top_color, bot_color, t)
            for px in range(char_img.width):
                r, g, b, a = pixels[px, py]
                if a > 0:
                    pixels[px, py] = (
                        int(tint[0] * a / 255),
                        int(tint[1] * a / 255),
                        int(tint[2] * a / 255),
                        a
                    )

        # Get current x offset for this character
        preceding = text[:i]
        if preceding:
            p_bbox = font.getbbox(preceding)
            char_x = tx + p_bbox[2] - p_bbox[0]
        else:
            char_x = tx

        text_layer.paste(char_img, (char_x, ty), char_img)

    # Composite text onto main image
    result = Image.alpha_composite(img.convert("RGBA"), text_layer)
    img.paste(result.convert("RGB"))


def generate_splash(width, height, filename_base):
    """Generate a single splash screen"""
    print(f"Generating {width}x{height} splash: {filename_base}")

    img = Image.new("RGB", (width, height), (0, 0, 0))
    draw = ImageDraw.Draw(img, "RGBA")

    # 1. Gradient background
    draw_gradient_bg(draw, width, height)

    # 2. Grid texture
    grid_spacing = max(30, width // 25)
    draw_grid(draw, width, height, spacing=grid_spacing, opacity=8)

    # 3. Light streaks
    streak_count = max(3, width // 300)
    draw_light_streaks(draw, width, height, count=streak_count)

    # 4. Noise
    draw_noise(img, width, height, intensity=5)

    # 5. Vignette
    draw_vignette(img, width, height, strength=0.65)

    # 6. Title text
    title_size = max(24, width // 14)
    title_font = get_font(title_size, bold=True)
    title_y = int(height * 0.35)
    draw_metallic_text(img, draw, "STORMBREAKER", title_font, width // 2, title_y)

    # 7. Tagline
    tag_size = max(12, width // 40)
    tag_font = get_font(tag_size, bold=False)
    tag_y = title_y + title_size // 2 + tag_size
    tag_bbox = tag_font.getbbox("BATTLE ROYALE")
    tag_w = tag_bbox[2] - tag_bbox[0]
    draw.text(
        (width // 2 - tag_w // 2, tag_y),
        "BATTLE ROYALE",
        font=tag_font,
        fill=(160, 175, 200, 200)
    )

    # 8. Accent line under title
    accent_y = tag_y + tag_size + 8
    accent_len = min(width // 3, 400)
    draw_accent_line(draw, width // 2, accent_y, accent_len, thickness=2)

    # 9. Bottom-right: engine version
    ver_size = max(10, width // 80)
    ver_font = get_font(ver_size, bold=False)
    ver_text = "Unreal Engine 5.8"
    ver_bbox = ver_font.getbbox(ver_text)
    ver_w = ver_bbox[2] - ver_bbox[0]
    draw.text(
        (width - ver_w - 15, height - ver_size - 12),
        ver_text,
        font=ver_font,
        fill=(80, 90, 110, 120)
    )

    # 10. Bottom-left: copyright
    copy_text = "StormBreaker Games"
    draw.text(
        (15, height - ver_size - 12),
        copy_text,
        font=ver_font,
        fill=(60, 70, 85, 100)
    )

    # Save
    bmp_path = os.path.join(OUTPUT_DIR, f"{filename_base}.bmp")
    png_path = os.path.join(OUTPUT_DIR, f"{filename_base}.png")

    img.save(bmp_path, "BMP")
    img.save(png_path, "PNG")
    print(f"  -> {bmp_path}")
    print(f"  -> {png_path}")

    return img


def main():
    print("=" * 50)
    print("StormBreaker Splash Screen Generator")
    print("=" * 50)

    # Game Splash (1920x1080)
    generate_splash(1920, 1080, "Splash")

    # Editor Splash (600x200)
    generate_splash(600, 200, "EdSplash")

    # Android splash screens
    android_sizes = {
        "SplashAndroid_480x800": (480, 800),
        "SplashAndroid_720x1280": (720, 1280),
        "SplashAndroid_1080x1920": (1080, 1920),
        "SplashAndroid_1440x2560": (1440, 2560),
    }

    for name, (w, h) in android_sizes.items():
        generate_splash(w, h, name)

    print()
    print("=" * 50)
    print("All splash screens generated successfully!")
    print("=" * 50)


if __name__ == "__main__":
    main()
