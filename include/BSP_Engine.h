#ifndef BSP_ENGINE_H
#define BSP_ENGINE_H

#include "BSP_Settings.h"
#include "Display_Driver.h"


/* ============================================================================
 *  BSP_Engine.h
 *
 *  Map generation:
 *      Recursive geometric splits (https://eskerda.com/bsp-dungeon-generation/)
 *      - originally JavaScript, ported to C and pruned for AVR.
 *
 *  Rendering:
 *      Inspired by Doom (1993):
 *          - true front-to-back BSP traversal
 *          - per-column xBuffer occlusion
 *          - perspective-correct (1/depth) wall column projection
 *      Plus a small camera-space clipper that removes the "wall behind the
 *      player still draws as a stripe" artifact.
 *
 *  All tunable knobs live in BSP_Settings.h.
 * ============================================================================ */


/* ----- Geometry types --------------------------------------------------- */

// 2D integer point. Used for wall edge endpoints during projection.
struct xy
{
    int x;
    int y;
};


// One BSP leaf sector. Position + size, color, and two flag bits.
//
// Kept small for AVR RAM (Mega 2560 has 8 KB SRAM). Derived values like
// edge corners and the center point are computed on demand by the inline
// helpers below.
struct Container
{
    int16_t  x, y, w, h;     // top-left + size; map_Size <= 32k -> int16 fits
    uint16_t order;          // 1..rooms, leaf number assigned at build time
    uint16_t color;          // RGB565

    uint8_t  is_wall : 1;    // packed into a single byte alongside any future flags
    uint8_t  _pad    : 7;

    // C++11 ctor (bitfield default member initializers are C++20).
    Container()
        : x(0), y(0), w(0), h(0),
          order(0), color(BLACK),
          is_wall(0), _pad(0) {}
};


// Inline helpers for derived geometry. Compiler inlines these completely.
inline int16_t cx(const Container& c)       { return c.x + c.w / 2; }
inline int16_t cy(const Container& c)       { return c.y + c.h / 2; }
inline xy edge_lu(const Container& c)       { xy p; p.x = c.x;       p.y = c.y;       return p; }
inline xy edge_ru(const Container& c)       { xy p; p.x = c.x + c.w; p.y = c.y;       return p; }
inline xy edge_ld(const Container& c)       { xy p; p.x = c.x;       p.y = c.y + c.h; return p; }
inline xy edge_rd(const Container& c)       { xy p; p.x = c.x + c.w; p.y = c.y + c.h; return p; }


// Pair of containers - return value of geo_split.
struct Array2
{
    Container array[2];
};


// BSP tree node. Inner nodes have lchild + rchild; leaves have both NULL.
struct Tree
{
    Container leaf;
    Tree*     lchild;
    Tree*     rchild;
};


/* ----- View (camera + scene) -------------------------------------------- */

// Holds the player's camera state plus pointers into the map data.
//   leaves       - flat array of all leaf sectors. Wall flags are kept here.
//   Container_map - root of the BSP tree. Used for getPosition() lookups
//                   and for the front-to-back render traversal.
//   The two structures share data (printLeafNodes copies the leaves) so any
//   wall flag must be set on `leaves`, not on the tree.
struct View
{
    int   rooms       = pow(2, iteration);
    float playerAngle = SPAWN_ANGLE;
    float FOV         = (5.0f * PI / 9.0f);   // 100 degrees

    Tree* test;            // (legacy name) flat leaf array - wall data lives here
    Tree* Container_map;   // (legacy name) root of the BSP tree
};


/* ----- Map building ----------------------------------------------------- */

Tree*   newLeaf(Container leaf);
Array2  geo_split(Container* container);
Tree*   split_container(Container container, int iter);
Tree*   printLeafNodes(Tree* root, Tree* order, int* step);
Tree*   getPosition(int value_x, int value_y, Tree* node);

float   VectorDegree(int& x_wall, int& y_wall, int& x2_wall, int& y2_wall);

View    Save_BSP_Engine();


/* ----- Drawing pipeline ------------------------------------------------- */
//
// FillSegment   : writes one column strip given screen X positions and
//                 perpendicular depths at the two endpoints.
// DrawSegment   : projects a wall edge (two world endpoints) into screen
//                 space + near-plane clipping; then calls FillSegment.
// DrawWall      : picks the visible faces of a wall sector and draws them.
// RenderBSP     : true front-to-back BSP traversal with xBuffer occlusion.

void FillSegment(float dist_1, float dist_2,
                 int x_pos_1, int x_pos_2,
                 uint16_t color,
                 Adafruit_SSD1351 tft, BSP_Player P, View V);

void DrawSegment(xy edge1, xy edge2,
                 uint16_t color,
                 Adafruit_SSD1351 tft, BSP_Player P, View V);

void DrawWall(Tree* wall,
              Adafruit_SSD1351 tft, BSP_Player P, View V);

void RenderBSP(Tree* node,
               Adafruit_SSD1351 tft, BSP_Player P, View V);


/* ----- View helpers ----------------------------------------------------- */

// Wraps V.playerAngle into [0, 2*PI). Takes V by reference so the
// normalisation actually persists on the caller's struct.
void FieldOfView2Angle(View& V);


/* ----- Background (sky / floor) ----------------------------------------- */
//
// Both helpers pick BLACK, SKY_COLOR or FLOOR_COLOR depending on
// ENABLE_FLOOR_AND_SKY at compile time. They are used:
//   - FillSegment, when a wall got smaller (delta redraw)
//   - Game_loop column cleanup, when a column lost its wall
//   - Game_loop initial frame, to lay down the horizon

// Paints a vertical column strip [y, y+h) with the appropriate background.
// Splits across screenHeight/2 so sky stays on top and floor on the bottom.
void paint_background_strip(Adafruit_SSD1351 tft, int x, int y, int h);

// Paints the entire screen with sky + floor (or solid black).
void paint_full_background(Adafruit_SSD1351 tft);


#endif
