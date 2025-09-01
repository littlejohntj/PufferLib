/* Pure C demo file for Football. Build it with:
 * bash scripts/build_ocean.sh football local (debug)
 * bash scripts/build_ocean.sh football fast
 */
 #include "football.h"
 #include "puffernet.h"
 
 int main() {
     int num_agents = 2;
     int num_goals = 1;
     int num_obs = 2*(num_agents + num_goals) + 4;
 
     // Weights are exported by running puffer export
     Weights* weights = load_weights("resources/football/football.bin", 137743);
 
     int logit_sizes[2] = {9, 5};
     LinearLSTM* net = make_linearlstm(weights, num_agents, num_obs, logit_sizes, 2);
 
     Football env = {
         .width = 318,
         .height = 720,
         .num_agents = num_agents,
         .num_goals = num_goals 
     };
     init(&env);
 
     // Allocate these manually since they aren't being passed from Python
     env.observations = calloc(env.num_agents*num_obs, sizeof(float));
     env.actions = calloc(2*env.num_agents, sizeof(int));
     env.rewards = calloc(env.num_agents, sizeof(float));
     env.terminals = calloc(env.num_agents, sizeof(unsigned char));
 
     // Always call reset and render first
     c_reset(&env);
     c_render(&env);
 
     // while(True) will break web builds
     while (!WindowShouldClose()) {
         for (int i=0; i<env.num_agents; i++) {
             env.actions[2*i] = rand() % 9;
             env.actions[2*i + 1] = rand() % 5;
         }
 
         forward_linearlstm(net, env.observations, env.actions);
         c_step(&env);
         c_render(&env);
     }
 
     // Try to clean up after yourself
     free_linearlstm(net);
     free(env.observations);
     free(env.actions);
     free(env.rewards);
     free(env.terminals);
     c_close(&env);
 }
 
 