#ifndef tpg_arg_parse_h
#define tpg_arg_parse_h
#include <TPG.h>
#include <unistd.h>

void tpg_arg_parse(TPG& tpg, int argc, char** argv) {
  int option_char;
  while ((option_char = getopt(argc, argv, "aC:g:R:r:s:t:p:")) != -1) {
    switch (option_char) {
      case 'C': {
        tpg.params_["checkpoint"] = 1;
        tpg.params_["checkpoint_in_phase"] = atoi(optarg);
        break;
      }
      case 'g': {
        uint_fast32_t s = uint_fast32_t(atoi(optarg));
        tpg.Seed(AUX_SEED, s);
        break;
      }
      case 'R': {
        tpg.params_["replay"] = 1;
        tpg.params_["host_to_replay"] = atoi(optarg);
        break;
      }
      case 'r': {
        tpg.state_["replay_task"] = atoi(optarg);
        break;
      }
      case 's': {
        uint_fast32_t s = uint_fast32_t(atoi(optarg));
        tpg.Seed(TPG_SEED, s);
        tpg.Seed(AUX_SEED, s);
        break;
      }
      case 't': {
        tpg.params_["active_task"] = atoi(optarg);
        break;
      }
      case 'p': {
        tpg.params_["t_pickup"] = atoi(optarg);
        tpg.params_["t_start"] = atoi(optarg) + 1;
        break;
      }
      case 'a': {
        tpg.params_["animate"] = 1;
        break;
      }
      case '?': {
        exit(0);
        break;
      }
    }
  }
}

#endif
