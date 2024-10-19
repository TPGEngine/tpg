// TODO(skelly): move visualization code the MujocoEnv.h
#ifndef evaluators_mujoco_h
#define evaluators_mujoco_h

#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>

/******************************************************************************/
// MuJoCo data structures
mjModel* m = NULL;  // MuJoCo model
mjData* d = NULL;   // MuJoCo data
mjvCamera cam;      // abstract camera
mjvOption opt;      // visualization options
mjvScene scn;       // abstract scene
mjrContext con;     // custom GPU context

// mouse interaction
bool button_left = false;
bool button_middle = false;
bool button_right = false;
double lastx = 0;
double lasty = 0;

GLFWwindow* window = 0;

// keyboard callback
void keyboard(GLFWwindow* window, int key, int scancode, int act, int mods) {
    // backspace: reset simulation
    if (act == GLFW_PRESS && key == GLFW_KEY_BACKSPACE) {
        mj_resetData(m, d);
        mj_forward(m, d);
    }
}

// mouse button callback
void mouse_button(GLFWwindow* window, int button, int act, int mods) {
    // update button state
    button_left =
        (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    button_middle =
        (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    button_right =
        (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

    // update mouse position
    glfwGetCursorPos(window, &lastx, &lasty);
}

// mouse move callback
void mouse_move(GLFWwindow* window, double xpos, double ypos) {
    // no buttons down: nothing to do
    if (!button_left && !button_middle && !button_right) {
        return;
    }

    // compute mouse displacement, save
    double dx = xpos - lastx;
    double dy = ypos - lasty;
    lastx = xpos;
    lasty = ypos;

    // get current window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // get shift key state
    bool mod_shift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    // determine action based on mouse button
    mjtMouse action;
    if (button_right) {
        action = mod_shift ? mjMOUSE_MOVE_H : mjMOUSE_MOVE_V;
    } else if (button_left) {
        action = mod_shift ? mjMOUSE_ROTATE_H : mjMOUSE_ROTATE_V;
    } else {
        action = mjMOUSE_ZOOM;
    }

    // move camera
    mjv_moveCamera(m, action, dx / height, dy / height, &scn, &cam);
}

// scroll callback
void scroll(GLFWwindow* window, double xoffset, double yoffset) {
    // emulate vertical mouse motion = 5% of window height
    mjv_moveCamera(m, mjMOUSE_ZOOM, 0, -0.05 * yoffset, &scn, &cam);
}

void InitVisualization(mjModel* task_m, mjData* task_d) {
    m = task_m;
    d = task_d;

    // init GLFW
    if (!glfwInit()) {
        mju_error("Could not initialize GLFW");
    }

    // create window, make OpenGL context current, request v-sync
    // GLFWwindow* window = glfwCreateWindow(1200, 900, "Demo", NULL, NULL);
    window = glfwCreateWindow(1200, 900, "Demo", NULL, NULL);
    cout << "WINDOW1:";
    cout << glfwWindowShouldClose(window) << endl;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // initialize visualization data structures
    mjv_defaultCamera(&cam);
    mjv_defaultOption(&opt);
    mjv_defaultScene(&scn);
    mjr_defaultContext(&con);

    // create scene and context
    mjv_makeScene(m, &scn, 2000);
    mjr_makeContext(m, &con, mjFONTSCALE_150);

    // install GLFW mouse and keyboard callbacks
    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, mouse_move);
    glfwSetMouseButtonCallback(window, mouse_button);
    glfwSetScrollCallback(window, scroll);

    cout << "WINDOW2:" << glfwWindowShouldClose(window) << endl;
}

void StepVisualization() {
    // get framebuffer viewport
    mjrRect viewport = {0, 0, 0, 0};
    glfwGetFramebufferSize(window, &viewport.width, &viewport.height);

    // update scene and render
    mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
    mjr_render(viewport, &scn, &con);

    // swap OpenGL buffers (blocking call due to v-sync)
    glfwSwapBuffers(window);

    // process pending GUI events, call GLFW callbacks
    glfwPollEvents();
}

void MaybeStartAnimation(TPG& tpg, TaskEnv* task) {
    if (tpg.GetParam<int>("animate")) {
        MujocoEnv* t = dynamic_cast<MujocoEnv*>(task);
        InitVisualization(t->m_, t->d_);
    }
}

void MaybeAnimateStep(TPG& tpg) {
    if (tpg.GetParam<int>("animate")) {
        StepVisualization();
    }
}

/******************************************************************************/
void EvalMujoco(TPG& tpg, EvalData& eval) {
    MujocoEnv* task = dynamic_cast<MujocoEnv*>(eval.task);
    task->reset(tpg.rngs_[AUX_SEED]);
    MaybeStartAnimation(tpg, task);
    MaybeAnimateStep(tpg);
    eval.n_prediction = 0;
    state* obs = new state(task->GetObsSize());
    obs->Set(task->GetObsVec(eval.partially_observable));
    while (!task->terminal()) {
        eval.program_out = tpg.getAction(
            eval.tm, obs, true, eval.teams_visited, eval.instruction_count,
            task->step_, eval.team_path, tpg.rngs_[AUX_SEED], false);
        auto ctrl = WrapVectorActionTanh(eval);
        TaskEnv::Results r = task->sim_step(ctrl);  // TODO(skelly): slow?
        eval.stats_double[REWARD1_IDX] += r.r1;
        eval.AccumulateStepData();
        eval.n_prediction++;
        obs->Set(task->GetObsVec(eval.partially_observable));
        MaybeAnimateStep(tpg);
    }
    delete obs;
}

#endif