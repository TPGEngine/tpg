# config.py
import os

def create_hyper_param_mapping(tpg_env_var):
    return {
        "half_cheetah": os.path.join(tpg_env_var, "configs", "2024-01-18_TPG_MuJoco_Half_Cheetah.txt"),
        "classic_control": os.path.join(tpg_env_var, "configs", "2025-02-03-Classic-Control.txt"),
        "hopper": os.path.join(tpg_env_var, "configs", "2025-02-13_TPG_MuJoco_Hopper.txt"),
        "reacher": os.path.join(tpg_env_var, "configs", "2025-02-03-MuJoco_Reacher.txt"),
        "humanoid_standup": os.path.join(tpg_env_var, "configs", "2025-02-05_TPG_MuJoco_Humanoid_Standup.txt"),
        "inverted_double_pendulum": os.path.join(tpg_env_var, "configs", "2025-02-05_TPG_MuJoco_Inverted_Double_Pendulum.txt"),
        "inverted_pendulum": os.path.join(tpg_env_var, "configs", "2025-02-05_TPG_MuJoco_Inverted_Pendulum.txt"),
        "mujoco_multi_task": os.path.join(tpg_env_var, "configs", "2025-02-05_TPG_MuJoco_MultiTask.txt"),
    }

TPG = os.getenv('TPG')
HYPER_PARAM_MAPPING = create_hyper_param_mapping(TPG)
