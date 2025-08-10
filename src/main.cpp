#include "raylib.h"
#include <array>
#include <vector>
#include <random>
#include <algorithm>

constexpr int COLS = 10;
constexpr int ROWS = 20;
constexpr int TILE = 28;

constexpr int BOARD_W = COLS * TILE;
constexpr int BOARD_H = ROWS * TILE;

constexpr int SIDE_W  = 220;
constexpr int PAD = 18;

struct Cell { int id = 0; };           // 0 empty, 1..7 tetrominoes
using Grid = std::array<std::array<Cell, COLS>, ROWS>;

struct Piece {
    int type = 0;                       // 0..6 for I,J,L,O,S,T,Z
    int r = 0;                          // rotation index
    int x = 3, y = -2;                  // spawn
};

static const std::array<Color,8> COLORS = {
    Color{0,0,0,0},
    Color{ 89,203,232,255}, // I
    Color{ 91,110,225,255}, // J
    Color{242,166, 90,255}, // L
    Color{245,220, 92,255}, // O
    Color{ 87,211,140,255}, // S
    Color{199,114,230,255}, // T
    Color{239,106,106,255}  // Z
};

// rotations for 7 pieces (each rotation = 4 coords)
using Shape = std::array<Vector2,4>;
static const std::array<std::vector<Shape>,7> SHAPES = {{
    // I
    { Shape{ Vector2{0,1}, {1,1}, {2,1}, {3,1} },
      Shape{ Vector2{2,0}, {2,1}, {2,2}, {2,3} } },
    // J
    { Shape{ Vector2{0,0}, {0,1}, {1,1}, {2,1} },
      Shape{ Vector2{1,0}, {2,0}, {1,1}, {1,2} },
      Shape{ Vector2{0,1}, {1,1}, {2,1}, {2,2} },
      Shape{ Vector2{1,0}, {1,1}, {1,2}, {0,2} } },
    // L
    { Shape{ Vector2{2,0}, {0,1}, {1,1}, {2,1} },
      Shape{ Vector2{1,0}, {1,1}, {1,2}, {2,2} },
      Shape{ Vector2{0,1}, {1,1}, {2,1}, {0,2} },
      Shape{ Vector2{0,0}, {1,0}, {1,1}, {1,2} } },
    // O
    { Shape{ Vector2{1,0}, {2,0}, {1,1}, {2,1} } },
    // S
    { Shape{ Vector2{1,0}, {2,0}, {0,1}, {1,1} },
      Shape{ Vector2{1,0}, {1,1}, {2,1}, {2,2} } },
    // T
    { Shape{ Vector2{1,0}, {0,1}, {1,1}, {2,1} },
      Shape{ Vector2{1,0}, {1,1}, {2,1}, {1,2} },
      Shape{ Vector2{0,1}, {1,1}, {2,1}, {1,2} },
      Shape{ Vector2{1,0}, {0,1}, {1,1}, {1,2} } },
    // Z
    { Shape{ Vector2{0,0}, {1,0}, {1,1}, {2,1} },
      Shape{ Vector2{2,0}, {1,1}, {2,1}, {1,2} } }
}};

std::mt19937 rng(std::random_device{}());

struct SevenBag {
    std::vector<int> bag;
    int next() {
        if (bag.empty()) {
            bag = {0,1,2,3,4,5,6};
            std::shuffle(bag.begin(), bag.end(), rng);
        }
        int t = bag.back(); bag.pop_back(); return t;
    }
};

bool valid(const Grid& g, const Piece& p) {
    const auto& rots = SHAPES[p.type];
    const auto& shp = rots[p.r % rots.size()];
    for (auto c : shp) {
        int x = p.x + (int)c.x;
        int y = p.y + (int)c.y;
        if (x < 0 || x >= COLS || y >= ROWS) return false;
        if (y >= 0 && g[y][x].id != 0) return false;
    }
    return true;
}

void lockPiece(Grid& g, const Piece& p) {
    const auto& rots = SHAPES[p.type];
    const auto& shp = rots[p.r % rots.size()];
    for (auto c : shp) {
        int x = p.x + (int)c.x;
        int y = p.y + (int)c.y;
        if (y >= 0) g[y][x].id = p.type + 1; // 1..7
    }
}

int clearLines(Grid& g) {
    int cleared = 0;
    for (int y = ROWS - 1; y >= 0; --y) {
        bool full = true;
        for (int x = 0; x < COLS; ++x) if (g[y][x].id == 0) { full = false; break; }
        if (full) {
            for (int yy = y; yy > 0; --yy) g[yy] = g[yy - 1];
            for (int x = 0; x < COLS; ++x) g[0][x].id = 0;
            cleared++; y++;
        }
    }
    return cleared;
}

void drawCell(int bx, int by, Color fill, bool bevel=true) {
    Rectangle r{ (float)(bx*TILE+1), (float)(by*TILE+1), (float)(TILE-2), (float)(TILE-2) };
    DrawRectangleRounded(r, 0.18f, 6, fill);
    if (bevel) {
        DrawRectangleGradientV((int)r.x, (int)r.y, (int)r.width, 6, Fade(WHITE,0.13f), BLANK);
        DrawRectangle((int)r.x, (int)(r.y + r.height - 6), (int)r.width, 6, Fade(BLACK,0.18f));
    }
}

void drawPanel(Rectangle rect, const char* title) {
    DrawRectangleRounded(rect, 0.2f, 8, Color{20,24,48,255});
    DrawRectangleRoundedLines(rect, 0.2f, 8, Color{48,56,112,255});
    DrawText(title, (int)rect.x + 12, (int)rect.y + 10, 18, Color{154,163,178,255});
}

void drawMiniPiece(int ox, int oy, int cell, int type) {
    const auto& shp = SHAPES[type][0];
    // center it
    int minx=100, maxx=-100, miny=100, maxy=-100;
    for (auto c : shp) { minx = std::min(minx,(int)c.x); maxx = std::max(maxx,(int)c.x);
                         miny = std::min(miny,(int)c.y); maxy = std::max(maxy,(int)c.y); }
    int w = (maxx - minx + 1), h = (maxy - miny + 1);
    int offx = ox + (120 - w*cell)/2;
    int offy = oy + (120 - h*cell)/2;
    for (auto c : shp) {
        int px = offx + ((int)c.x - minx)*cell;
        int py = offy + ((int)c.y - miny)*cell;
        DrawRectangleRounded({(float)px,(float)py,(float)cell-2,(float)cell-2},0.25f,6,COLORS[type+1]);
    }
}

inline bool pieceAboveTop(const Piece& p) {
    const auto& shp = SHAPES[p.type][p.r % SHAPES[p.type].size()];
    for (auto c : shp) {
        if (p.y + (int)c.y < 0) return true;  // any block above the visible field
    }
    return false;
}


int main() {
    const int winW = BOARD_W + SIDE_W + PAD*3;
    const int winH = BOARD_H + PAD*2;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(winW, winH, "Tetris — Modern C++/raylib");
    SetTargetFPS(60);
    InitAudioDevice();
    SetMasterVolume(1.0f); // global volume

    Music music = LoadMusicStream("assets/theme.mp3");
    music.looping = true;
    SetMusicVolume(music, 0.6f);
    PlayMusicStream(music);

    Sound sfxDrop  = LoadSound("assets/drop.wav");
    Sound sfxClear = LoadSound("assets/clear.wav");
    SetSoundVolume(sfxDrop,  0.9f);
    SetSoundVolume(sfxClear, 0.9f);

    // mute toggle
    bool muted = false;

    Grid grid{}; for (auto& r : grid) for (auto& c : r) c.id = 0;

    SevenBag bag;
    Piece cur{ bag.next(), 0, 3, -2 };
    Piece nxt{ bag.next(), 0, 3, -2 };
    int holdType = -1;
    bool canHold = true;

    int score = 0, lines = 0, level = 1;
    float gravityMs = 1000.f;           // speeds up per level
    float fallTimer = 0.f;

    const float lockDelayMs = 350.f;    // small grace before lock
    float lockTimer = 0.f;
    bool grounded = false;

    bool paused = false;
    bool gameOver = false;

    auto reset = [&](){
        for (auto& r : grid) for (auto& c : r) c.id = 0;
        bag = SevenBag{};
        cur = Piece{ bag.next(), 0, 3, -2 };
        nxt = Piece{ bag.next(), 0, 3, -2 };
        holdType = -1; canHold = true;
        score = lines = 0; level = 1;
        gravityMs = 1000.f; fallTimer = 0.f;
        lockTimer = 0.f; grounded = false;
        paused = false; gameOver = false;
    };

    auto softMove = [&](int dx, int dy) {
        Piece t = cur; t.x += dx; t.y += dy;
        if (valid(grid, t)) { cur = t; return true; }
        return false;
    };

    auto rotate = [&](int dir){
        Piece t = cur;
        const auto& rots = SHAPES[t.type];
        t.r = (t.r + (dir>0 ? 1 : (int)rots.size()-1)) % (int)rots.size();
        // simple wall kicks
        const int kicks[5] = {0,-1,1,-2,2};
        for (int k: kicks) { Piece w=t; w.x += k; if (valid(grid,w)) { cur = w; return; } }
    };

    auto hardDrop = [&](){
        int dropped = 0;
        while (softMove(0,1)) { dropped++; }

        bool overflow = pieceAboveTop(cur);
        lockPiece(grid, cur);
        PlaySound(sfxDrop);
        if (overflow) {
            gameOver = true; paused = true;
            return;
        }

        score += dropped * 2;
        int c = clearLines(grid);
        if (c) {
            PlaySound(sfxClear);
            static const int table[5] = {0,40,100,300,1200};
            score += table[c] * level;
            lines += c;
            level = 1 + lines/10;
            gravityMs = std::max(120.f, 1000.f - 80.f*(level-1));
        }
        cur = nxt; nxt = Piece{ bag.next(), 0, 3, -2 }; canHold = true;
        if (!valid(grid, cur)) { gameOver = true; paused = true; }
        grounded = false; lockTimer = 0.f;
    };

    auto doHold = [&](){
        if (!canHold) return;
        if (holdType == -1) {
            holdType = cur.type;
            cur = nxt; nxt = Piece{ bag.next(), 0, 3, -2 };
        } else {
            std::swap(holdType, cur.type);
            cur.r = 0; cur.x = 3; cur.y = -2;
        }
        canHold = false;
        if (!valid(grid, cur)) { gameOver = true; paused = true; }
    };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime() * 1000.0f;

        // INPUT
        if (IsKeyPressed(KEY_P)) paused = !paused;
        if (IsKeyPressed(KEY_R)) reset();
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
            if (!IsWindowFullscreen()) {
                SetWindowSize(winW, winH);
            }
        }
        if (IsKeyPressed(KEY_M)) {           // optional mute key
            muted = !muted;
            SetMasterVolume(muted ? 0.0f : 1.0f);
        }

        UpdateMusicStream(music);

        if (!paused && !gameOver) {
            if (IsKeyPressed(KEY_LEFT))  softMove(-1,0);
            if (IsKeyPressed(KEY_RIGHT)) softMove(1,0);
            // DAS/ARR-lite
            static float holdL=0, holdR=0;
            if (IsKeyDown(KEY_LEFT))  { holdL += dt; if (holdL>170) { if (softMove(-1,0)) holdL -= 40; } }
            else holdL = 0;
            if (IsKeyDown(KEY_RIGHT)) { holdR += dt; if (holdR>170) { if (softMove(1,0))  holdR -= 40; } }
            else holdR = 0;

            if (IsKeyPressed(KEY_UP)) rotate(+1);
            if (IsKeyPressed(KEY_Z))  rotate(-1);
            if (IsKeyPressed(KEY_SPACE)) hardDrop();
            if (IsKeyDown(KEY_DOWN))  { if (softMove(0,1)) score += 1; }
            if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) doHold();

            // gravity + lock delay
            fallTimer += dt;
            bool couldFall = false;
            if (fallTimer >= gravityMs) {
                fallTimer = 0.f;
                couldFall = softMove(0,1);
            }
            // grounded?
            Piece t = cur; t.y += 1;
            bool onFloor = !valid(grid, t);
            if (onFloor) {
                if (!grounded) { grounded = true; lockTimer = 0.f; }
                else           { lockTimer += dt; }
                if (!couldFall && lockTimer >= lockDelayMs) {
                    bool overflow = pieceAboveTop(cur);   // <--- NEW
                    lockPiece(grid, cur);
                    if (overflow) {                       // <--- NEW
                        gameOver = true; paused = true;
                    } else {
                        int c = clearLines(grid);
                        if (c) {
                            PlaySound(sfxClear);
                            static const int table[5] = {0,40,100,300,1200};
                            score += table[c] * level;
                            lines += c;
                            level = 1 + lines/10;
                            gravityMs = std::max(120.f, 1000.f - 80.f*(level-1));
                        }
                        cur = nxt; nxt = Piece{ bag.next(), 0, 3, -2 }; canHold = true;
                        if (!valid(grid, cur)) { gameOver = true; paused = true; }
                    }
                    grounded = false; lockTimer = 0.f;
                }
            } else {
                grounded = false; lockTimer = 0.f;
            }
        }

        // RENDER
        BeginDrawing();
        ClearBackground(Color{15,18,32,255});

        // Board panel
        Rectangle boardRect{ (float)PAD, (float)PAD, (float)BOARD_W, (float)BOARD_H };
        DrawRectangleRounded(boardRect, 0.02f, 4, Color{21,25,53,255});
        // grid bg
        for (int y=0;y<ROWS;y++){
            for (int x=0;x<COLS;x++){
                Color base = ( (x+y)&1 ) ? Color{25,28,56,255} : Color{22,26,52,255};
                drawCell(x, y, base, false);
            }
        }
        // locked cells
        for (int y=0;y<ROWS;y++){
            for (int x=0;x<COLS;x++){
                if (grid[y][x].id) drawCell(x,y, COLORS[grid[y][x].id]);
            }
        }
        // ghost
        if (!gameOver) {
            Piece g = cur;
            while (valid(grid, Piece{g.type,g.r,g.x,g.y+1})) g.y++;
            const auto& shp = SHAPES[g.type][g.r % SHAPES[g.type].size()];
            for (auto c : shp) {
                int gx = g.x + (int)c.x, gy = g.y + (int)c.y;
                if (gy>=0) drawCell(gx, gy, Color{255,255,255,25}, false);
            }
            // current piece
            const auto& cp = SHAPES[cur.type][cur.r % SHAPES[cur.type].size()];
            for (auto c : cp) {
                int cx = cur.x + (int)c.x, cy = cur.y + (int)c.y;
                if (cy>=0) drawCell(cx, cy, COLORS[cur.type+1]);
            }
        }

        // Side UI
        int sx = PAD + BOARD_W + PAD;
        int top = PAD;

        // Score/Level/Lines
        Rectangle card1{ (float)sx, (float)top, (float)SIDE_W, 120 };
        drawPanel(card1, "Stats");
        DrawText(TextFormat("Score"), sx+14, top+40, 16, Color{154,163,178,255});
        DrawText(TextFormat("%d", score), sx+14, top+64, 28, RAYWHITE);
        DrawText(TextFormat("Level  %d", level), sx+120, top+40, 16, Color{154,163,178,255});
        DrawText(TextFormat("Lines  %d", lines), sx+120, top+64, 16, RAYWHITE);

        // Next
        top += 140;
        Rectangle card2{ (float)sx, (float)top, (float)SIDE_W, 150 };
        drawPanel(card2, "Next");
        DrawRectangleRounded({(float)(sx+14),(float)(top+34),192,102},0.16f,6,Color{17,22,47,255});
        drawMiniPiece(sx+20, top+40, 26, nxt.type);

        // Hold
        top += 170;
        Rectangle card3{ (float)sx, (float)top, (float)SIDE_W, 150 };
        drawPanel(card3, "Hold");
        DrawRectangleRounded({(float)(sx+14),(float)(top+34),192,102},0.16f,6,Color{17,22,47,255});
        if (holdType >= 0) drawMiniPiece(sx+20, top+40, 26, holdType);
        DrawText("Shift to hold", sx+14, top+118, 14, Color{154,163,178,255});

        // Controls
        top += 170;
        Rectangle card4{ (float)sx, (float)top, (float)SIDE_W, 120 };
        drawPanel(card4, "Controls");
        DrawText("L/R: move  Down: soft", sx+14, top+40, 16, Color{200,206,220,255});
        DrawText("Z: rotate  Space: hard", sx+14, top+64, 16, Color{200,206,220,255});
        DrawText("P pause     R restart", sx+14, top+88, 16, Color{200,206,220,255});
        DrawText("M Muted", sx+14, top+102, 16, Color{200,206,220,255});

        if (paused) {
            DrawRectangle(0,0,winW,winH, Color{0,0,0,120});
            const char* t = gameOver? "GAME OVER" : "PAUSED";
            int tw = MeasureText(t, 48);
            DrawText(t, winW/2 - tw/2, winH/2 - 48, 48, RAYWHITE);
            DrawText("Press R to restart", winW/2 - 120, winH/2 + 6, 20, Color{220,220,230,255});
        }

        EndDrawing();
    }

    UnloadSound(sfxDrop);
    UnloadSound(sfxClear);
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
