// TODO(skelly): move visualization code the MujocoEnv.h
#ifndef evaluators_mujoco_h
#define evaluators_mujoco_h

#include <GLFW/glfw3.h>
#include <mujoco/mujoco.h>
#include <MujocoEnv.h>

#include <GL/osmesa.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <thread>
#include "ActionWrappers.h"
#include "EvalData.h"
#include "MujocoEnv.h"
#include "TPG.h"

/******************************************************************************/
// MuJoCo data structures
inline mjModel* m = NULL;  // MuJoCo model
inline mjData* d = NULL;   // MuJoCo data
inline mjvCamera cam;      // abstract camera
inline mjvOption opt;      // visualization options
inline mjvScene scn;       // abstract scene
inline mjrContext con;     // custom GPU context

// mouse interaction
inline bool button_left = false;
inline bool button_middle = false;
inline bool button_right = false;
inline double lastx = 0;
inline double lasty = 0;

// Add new global variables after existing globals
inline OSMesaContext osmesa_ctx = NULL;
inline unsigned char* osmesa_buffer = NULL;
inline bool headless = false;
inline int frame_idx = 0;

inline GLFWwindow* window = 0;

// keyboard callback
inline void keyboard(GLFWwindow* window, int key, int scancode, int act, int mods) {
    // backspace: reset simulation
    if (act == GLFW_PRESS && key == GLFW_KEY_BACKSPACE) {
        mj_resetData(m, d);
        mj_forward(m, d);
    }
}

// mouse button callback
inline void mouse_button(GLFWwindow* window, int button, int act, int mods) {
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
inline void mouse_move(GLFWwindow* window, double xpos, double ypos) {
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
inline void scroll(GLFWwindow* window, double xoffset, double yoffset) {
    // emulate vertical mouse motion = 5% of window height
    mjv_moveCamera(m, mjMOUSE_ZOOM, 0, -0.05 * yoffset, &scn, &cam);
}

inline void InitVisualization(mjModel* task_m, mjData* task_d) {
    m = task_m;
    d = task_d;

 // Check if we are in a headless environment
    const char* display = getenv("DISPLAY");
    headless = (display == NULL || strlen(display) == 0);

    if (!headless) {
        // Initialize GLFW
        if (!glfwInit()) {
            mju_error("Could not initialize GLFW");
        }

        // Create window, make OpenGL context current, request v-sync
        window = glfwCreateWindow(1200, 900, "Demo", NULL, NULL);
        if (!window) {
            mju_error("Could not create GLFW window");
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // Initialize visualization data structures
        mjv_defaultCamera(&cam);
        mjv_defaultOption(&opt);
        mjv_defaultScene(&scn);
        mjr_defaultContext(&con);

        // Create scene and context
        mjv_makeScene(m, &scn, 2000);
        mjr_makeContext(m, &con, mjFONTSCALE_150);

        // Install GLFW mouse and keyboard callbacks
        glfwSetKeyCallback(window, keyboard);
        glfwSetCursorPosCallback(window, mouse_move);
        glfwSetMouseButtonCallback(window, mouse_button);
        glfwSetScrollCallback(window, scroll);
    } else {
        // Headless mode using OSMesa
        osmesa_ctx = OSMesaCreateContextExt(OSMESA_RGBA, 24, 8, 8, NULL);
        if (!osmesa_ctx) {
            mju_error("OSMesa context creation failed");
        }

        // Allocate buffer for OSMesa
        int width = 1200;
        int height = 900;
        osmesa_buffer = (unsigned char*)malloc(width * height * 4 * sizeof(GLubyte));
        if (!osmesa_buffer) {
            mju_error("OSMesa buffer allocation failed");
        }

        // Bind the buffer to the context and make it current
        if (!OSMesaMakeCurrent(osmesa_ctx, osmesa_buffer, GL_UNSIGNED_BYTE, width, height)) {
            mju_error("OSMesa make current failed");
        }

        // Initialize visualization data structures
        mjv_defaultCamera(&cam);
        mjv_defaultOption(&opt);
        mjv_defaultScene(&scn);
        mjr_defaultContext(&con);

        // Create scene and context
        mjv_makeScene(m, &scn, 2000);
        mjr_makeContext(m, &con, mjFONTSCALE_150);

        // Initialize camera for headless rendering
        mjv_defaultFreeCamera(m, &cam);
    }
}

inline void StepVisualization(TPG& tpg) {
if (!headless) {
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
        // Headless mode
        // Set offscreen rendering
        mjr_setBuffer(mjFB_OFFSCREEN, &con);
        if (con.currentBuffer != mjFB_OFFSCREEN) {
            printf("Warning: offscreen rendering not supported, using default/window framebuffer\n");
        }

        // Get size of active renderbuffer
        mjrRect viewport = mjr_maxViewport(&con);

        // Update scene and render
        mjv_updateScene(m, d, &opt, NULL, &cam, mjCAT_ALL, &scn);
        mjr_render(viewport, &scn, &con);

        // Read pixels from the offscreen buffer
        unsigned char* rgb = (unsigned char*)malloc(3 * viewport.width * viewport.height);
        if (!rgb) {
            mju_error("Could not allocate buffer for rgb");
        }
        mjr_readPixels(rgb, NULL, viewport, &con);

        // Save the image to disk
        char filename[100];
        snprintf(filename, sizeof(filename), "frames/frame_%05d.ppm", frame_idx);
        FILE* fp = fopen(filename, "wb");
        if (!fp) {
            mju_error("Could not open file for writing");
        }
        fprintf(fp, "P6\n%d %d\n255\n", viewport.width, viewport.height);
        // PPM format expects top-to-bottom, left-to-right
        // MuJoCo's mjr_readPixels reads from bottom-left, so we need to flip vertically
        for (int row = viewport.height - 1; row >= 0; --row) {
            fwrite(&rgb[3 * row * viewport.width], 1, 3 * viewport.width, fp);
        }
        fclose(fp);

        free(rgb);

        frame_idx++;
    }
}

inline void MaybeStartAnimation(TPG& tpg, TaskEnv* task, EvalData& eval) {
    if (tpg.GetParam<int>("animate") && eval.episode == 0) {
        MujocoEnv* t = dynamic_cast<MujocoEnv*>(task);
        InitVisualization(t->m_, t->d_);
        // If in headless mode, create frames directory
        if (headless) {
            // Create frames directory if it doesn't exist
            struct stat st{};
            if (stat("frames", &st) == -1) {
                if (mkdir("frames", 0700) != 0) {
                    mju_error("Failed to create frames directory");
                }
            }
            // Reset frame index
            frame_idx = 0;
        }
    }
}

inline void MaybeAnimateStep(TPG& tpg) {
    if (tpg.GetParam<int>("animate")) {
        StepVisualization(tpg);
        this_thread::sleep_for(std::chrono::milliseconds(25));
    }
}

/******************************************************************************/
inline void EvalMujoco(TPG& tpg, EvalData& eval_data) {
   MujocoEnv* task = dynamic_cast<MujocoEnv*>(eval_data.task);
   task->reset(tpg.rngs_[AUX_SEED]);
   MaybeStartAnimation(tpg, task, eval_data);
   MaybeAnimateStep(tpg);
   eval_data.n_prediction = 0;
   eval_data.obs = new state(task->GetObsSize());
   eval_data.obs->Set(task->GetObsVec(eval_data.partially_observable));                 
   while (!task->terminal()) {
      tpg.GetAction(eval_data);
      auto ctrl = WrapVectorActionMuJoco(eval_data);
      TaskEnv::Results r = task->sim_step(ctrl);
      eval_data.stats_double[REWARD1_IDX] += r.r1;
      eval_data.AccumulateStepData();
      eval_data.n_prediction++;
      eval_data.obs->Set(task->GetObsVec(eval_data.partially_observable));
      MaybeAnimateStep(tpg);
   }
   delete eval_data.obs;
}

#endif