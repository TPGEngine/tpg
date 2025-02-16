#include "TaskEnvFactory.h"
#include "CartPole.h"
#include "Acrobot.h"
#include "CartCentering.h"
#include "Pendulum.h"
#include "MountainCar.h"
#include "MountainCarContinuous.h"
#include "RecursiveForecast.h"
#include "Mujoco_Ant_v4.h"
#include "Mujoco_Inverted_Pendulum_v4.h"
#include "Mujoco_Inverted_Double_Pendulum_v4.h"
#include "Mujoco_Half_Cheetah_v4.h"
#include "Mujoco_Reacher_v4.h"
#include "Mujoco_Hopper_v4.h"
#include "Mujoco_Humanoid_Standup_v4.h"

TaskEnv* TaskEnvFactory::createTask(const std::string& name, std::unordered_map<std::string, std::any>& params) {
    static const std::map<std::string, CreatorFunc> registry = {
        {"CartPole", [](std::unordered_map<std::string, std::any>&) { return new CartPole(); }},
        {"Acrobot", [](std::unordered_map<std::string, std::any>&) { return new Acrobot(); }},
        {"CartCentering", [](std::unordered_map<std::string, std::any>&) { return new CartCentering(); }},
        {"Pendulum", [](std::unordered_map<std::string, std::any>&) { return new Pendulum(); }},
        {"MountainCar", [](std::unordered_map<std::string, std::any>&) { return new MountainCar(); }},
        {"MountainCarContinuous", [](std::unordered_map<std::string, std::any>&) { return new MountainCarContinuous(); }},
        {"Sunspots", [](std::unordered_map<std::string, std::any>&) { return new RecursiveForecast("Sunspots"); }},
        {"Mackey", [](std::unordered_map<std::string, std::any>&) { return new RecursiveForecast("Mackey"); }},
        {"Laser", [](std::unordered_map<std::string, std::any>&) { return new RecursiveForecast("Laser"); }},
        {"Offset", [](std::unordered_map<std::string, std::any>&) { return new RecursiveForecast("Offset"); }},
        {"Duration", [](std::unordered_map<std::string, std::any>&) { return new RecursiveForecast("Duration"); }},
        {"Pitch", [](std::unordered_map<std::string, std::any>&) { return new RecursiveForecast("Pitch"); }},
        {"PitchBach", [](std::unordered_map<std::string, std::any>&) { return new RecursiveForecast("PitchBach"); }},
        {"Bach", [](std::unordered_map<std::string, std::any>&) { return new RecursiveForecast("Bach"); }},
        {"Mujoco_Ant_v4", [](std::unordered_map<std::string, std::any>& params) { return new Mujoco_Ant_v4(params); }},
        {"Mujoco_Inverted_Pendulum_v4", [](std::unordered_map<std::string, std::any>& params) { return new Mujoco_Inverted_Pendulum_v4(params); }},
        {"Mujoco_Inverted_Double_Pendulum_v4", [](std::unordered_map<std::string, std::any>& params) { return new Mujoco_Inverted_Double_Pendulum_v4(params); }},
        {"Mujoco_Half_Cheetah_v4", [](std::unordered_map<std::string, std::any>& params) { return new Mujoco_Half_Cheetah_v4(params); }},
        {"Mujoco_Reacher_v4", [](std::unordered_map<std::string, std::any>& params) { return new Mujoco_Reacher_v4(params); }},
        {"Mujoco_Hopper_v4", [](std::unordered_map<std::string, std::any>& params) { return new Mujoco_Hopper_v4(params); }},
        {"Mujoco_Humanoid_Standup_v4", [](std::unordered_map<std::string, std::any>& params) { return new Mujoco_Humanoid_Standup_v4(params); }},
    };

    auto it = registry.find(name);
    if (it != registry.end()) {
        return it->second(params);
    }
    return nullptr;
}
