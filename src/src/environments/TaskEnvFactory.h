#ifndef TASK_ENV_FACTORY_H
#define TASK_ENV_FACTORY_H

#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <any>
#include "TaskEnv.h"

class TaskEnvFactory {
 public:
  using CreatorFunc = std::function<TaskEnv*(std::unordered_map<std::string, std::any>&)>;
  
  static TaskEnv* createTask(const std::string& name, std::unordered_map<std::string, std::any>& params);
};

#endif
