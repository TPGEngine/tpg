# config.py
import os

def create_hyper_param_mapping(tpg_env_var):
    return {
        "half_cheetah": os.path.join(tpg_env_var, "configs", "MuJoCo_Half_Cheetah.yaml"),
        "classic_control": os.path.join(tpg_env_var, "configs", "Classic_Control.yaml"),
        "hopper": os.path.join(tpg_env_var, "configs", "MuJoCo_Hopper.yaml"),
        "reacher": os.path.join(tpg_env_var, "configs", "MuJoCo_Reacher.yaml"),
        "humanoid_standup": os.path.join(tpg_env_var, "configs", "MuJoCo_Humanoid_Standup.yaml"),
        "inverted_double_pendulum": os.path.join(tpg_env_var, "configs", "MuJoCo_Inverted_Double_Pendulum.yaml"),
        "inverted_pendulum": os.path.join(tpg_env_var, "configs", "MuJoCo_Inverted_Pendulum.yaml"),
        "ant": os.path.join(tpg_env_var, "configs", "MuJoCo_Ant.yaml"),
        "mujoco_multitask": os.path.join(tpg_env_var, "configs", "MuJoCo_MultiTask.yaml"),
        "mujoco_multitask_half_cheetah": os.path.join(tpg_env_var, "configs", "MuJoCo_MultiTask_Half_Cheetah.yaml"),
    }

TPG = os.getenv('TPG')
HYPER_PARAM_MAPPING = create_hyper_param_mapping(TPG)
