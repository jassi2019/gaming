"""
StormBreaker Animated Splash Screen Generator
Creates a cinematic MP4 splash video with:
- Fade from black
- Lightning flashes
- Logo reveal with glow pulse
- Tagline fade in
- Accent line draw animation
- Particle-like sparks
- Fade to black at end
"""

import cv2
import numpy as np
import math
import random
import os

random.seed(42)

WIDTH = 1920
HEIGHT = 1080
FPS = 30
DURATION = 5.0  # seconds
TOTAL_FRAMES = int(FPS * DURATION)

OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_PATH = os.path.join(OUTPUT_DIR, "SplashVideo.mp4")


def lerp(a, b, t):
    return a + (b - a) * max(0.0, min(1.0, t))


def ease_out(t):
    return 1.0 - (1.0 - t) ** 3


def ease_in_out(t):
    if t < 0.5:
        return 4 * t * t * t
    else:
        return 1 - (-2 * t + 2) ** 3 / 2


def draw_gradient_bg(frame):
    """Dark cinematic gradient"""
    for y in range(HEIGHT):
        ty = y / HEIGHT
        for x in range(WIDTH):
            tx = x / WIDTH
            r = int(lerp(lerp(10, 5, tx), lerp(3, 0, tx), ty))
            g = int(lerp(lerp(15, 8, tx), lerp(5, 0, tx), ty))
            b = int(lerp(lerp(35, 20, tx), lerp(12, 5, tx), ty))
            frame[y, x] = (b, g, r)  # BGR


def draw_vignette(frame, strength=0.6):
    """Radial vignette"""
    cx, cy = WIDTH // 2, HEIGHT // 2
    max_dist = math.sqrt(cx * cx + cy * cy)
    # Vectorized approach for speed
    Y, X = np.mgrid[0:HEIGHT, 0:WIDTH]
    dist = np.sqrt((X - cx) ** 2 + (Y - cy) ** 2)
    factor = 1.0 - (dist / max_dist) * strength
    factor = np.clip(factor, 0, 1)
    for c in range(3):
        frame[:, :, c] = (frame[:, :, c] * factor).astype(np.uint8)


def draw_grid(frame, opacity=0.03, spacing=80):
    """Faint grid"""
    overlay = frame.copy()
    color = (120, 100, 60)  # BGR: blue-ish
    for x in range(0, WIDTH, spacing):
        cv2.line(overlay, (x, 0), (x, HEIGHT), color, 1)
    for y in range(0, HEIGHT, spacing):
        cv2.line(overlay, (0, y), (WIDTH, y), color, 1)
    cv2.addWeighted(overlay, opacity, frame, 1 - opacity, 0, frame)


def draw_lightning(frame, intensity):
    """Random lightning flash"""
    if intensity <= 0:
        return
    flash = np.full_like(frame, int(255 * intensity))
    # Tint blue
    flash[:, :, 0] = np.clip(flash[:, :, 0] * 1.2, 0, 255).astype(np.uint8)
    flash[:, :, 1] = np.clip(flash[:, :, 1] * 0.95, 0, 255).astype(np.uint8)
    flash[:, :, 2] = np.clip(flash[:, :, 2] * 0.9, 0, 255).astype(np.uint8)
    alpha = min(1.0, intensity * 0.3)
    cv2.addWeighted(flash, alpha, frame, 1.0, 0, frame)


def draw_sparks(frame, t, count=15):
    """Floating spark particles"""
    for i in range(count):
        seed = i * 137 + 42
        random.seed(seed)
        life_start = random.uniform(0.2, 0.8)
        life_dur = random.uniform(0.5, 1.5)
        life_t = (t - life_start) / life_dur

        if life_t < 0 or life_t > 1:
            continue

        x = int(WIDTH * 0.3 + random.uniform(-200, 600))
        base_y = int(HEIGHT * 0.35 + random.uniform(-50, 80))
        y = int(base_y - life_t * random.uniform(30, 120))
        x += int(math.sin(life_t * 6 + i) * 20)

        alpha = (1.0 - life_t) * 0.8
        size = max(1, int(3 * (1.0 - life_t)))
        brightness = int(255 * alpha)
        color = (brightness, int(brightness * 0.8), int(brightness * 0.4))  # BGR: warm spark

        cv2.circle(frame, (x, y), size, color, -1)


def put_text_centered(frame, text, y, font_scale, color, thickness=2, font=cv2.FONT_HERSHEY_SIMPLEX):
    """Draw centered text"""
    (tw, th), _ = cv2.getTextSize(text, font, font_scale, thickness)
    x = (WIDTH - tw) // 2
    cv2.putText(frame, text, (x, y + th // 2), font, font_scale, color, thickness, cv2.LINE_AA)
    return x, y, tw, th


def draw_glow_text(frame, text, y, font_scale, color, thickness, glow_radius=8, glow_alpha=0.3):
    """Text with glow effect"""
    # Draw glow (blurred larger text)
    glow_layer = np.zeros_like(frame)
    glow_color = (int(color[0] * 0.6), int(color[1] * 0.6), int(color[2] * 0.8))
    put_text_centered(glow_layer, text, y, font_scale * 1.02, glow_color, thickness + 2)
    glow_layer = cv2.GaussianBlur(glow_layer, (glow_radius * 2 + 1, glow_radius * 2 + 1), glow_radius)
    cv2.addWeighted(glow_layer, glow_alpha, frame, 1.0, 0, frame)

    # Draw main text
    put_text_centered(frame, text, y, font_scale, color, thickness)


def draw_accent_line(frame, progress):
    """Animated accent line drawing from center outward"""
    if progress <= 0:
        return
    cx = WIDTH // 2
    y = HEIGHT // 2 + 60
    max_half_len = 200
    half_len = int(max_half_len * min(1.0, progress))

    # Glow
    for i in range(1, 4):
        alpha = 0.15 / i
        glow = np.zeros_like(frame)
        cv2.line(glow, (cx - half_len, y), (cx + half_len, y), (255, 140, 30), 2 + i * 2)
        cv2.addWeighted(glow, alpha, frame, 1.0, 0, frame)

    # Main line
    cv2.line(frame, (cx - half_len, y), (cx + half_len, y), (255, 140, 30), 2)


def generate_frame(frame_num):
    """Generate a single frame"""
    t = frame_num / TOTAL_FRAMES  # 0.0 to 1.0

    frame = np.zeros((HEIGHT, WIDTH, 3), dtype=np.uint8)

    # --- Background ---
    draw_gradient_bg(frame)
    draw_grid(frame, opacity=0.025)
    draw_vignette(frame, strength=0.55)

    # --- Lightning flashes ---
    lightning = 0.0
    # Flash at t=0.3 and t=0.5
    for flash_time in [0.25, 0.45]:
        dt = abs(t - flash_time)
        if dt < 0.03:
            lightning = max(lightning, 1.0 - dt / 0.03)

    draw_lightning(frame, lightning)

    # --- Title: "STORMBREAKER" ---
    title_appear = ease_out(max(0, (t - 0.15) / 0.3))  # appears 0.15-0.45
    if title_appear > 0:
        # Metallic silver-blue color
        brightness = int(220 * title_appear)
        title_color = (int(brightness * 1.0), int(brightness * 0.9), int(brightness * 0.85))  # BGR

        # Glow pulse
        pulse = 0.7 + 0.3 * math.sin(t * 8)
        glow_alpha = 0.4 * title_appear * pulse

        title_y = HEIGHT // 2 - 40
        draw_glow_text(frame, "STORMBREAKER", title_y, 3.0, title_color, 4,
                       glow_radius=15, glow_alpha=glow_alpha)

    # --- Tagline: "BATTLE ROYALE" ---
    tag_appear = ease_out(max(0, (t - 0.45) / 0.25))  # appears 0.45-0.70
    if tag_appear > 0:
        tag_brightness = int(160 * tag_appear)
        tag_color = (int(tag_brightness * 1.1), int(tag_brightness * 0.95), int(tag_brightness * 0.85))
        tag_y = HEIGHT // 2 + 20
        put_text_centered(frame, "BATTLE ROYALE", tag_y, 1.2, tag_color, 2)

    # --- Accent line ---
    line_progress = max(0, (t - 0.50) / 0.2)  # draws 0.50-0.70
    draw_accent_line(frame, line_progress)

    # --- Sparks ---
    if t > 0.2:
        draw_sparks(frame, t, count=20)

    # --- Bottom text ---
    if t > 0.6:
        bottom_alpha = min(1.0, (t - 0.6) / 0.2)
        ver_brightness = int(80 * bottom_alpha)
        ver_color = (int(ver_brightness * 1.2), int(ver_brightness * 0.9), int(ver_brightness * 0.7))

        cv2.putText(frame, "Unreal Engine 5.8", (WIDTH - 280, HEIGHT - 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, ver_color, 1, cv2.LINE_AA)

        cv2.putText(frame, "StormBreaker Games", (20, HEIGHT - 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, ver_color, 1, cv2.LINE_AA)

    # --- Fade from black at start ---
    if t < 0.1:
        fade = t / 0.1
        frame = (frame * fade).astype(np.uint8)

    # --- Fade to black at end ---
    if t > 0.85:
        fade = 1.0 - (t - 0.85) / 0.15
        frame = (frame * max(0, fade)).astype(np.uint8)

    return frame


def main():
    print("=" * 50)
    print("StormBreaker Animated Splash Generator")
    print(f"Resolution: {WIDTH}x{HEIGHT} @ {FPS}fps")
    print(f"Duration: {DURATION}s ({TOTAL_FRAMES} frames)")
    print("=" * 50)

    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    writer = cv2.VideoWriter(OUTPUT_PATH, fourcc, FPS, (WIDTH, HEIGHT))

    for i in range(TOTAL_FRAMES):
        frame = generate_frame(i)
        writer.write(frame)

        if (i + 1) % 15 == 0 or i == TOTAL_FRAMES - 1:
            pct = int((i + 1) / TOTAL_FRAMES * 100)
            print(f"  Frame {i + 1}/{TOTAL_FRAMES} ({pct}%)")

    writer.release()

    file_size = os.path.getsize(OUTPUT_PATH) / (1024 * 1024)
    print(f"\nOutput: {OUTPUT_PATH}")
    print(f"Size: {file_size:.1f} MB")
    print("Done!")


if __name__ == "__main__":
    main()
