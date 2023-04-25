#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

const int WINDOW_WIDTH = 1900;
const int WINDOW_HEIGHT = 1200;
const int AGENT_COUNT = 100;
const float AGENT_SPEED = 2.0f;
const float SEPARATION_DISTANCE = 20.0f;
const float ALIGNMENT_DISTANCE = 50.0f;
const float COHESION_DISTANCE = 100.0f;
const float CURSOR_ATTRACTION_FACTOR = 1.55f;

typedef struct {
    float x, y;
    float vx, vy;
} Agent;

typedef struct {
    int x, y;
} Cursor;

void move_agent(Agent *agent) {
    agent->x += agent->vx;
    agent->y += agent->vy;

    // Wrap agent position around the screen
    if (agent->x < 0) agent->x += WINDOW_WIDTH;
    if (agent->x > WINDOW_WIDTH) agent->x -= WINDOW_WIDTH;
    if (agent->y < 0) agent->y += WINDOW_HEIGHT;
    if (agent->y > WINDOW_HEIGHT) agent->y -= WINDOW_HEIGHT;
}

void draw_agent(SDL_Renderer *renderer, Agent *agent) {
    SDL_Rect rect;
    rect.x = (int)agent->x;
    rect.y = (int)agent->y;
    rect.w = 4;
    rect.h = 4;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);
}

float distance(Agent *a, Agent *b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    return sqrtf(dx * dx + dy * dy);
}

void update_agent(Agent *agent, Agent *agents, int agent_count, Cursor cursor) {
    // Separation, alignment, cohesion, and cursor attraction vectors
    float sx = 0, sy = 0;
    float ax = 0, ay = 0;
    float cx = 0, cy = 0;
    float tx = 0, ty = 0;

    // Count the number of agents considered for alignment and cohesion
    int alignment_count = 0;
    int cohesion_count = 0;

    for (int i = 0; i < agent_count; i++) {
        Agent *other = &agents[i];
        if (other == agent) continue;

        float d = distance(agent, other);
        if (d < SEPARATION_DISTANCE) {
            sx += agent->x - other->x;
            sy += agent->y - other->y;
        }
        if (d < ALIGNMENT_DISTANCE) {
            ax += other->vx;
            ay += other->vy;
            alignment_count++;
        }
        if (d < COHESION_DISTANCE) {
            cx += other->x;
            cy += other->y;
            cohesion_count++;
        }
    }

    // Normalize separation vector
    float separation_length = sqrtf(sx * sx + sy * sy);
    if (separation_length > 0) {
        sx /= separation_length;
        sy /= separation_length;
    }

    // Normalize alignment vector
    if (alignment_count > 0) {
        ax /= alignment_count;
        ay /= alignment_count;
    }

    // Normalize cohesion vector
    if (cohesion_count > 0) {
        cx /= cohesion_count;
        cy /= cohesion_count;
        cx -= agent->x;
        cy -= agent->y;
        float cohesion_length = sqrtf(cx * cx + cy * cy);
        if (cohesion_length > 0) {
            cx /= cohesion_length;
            cy /= cohesion_length;
        }
    }

    // Calculate cursor attraction vector
    tx = cursor.x - agent->x;
    ty = cursor.y - agent->y;
    float cursor_distance = sqrtf(tx * tx + ty * ty);
    if (cursor_distance > 0) {
        tx /= cursor_distance;
        ty /= cursor_distance;
    }

    // Increase the influence of the separation force near the cursor
    float separation_factor = 1.0f + (SEPARATION_DISTANCE - cursor_distance) / SEPARATION_DISTANCE;

    // Update agent velocity based on separation, alignment, cohesion, and cursor attraction
    agent->vx += separation_factor * sx + ax + cx + CURSOR_ATTRACTION_FACTOR * tx;
    agent->vy += separation_factor * sy + ay + cy + CURSOR_ATTRACTION_FACTOR * ty;

    // Limit agent velocity to AGENT_SPEED
    float speed = sqrtf(agent->vx * agent->vx + agent->vy * agent->vy);
    if (speed > AGENT_SPEED) {
        agent->vx = (agent->vx / speed) * AGENT_SPEED;
        agent->vy = (agent->vy / speed) * AGENT_SPEED;
    }
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Swarming",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Agent agents[AGENT_COUNT];
    Cursor cursor;
    srand(time(NULL));

    // Initialize agent positions and velocities
    for (int i = 0; i < AGENT_COUNT; i++) {
        agents[i].x = rand() % WINDOW_WIDTH;
        agents[i].y = rand() % WINDOW_HEIGHT;
        agents[i].vx = (float)(rand() % 100 - 50) / 50.0f;
        agents[i].vy = (float)(rand() % 100 - 50) / 50.0f;
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEMOTION) {
                cursor.x = event.motion.x;
                cursor.y = event.motion.y;
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Update and draw agents
        for (int i = 0; i < AGENT_COUNT; i++) {
            update_agent(&agents[i], agents, AGENT_COUNT, cursor);
            move_agent(&agents[i]);
            draw_agent(renderer, &agents[i]);
        }

        // Present the updated screen
        SDL_RenderPresent(renderer);

        // Add a delay to limit the frame rate
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}