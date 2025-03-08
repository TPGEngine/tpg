import os
import glob

def generate_key_from_filename(filename):
    """
    Transform the filename into a suitable key.
    For example, transform 'MuJoCo_Inverted_Pendulum.yaml'
    into 'inverted_pendulum'.
    """
    # Remove the extension (.yaml, .yml)
    name, _ = os.path.splitext(filename)

    # Optionally remove a known prefix
    prefix = "MuJoCo_"
    if name.startswith(prefix):
        name = name[len(prefix):]

    # Return the cleaned-up, lower-case key
    return name.lower()


def create_hyper_param_mapping(tpg_env_var):
    """
    Create mapping automatically by scanning the configs directory.
    Each YAML file in the directory is processed to produce a command key.
    """
    config_dir = os.path.join(tpg_env_var, "configs")
    mapping = {}

    # Use glob to list all yaml files
    yaml_files = glob.glob(os.path.join(config_dir, "*.yaml"))

    for filepath in yaml_files:
        filename = os.path.basename(filepath)
        key = generate_key_from_filename(filename)
        mapping[key] = filepath

    return mapping


TPG = os.getenv("TPG")
HYPER_PARAM_MAPPING = create_hyper_param_mapping(TPG)

# Debug print (optional)
if __name__ == "__main__":
    import pprint
    pprint.pprint(HYPER_PARAM_MAPPING)
