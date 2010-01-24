#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define VIDEO_W 1024
#define VIDEO_H 768
#define VIDEO_BPP 32

#define CELL_W 1
#define CELL_H 1
#define ARR_W (VIDEO_W / CELL_W)
#define ARR_H (VIDEO_H / CELL_H)
#define PIX_CNT 100

#define LOG(...) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
    } while(0)

enum {
    COLOR_WHITE = 0xffffffff,
    COLOR_BLACK = 0x000000ff,
    COLOR_BLUE  = 0x0000ffff,
};

struct cell {
    bool empty;
};

struct world {
    struct cell w[ARR_W][ARR_H];
};

#define WORLD_CNT 2
static struct worlds {
    struct world worlds[WORLD_CNT];
    uint32_t world_id;
    struct world * current_world;
    struct world * next_world;
} universe;
#define UCURRENT (universe.current_world)
#define UNEXT (universe.next_world)

static SDL_Surface * surface = NULL;
static bool run = false;
static bool pause = false;
static inline void world_make_empty(struct world * w, uint32_t x, uint32_t y);
static inline void world_make_fill(struct world * w, uint32_t x, uint32_t y);

void
init_gfx(void) {
    SDL_VideoInfo * video_info = NULL;
    uint32_t video_flags = 0;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG("SDL: init fail\n");
        exit(1);
    }

	video_info = SDL_GetVideoInfo();
    LOG("video mem (kb): %d\n", video_info->video_mem);
    video_flags = SDL_DOUBLEBUF;
    if (video_info->hw_available) {
        LOG("video: HW avail\n");
        video_flags |= SDL_HWACCEL;
    }

    if (video_info->blit_hw) {
        LOG("video: HW surface\n");
        video_flags |= SDL_HWSURFACE;
    } else
        video_flags |= SDL_SWSURFACE;
    
	surface = SDL_SetVideoMode(VIDEO_W, VIDEO_H, VIDEO_BPP, video_flags);
    if (surface != NULL) {
        LOG("video: initialized\n");
    } else {
        LOG("video: init failed\n");
        exit(1);
    }

}

void 
init_random() {
    FILE * f = NULL;
    unsigned int seed = 0;

    f = fopen("/dev/urandom", "r");
    if (f != NULL) {
        LOG("using /dev/urandom\n");
        fread(&seed, sizeof(seed), 1, f);
        fclose(f);
    } else {
        LOG("using time()\n");
        seed = time(NULL);
    }
    LOG("seed: %u\n", seed);
    srandom(seed);
}

void 
init_world() {
    LOG("init world\n");
    universe.current_world = &universe.worlds[0];
    universe.world_id = 0;
    universe.next_world = &universe.worlds[1];

    for(int i = 0; i < ARR_W; i++) {
        for (int j = 0; j < ARR_H; j++) {
#if 0
            if (random() > (RAND_MAX / 2))
                universe.worlds[0].w[i][j].empty = false;
            else
                universe.worlds[0].w[i][j].empty = true;
#else
            universe.worlds[0].w[i][j].empty = true;
#endif
            universe.worlds[1].w[i][j].empty = true;
        }
    }

#if 1
    for (int x = 0; x < 30; x++) {
        int32_t offs_x = (random() % ARR_W) - CELL_W;
        int32_t offs_y = (random() % ARR_H) - CELL_H;
        for (int i = random() % 10; i > 0; i--) {
            for (int j = random() % 10; j > 0; j--) {
                world_make_fill(UCURRENT, offs_x + i, offs_y + j);
                world_make_fill(UCURRENT, offs_x - i, offs_y - j);
                world_make_fill(UCURRENT, offs_x + i, offs_y - j);
            }
        }
    }
#endif
#if 0
    int32_t offs_x = ARR_W / 2;
    int32_t offs_y = ARR_H / 2;
    world_make_fill(UCURRENT, offs_x , offs_y);
    world_make_fill(UCURRENT, offs_x + 1, offs_y);
    world_make_fill(UCURRENT, offs_x, offs_y + 1);
    world_make_fill(UCURRENT, offs_x + 1,  offs_y + 1);
    world_make_fill(UCURRENT, offs_x + 2, offs_y);
#endif

}

static inline void 
world_make_empty(struct world * w, uint32_t x, uint32_t y) {
    if (x >= ARR_W || y >= ARR_H)
        return;
    w->w[x][y].empty = true;
}

static inline void 
world_make_fill(struct world * w, uint32_t x, uint32_t y) {
    if (x >= ARR_W || y >= ARR_H)
        return;
    w->w[x][y].empty = false;
}

bool 
world_will_survive(struct world * w, uint32_t x, uint32_t y) {
    uint32_t neighbors_cnt = 0;

#if 0
    LOG("[%d, %d] <<\n", x, y);
#endif
    for (int i = -1; i < 2; i += 1)
        for (int j = -1; j < 2; j += 1) {
            int32_t xi = (int32_t )x + i;
            int32_t yj = (int32_t )y + j;
            if (i == 0 && j == 0)
                continue;
            if (xi >= ARR_W)
                xi = ARR_W - 1;
            else if (xi < 0)
                xi = 0;

            if (yj >= ARR_H)
                yj = ARR_H - 1;
            else if (yj < 0)
                yj = 0;
            if (w->w[xi][yj].empty == false) {
                neighbors_cnt++;
#if 0
                LOG("[%d, %d] en: %d xi: %d yj: %d n: %d\n", x, y, w->w[xi][yj].empty, xi, yj, neighbors_cnt);
#endif
            }
        }
#if 0
    LOG("[%d, %d] >>\n", x, y);
    LOG("[%u, %u] e: %d n: %d\n", x, y, UCURRENT->w[x][y].empty, neighbors_cnt);
#endif
    if (w->w[x][y].empty == false) {
        if (neighbors_cnt >= 1 && neighbors_cnt <= 5)
            return true;
    } else {
        if (neighbors_cnt >= 3 && neighbors_cnt <= 3)
            return true;
    }
    return false;
}

void
next_generation() 
{
    for(int i = 1; i < ARR_W - 1; i++) {
        for (int j = 1; j < ARR_H - 1; j++) {
#if 1
            if (world_will_survive(UCURRENT, i, j)) {
#if 0
                LOG("[%u, %u] e: %d\n", i, j, UCURRENT->w[i][j].empty);
#endif
                world_make_fill(UNEXT, i, j);
            } else
                world_make_empty(UNEXT, i, j);
#endif
#if 0
            if (UCURRENT->w[i][j].empty == false)
                world_make_fill(UNEXT, i, j);
            else
                world_make_empty(UNEXT, i, j);
#endif
        }
    }
            
    universe.world_id = (universe.world_id + 1) % WORLD_CNT;
    UCURRENT = &universe.worlds[universe.world_id];
    UNEXT = &universe.worlds[(universe.world_id + 1) % WORLD_CNT];
}

void
draw()
{
    /*boxColor(surface, 0, 0, VIDEO_W, VIDEO_H, COLOR_BLACK);*/
    /*uint32_t cnt = random() % PIX_CNT;*/
    for(int i = 0; i < ARR_W; i++) {
        for (int j = 0; j < ARR_H; j++) {
            uint32_t color = COLOR_BLACK;
            if (UCURRENT->w[i][j].empty == false)
                color = COLOR_WHITE;
            boxColor(surface, i * CELL_W, j * CELL_H, i * CELL_W + CELL_W, j * CELL_H + CELL_H, color);
        }
    }
}

int main(int argc, char * argv[]) {
    uint32_t generation = 0;
    init_gfx();
    init_random();
    run = true;

    init_world();
    while(run) {
        SDL_Event sdl_ev;
        if (SDL_PollEvent(&sdl_ev) != 0) {
            if (sdl_ev.type == SDL_KEYDOWN)
                switch (sdl_ev.key.keysym.sym) {
                    case SDLK_q:
                        run = false;
                        break;
                    case SDLK_SPACE:
                        pause = !pause;
                        break;
                    default:
                        break;
                }
        }
        if (!pause) {
            draw();
            SDL_Flip(surface);
            next_generation(); 
            generation++;
    /*        SDL_Delay(50); */
        }
    }
    SDL_Quit();
    return 0;
}

