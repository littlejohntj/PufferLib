/* Football: a sample multiagent env about puffers eating stars.
 * Use this as a tutorial and template for your own multiagent envs.
 * We suggest starting with the Squared env for a simpler intro.
 * Star PufferLib on GitHub to support. It really, really helps!
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raylib.h"
 
// Required struct. Only use floats!
typedef struct {
    float perf; // Recommended 0-1 normalized single real number perf metric
    float score; // Recommended unnormalized single real number perf metric
    float episode_return; // Recommended metric: sum of agent rewards over episode
    float episode_length; // Recommended metric: number of steps of agent episode
    // Any extra fields you add here may be exported to Python in binding.c
    float n; // Required as the last field 
} Log;

typedef struct {
    float quarter;
    float ball_on;
} Game;
 
typedef struct {
    Texture2D puffer;
    Texture2D star;
} Client;

typedef struct {
    float x;
    float y;
    float heading;
    float speed;
    int ticks_since_reward;
    float has_ball;
    float team;
} Agent;
 
// Required that you have some struct for your env
// Recommended that you name it the same as the env file
typedef struct {
    Log log; // Required field. Env binding code uses this to aggregate logs
    Client* client;
    Agent* agents;
    Game* game;
    float* observations; // Required. You can use any obs type, but make sure it matches in Python!
    int* actions; // Required. int* for discrete/multidiscrete, float* for box
    float* rewards; // Required
    unsigned char* terminals; // Required. We don't yet have truncations as standard yet
    int width;
    int height;
    int num_agents;
} Football;

/* Recommended to have an init function of some kind if you allocate 
* extra memory. This should be freed by c_close. Don't forget to call
* this in binding.c!
*/
void init(Football* env) {
    env->agents = calloc(env->num_agents, sizeof(Agent));
    env->game = calloc(1, sizeof(Game));
    // env->goals = calloc(env->num_goals, sizeof(Goal));
}

void reset_round(Football* env) {
    float starting_delta = 100;

    env->agents[0].has_ball = 1.0f;
    env->agents[0].x = env->width * 0.5;
    env->agents[0].y = ( env->height * 0.5 ) + starting_delta;
    env->agents[0].ticks_since_reward = 0;
    env->agents[0].team = 0.0f;

    env->agents[1].has_ball = 0.0f;
    env->agents[1].x = env->width * 0.5;
    env->agents[1].y = ( env->height * 0.5 ) - starting_delta;
    env->agents[1].ticks_since_reward = 0;
    env->agents[1].team = 1.0f;
}
 
void update_game(Football* env) {

    Agent* offense = &env->agents[0];
    Agent* defense = &env->agents[1];

    float dx = (offense->x - defense->x);
    float dy = (offense->y - defense->y);
    float dist = sqrt(dx*dx + dy*dy);

    if (dist <= 15) {
        env->rewards[1] = 1.0f;
        env->rewards[0] = -1.0f;
        env->log.perf += 1.0f;
        env->log.score += 1.0f;
        env->log.n++;
        env->log.episode_return += 1.0f;
        reset_round(env);
    } else if ( offense->y <= ( env->height * 0.083 ) ) {
        env->rewards[0] = 1.0f;
        env->rewards[1] = -1.0f;
        env->log.perf += 1.0f;
        env->log.score += 1.0f;
        env->log.n++;
        env->log.episode_return += 1.0f;
        reset_round(env);
    } else if ( defense->ticks_since_reward >= 512 ) {
        env->rewards[1] = -1.0f;
        env->rewards[0] = -1.0f;
        env->log.perf += 1.0f;
        env->log.score += 1.0f;
        env->log.n++;
        env->log.episode_return += 1.0f;
        reset_round(env);
    }
}
 
/* Recommended to have an observation function of some kind because
* you need to compute agent observations in both reset and in step.
* If using float obs, try to normalize to roughly -1 to 1 by dividing
* by an appropriate constant.
*/
void compute_observations(Football* env) {
    int obs_idx = 0;
    for (int a=0; a<env->num_agents; a++) {
        Agent* agent = &env->agents[a];
        for (int b=0; b<env->num_agents; b++) {
            Agent* other = &env->agents[b];
            env->observations[obs_idx++] = (other->x - agent->x)/env->width;
            env->observations[obs_idx++] = (other->y - agent->y)/env->height;
            // env->observations[obs_idx++] = (other->team == agent->team) ? 1.0f : 0.0f;
            // env->observations[obs_idx++] = other->has_ball;
        }
        env->observations[obs_idx++] = agent->heading/(2*PI);
        env->observations[obs_idx++] = env->rewards[a];
        env->observations[obs_idx++] = agent->x/env->width;
        env->observations[obs_idx++] = agent->y/env->height;
        // env->observations[obs_idx++] = agent->team;
        // env->observations[obs_idx++] = agent->has_ball;
    }
}

void reset_game(Football* env) {
    // env->game->
}


// Required function
void c_reset(Football* env) {
    reset_game(env);
    reset_round(env);
    compute_observations(env);
}

float clip(float val, float min, float max) {
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    }
    return val;
}

// Required function
void c_step(Football* env) {
    for (int i=0; i<env->num_agents; i++) {
        env->rewards[i] = 0;
        Agent* agent = &env->agents[i];
        agent->ticks_since_reward += 1;

        agent->heading += ((float)env->actions[2*i] - 4.0f)/12.0f;
        agent->heading = clip(agent->heading, 0, 2*PI);

        agent->speed += 1.0f*((float)env->actions[2*i + 1] - 2.0f);
        agent->speed = clip(agent->speed, -5.0f, 5.0f);

        agent->x += agent->speed*cosf(agent->heading);
        agent->x = clip(agent->x, 0, env->width);

        agent->y += agent->speed*sinf(agent->heading);
        agent->y = clip(agent->y, 0, env->height);

        // if (agent->ticks_since_reward % 512 == 0) {
        //     env->agents[i].x = rand() % env->width;
        //     env->agents[i].y = rand() % env->height;
        // }
    }
    update_game(env);
    compute_observations(env);
}
 
 // Required function. Should handle creating the client on first call
 void c_render(Football* env) {
     if (env->client == NULL) {
         InitWindow(env->width, env->height, "PufferLib Football");
         SetTargetFPS(60);
         env->client = (Client*)calloc(1, sizeof(Client));
 
         // Don't do this before calling InitWindow
        //  env->client->puffer = LoadTexture("resources/shared/puffers_128.png");
        //  env->client->star = LoadTexture("resources/football/star.png");
     }
 
     // Standard across our envs so exiting is always the same
    if (IsKeyDown(KEY_ESCAPE)) {
        exit(0);
    }
 
    BeginDrawing();
    ClearBackground((Color){116, 159, 55, 255});

    float border_size = 5;

    // Draw the field left side border
    DrawRectangle(
        0,
        0,
        border_size,
        env->height,
        WHITE
    );

    // Draw the field right side border
    DrawRectangle(
        env->width - border_size,
        0,
        border_size,
        env->height,
        WHITE
    );

    // Draw the field top border
    DrawRectangle(
        0,
        0,
        env->width,
        border_size,
        WHITE
    );

    // Draw the bottom border
    DrawRectangle(
        0,
        env->height - border_size,
        env->width,
        border_size,
        WHITE
    );
 
    for (int i=0; i<env->num_agents; i++) {

        Color player_color = (Color){255, 0, 0, 255};

        Agent* agent = &env->agents[i];

        if ( i == 0 ) {
            player_color =  (Color){240, 229, 146, 255};
        }

        float heading = agent->heading;
        float agent_size = 20;
        float half_agent = agent_size * 0.5;
        DrawRectangle(
            agent->x - half_agent, // X
            agent->y - half_agent, // Y
            agent_size, // Width
            agent_size, // Height
            player_color
        );
    }

    EndDrawing();
}
 
// Required function. Should clean up anything you allocated
// Do not free env->observations, actions, rewards, terminals
void c_close(Football* env) {
    free(env->agents);
    if (env->client != NULL) {
        Client* client = env->client;
        // UnloadTexture(client->puffer);
        // UnloadTexture(client->star);
        CloseWindow();
        free(client);
    }
}