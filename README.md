# Ghost-Engine-Classic-2D

A pseudo-3D rendering engine for a 128×128 RGB OLED on an Arduino Mega 2560,
combining two early-90s techniques:

- **Raycasting** (per-column distance scan, *Wolfenstein 3D*, 1992)
- **Binary Space Partitioning** (front-to-back tree traversal, *Doom*, 1993)

The whole pipeline runs on an ATmega2560 with 8 KB of SRAM.

> Originally my Bachelor thesis on an Arduino Uno. The code now targets a
> Mega 2560 for headroom, but the algorithms are unchanged.

[Engine + game on OLED](https://www.youtube.com/watch?v=XWTsxJl02wU) ·
[Engine on LED matrix](https://www.youtube.com/watch?v=WRVSrZ5XRVQ)


## Hardware & build

| Part           | Wiring                                       |
| -------------- | -------------------------------------------- |
| MCU            | Arduino Mega 2560 (16 MHz, 8 KB SRAM)        |
| Display        | 1.5" SSD1351 RGB OLED, 128×128, hardware SPI |
| SCLK / MOSI    | 52 / 51                                      |
| DC / CS / RST  | 9 / 10 / 8                                   |
| Buttons        | F=2, B=3, L=4, R=5, A1=6, A2=7               |

```bash
pio run                  # compile
pio run -t upload        # flash
pio device monitor       # serial @ 115200
```

Built with C++17 on AVR-GCC. Adafruit GFX, the SSD1351 driver and Bounce2
are pulled in via `lib_deps` in [`platformio.ini`](platformio.ini) — no
vendored library copies in the repo.


## Controls

Hardware buttons and Serial Monitor (`115200` baud) work in parallel.
Hold a hardware button for continuous motion; each Serial keystroke is a
single discrete step.

| Action       | Button (pin) | Key |
| ------------ | ------------ | --- |
| Forward      | F (D2)       | `w` |
| Backward     | B (D3)       | `s` |
| Rotate left  | L (D4)       | `a` |
| Rotate right | R (D5)       | `d` |
| Action 1     | A1 (D6)      | `e` |
| Action 2     | A2 (D7)      | `q` |
| Help         | —            | `h` |

All gameplay tuning (movement rates, FOV, wall density, sky/floor colors,
spawn point, …) lives in [`include/BSP_Settings.h`](include/BSP_Settings.h).


---

# How the rendering works

```
   Map gen (once)         Per frame
   --------------         -------------------------------------------
   BSP subdivide    -->   FOV update -> RenderBSP (front-to-back)
                                              |
                                              v
                                          DrawWall -> DrawSegment
                                                          |
                                                          v
                                                     FillSegment
                                                          |
                                                          v
                                              per-column SPI write
```


## 1. BSP map generation

The world is one square. We split it `iteration` times, alternating
vertical and horizontal cuts with a ratio constraint that prevents skinny
sectors. Leaves of the resulting binary tree are the playable rooms.

```
   iter 0          iter 1            iter 2

  +--------+      +----+----+       +---+--+----+
  |        |      |    |    |       |   |  |    |
  |        |  ->  |    |    |  ->   +---+--+----+
  |        |      |    |    |       |     |    |
  |        |      |    |    |       |     +-+--+
  +--------+      +----+----+       +-----+-+--+
```

`iteration = 6` gives **64 leaves**. Each leaf is a 13-byte `Container`
with `constexpr` member functions for the derived geometry (`cx()`,
`cy()`, `edge_lu()` …), so edges and the center are recomputed on demand
without any storage cost. The tree node is 17 bytes total. Sectors are
randomly flagged as walls afterwards; the spawn sector is forced empty
so every wall sits in front of the player.


## 2. Front-to-back BSP traversal

This is the *Doom* trick. We walk the tree once, picking the player's
side of the split first at every inner node. Combined with the
per-column occlusion buffer in step 4, we never z-sort and never
overdraw.

```
                BSP tree                       Map (top-down)

                   *                           +-----+-----+
                  / \                          |     |     |
                 /   \                         |  L  |  R  |
                L     R                        |     |     |
               / \   / \                       +--*--+-----+
              a   b c   d                         player
                                       
   Player is on the left side of the root split, so:
       RenderBSP(L)  -- drawn first into xBuffer
       RenderBSP(R)  -- drawn second, but already occluded
   Recursing into L picks the nearer of {a, b} first by the same rule.
```

The split axis at each inner node is recovered from the children's
geometry: same X column → horizontal split, same Y row → vertical split.
No per-node split-plane needs to be stored (precious bytes saved).


## 3. Wall projection + FOV clipping — `DrawSegment`

For each visible wall face we have two world endpoints. They go through
a **camera transform** and a **3-plane clip**:

```
                       ^ forward
                       |
        \              |              /
         \   outside   |   outside   /
   left FOV  \          |          /  right FOV
    plane     \  visible cone  /     plane
               \        |        /
                \       |       /
                 \______|______/
                        |
                     PLAYER
```

```
   wall in world :   A *-------------* B
                                              ^ forward
   in camera space:                            |
                              outside left  /  |  inside
                                          /    |
                              A *--------/-----* B
                                       A'      
   
   A is outside the left FOV plane.
   Linearly interpolate A onto the plane -> A'.
   A' has the correct depth at the FOV boundary,
   so the wall does not "stretch into the middle".
```

The clipping uses three half-spaces (Sutherland-Hodgman, one segment
endpoint pair, one plane at a time):

| Plane       | Inside half-space                       |
| ----------- | --------------------------------------- |
| Near plane  | `depth >= NEAR_PLANE_DEPTH`             |
| Right FOV   | `lateral <=  depth · tan(FOV/2)`        |
| Left  FOV   | `lateral >= -depth · tan(FOV/2)`        |

After the clip, both endpoints sit inside the visible cone, so the
projection is a clean perspective divide:

```
   screen_x = w/2 + (lateral / depth) / tan(FOV/2) · (w/2)
```

Why both clips and not just the near plane: with a near-plane clip
alone, a wall edge that runs past the side of the FOV gets yanked
forward to `depth = 1`. Its line height blows up to full screen at the
edge, regardless of how far the wall actually is — the "wall in your
face" artifact when walking parallel to a wall.


## 4. Per-column drawing — `FillSegment` + buffers

We maintain three flat 128-element arrays, one entry per screen column:

| Buffer           | Type       | Meaning                              |
| ---------------- | ---------- | ------------------------------------ |
| `xBuffer[x]`     | `bool`     | column already drawn this frame?     |
| `yBuffer[x]`     | `int`      | line height drawn last frame         |
| `colorBuffer[x]` | `uint16_t` | RGB565 color drawn last frame        |

`xBuffer` is the **occlusion list** that powers front-to-back rendering:

```
   one screen column over time:

        wall A (closer)          wall B (farther)
        ---------------          ----------------
        FillSegment runs         FillSegment runs
        xBuffer[x] is false  ->  xBuffer[x] is true
        paints column            skip column
        sets xBuffer[x] = true   (already painted by A)
```

`yBuffer` and `colorBuffer` enable a **delta redraw**. SPI is the
bottleneck on AVR, so columns whose height *and* color match last frame
send zero pixels. Only the differing strips at the top and bottom of a
shrinking wall, or a single full-column repaint on a color change, hit
the bus.

Line height is perspective-correct:

```
   line_height = WALL_PROJ_SCALE / depth
```

Across a wall, `1/depth` (NOT `depth` itself) is interpolated linearly
between the two endpoints — the same trick the PlayStation 1 famously
skipped, hence its wobbly textures.

Above and below the wall column we paint a sky strip and a floor strip
when `ENABLE_FLOOR_AND_SKY = 1`. The horizon gives the eye a stable
up/down reference during rotation. Set the flag to `0` for the original
solid-black look.


## 5. HUD overlays

Two cosmetic overlays are drawn on top of the world each frame, after
the column cleanup pass:

| Toggle (in `BSP_Settings.h`) | Effect                                          |
| ---------------------------- | ----------------------------------------------- |
| `ENABLE_FPS_COUNTER`         | Small white digits in the top-left, refreshed twice per second. Always repainted so a wall sliding into the corner cannot half-eat the digits. |
| `ENABLE_MINIMAP`             | 32×32 top-down view of the BSP map in the top-right. Walls in their real color, a white dot for the player, a yellow line for the view direction. |

Each overlay is gated by an `#if` on its toggle, so disabling either one
removes both the data and the call sites at compile time — zero runtime
cost when off.

```
   +------------+------+
   | FPS        | MAP  |
   |            |      |
   +            +------+
   |        wall view  |
   |                   |
   |                   |
   +-------------------+
```


---

# The game

Boot opens a two-entry menu:

- **PLAY** — generates a BSP world with random colored walls and drops
  you into it. No enemies, no goal: walk around. Spawn is in a corner
  with the map boundary at your back.

- **RAYCAST** — opens the Lode-style raycaster path with its own grid
  map editor (place walls in a 24×24 grid, then enter first-person view).


<p align="center">
  <img src="/Media/Final_Game.png" width="400" alt="Final Game">
  <img src="/Media/Game_update_2026.jpeg" width="400" alt="Game Update 2026">
  <img src="/Media/Scene.png" width="400" alt="Scene from the engine">
  <img src="/Media/Visual_Scene.png" width="400" alt="Wall rendering">
  <img src="/Media/Map_Editor.jpeg" width="400" alt="Map editor">
  <img src="/Media/Wolfenstein 3D.jpeg" width="800" alt="Wolfenstein 3D reference">
</p>
