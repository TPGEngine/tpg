#ifndef evaluators_mujoco_h
#define evaluators_mujoco_h

#include <mujoco/mujoco.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Variables for visualization
mjModel* m = NULL;  // MuJoCo model
mjData* d = NULL;   // MuJoCo data
mjvCamera cam;      // abstract camera
mjvOption opt;      // visualization options
mjvScene scn;       // abstract scene
mjrContext con;     // custom GPU context

// Mouse interaction
bool button_left = false;
bool button_middle = false;
bool button_right = false;
double lastx = 0;
double lasty = 0;

GLFWwindow* window = nullptr;
bool is_headless = false;
int offscreen_width;
int offscreen_height;
int frame_counter = 0;

// Keyboard callback
void keyboard(GLFWwindow* window, int key, int scancode, int act, int mods) {
    // Backspace: reset simulation
    if (act == GLFW_PRESS && key == GLFW_KEY_BACKSPACE) {
        mj_resetData(m, d);
        mj_forward(m, d);
    }
}

// Mouse button callback
void mouse_button(GLFWwindow* window, int button, int act, int mods) {
    // Update button state
    button_left =
        (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    button_middle =
        (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    button_right =
        (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

    // Update mouse position
    glfwGetCursorPos(window, &lastx, &lasty);
}

// Mouse move callback
void mouse_move(GLFWwindow* window, double xpos, double ypos) {
    // No buttons down: nothing to do
    if (!button_left && !button_middle && !button_right) {
        return;
    }

    // Compute mouse displacement, save
    double dx = xpos - lastx;
    double dy = ypos - lasty;
    lastx = xpos;
    lasty = ypos;

    // Get current window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Get shift key state
    bool mod_shift = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    // Determine action based on mouse button
    mjtMouse action;
    if (button_right) {
        action = mod_shift ? mjMOUSE_MOVE_H : mjMOUSE_MOVE_V;
    } else if (button_left) {
        action = mod_shift ? mjMOUSE_ROTATE_H : mjMOUSE_ROTATE_V;
    } else {
        action = mjMOUSE_ZOOM;
    }

    // Move camera
    mjv_moveCamera(m, action, dx / height, dy / height, &scn, &cam);
}

// Scroll callback
void scroll(GLFWwindow* window, double xoffset, double yoffset) {
    // Emulate vertical mouse motion = 5% of window height
    mjv_moveCamera(m, mjMOUSE_ZOOM, 0, -0.05 * yoffset, &scn, &cam);
}

void InitVisualization(mjModel* task_m, mjData* task_d) {
    m = task_m;
    d = task_d;
    is_headless = false;

    // Initialize GLFW
    if (!glfwInit()) {
        mju_error("Could not initialize GLFW");
    }

    // Set desired OpenGL version (optional)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Create window, make OpenGL context current, request v-sync
    window = glfwCreateWindow(1200, 900, "Demo", NULL, NULL);
    if (!window) {
        std::cerr << "Could not create GLFW window, falling back to offscreen rendering." << std::endl;
        is_headless = true;
    } else {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
    }

    // Initialize visualization data structures
    mjv_defaultCamera(&cam);
    mjv_defaultOption(&opt);
    mjv_defaultScene(&scn);
    mjr_defaultContext(&con);

    // Create scene and context
    mjv_makeScene(m, &scn, 2000);
    mjr_makeContext(m, &con, mjFONTSCALE_150);

    // Set the offscreen buffer size
    offscreen_width = 1200;
    offscreen_height = 900;
    m->vis.global.offwidth = offscreen_width;
    m->vis.global.offheight = offscreen_height;

    if (is_headless) {
        // Set offscreen buffer
        mjr_setBuffer(mjFB_OFFSCREEN, &con);
    } else {
        // Install GLFW mouse and keyboard callbacks
        glfwSetKeyCallback(window, keyboard);
        glfwSetCursorPosCallback(window, mouse_move);
        glfwSetMouseButtonCallback(window, mouse_button);
        glfwSetScrollCallback(window, scroll);
    }
}

void StepVisualization() {
    if (!is_headless) {
        // Get framebuffer viewport
        mjrRect viewport = {0, 0, 0, 0};
        glfwGetFramebufferSize(window, &viewport.width, &viewport.height);

        // Update scene and render
        mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
        mjr_render(viewport, &scn, &con);

        // Swap OpenGL buffers (blocking call due to v-sync)
        glfwSwapBuffers(window);

        // Process pending GUI events, call GLFW callbacks
        glfwPollEvents();
    } else {
        mjrRect viewport = {0, 0, offscreen_width, offscreen_height};

        // Update scene and render
        mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
        mjr_render(viewport, &scn, &con);

        // Allocate image buffer
        unsigned char* rgb = (unsigned char*)malloc(offscreen_width * offscreen_height * 3);
        float* depth = (float*)malloc(offscreen_width * offscreen_height * sizeof(float));

        // Read pixels from the offscreen buffer
        mjr_readPixels(rgb, depth, viewport, &con);

        // Flip the image vertically
        int row_stride = offscreen_width * 3;
        unsigned char* flipped_rgb = (unsigned char*)malloc(offscreen_width * offscreen_height * 3);
        for (int i = 0; i < offscreen_height; ++i) {
            memcpy(flipped_rgb + i * row_stride, rgb + (offscreen_height - 1 - i) * row_stride, row_stride);
        }

        // Ensure the directory exists
        int result = system("mkdir -p replay/frames");
        if (result != 0) {
            std::cerr << "Failed to create replay/frames directory. mkdir returned " << result << std::endl;
        }

        // Save image to file
        char filename[256];
        sprintf(filename, "replay/frames/frame_%05d.png", frame_counter);
        stbi_write_png(filename, offscreen_width, offscreen_height, 3, flipped_rgb, offscreen_width * 3);
        frame_counter++;

        free(rgb);
        free(flipped_rgb);
        free(depth);
    }
}

void CloseVisualization() {
    // Free visualization storage
    mjv_freeScene(&scn);
    mjr_freeContext(&con);

    if (!is_headless) {
        // Terminate GLFW
        glfwDestroyWindow(window);
#if defined(__APPLE__) || defined(_WIN32)
        glfwTerminate();
#endif
    }
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

void MaybeCloseAnimation(TPG& tpg) {
    if (tpg.GetParam<int>("animate")) {
        CloseVisualization();
    }
}

bool IsHeadless() {
    return is_headless;
}

int GetFrameCounter() {
    return frame_counter;
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
        TaskEnv::Results r = task->sim_step(ctrl);
        eval.stats_double[REWARD1_IDX] += r.r1;
        eval.AccumulateStepData();
        eval.n_prediction++;
        obs->Set(task->GetObsVec(eval.partially_observable));
        MaybeAnimateStep(tpg);
    }
    delete obs;
    MaybeCloseAnimation(tpg);
}

#endif
