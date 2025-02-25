#ifndef ClassicControlEnv_h
#define ClassicControlEnv_h

#include <GL/gl.h>
#include <GL/glut.h>
#include <TaskEnv.h>

class ClassicControlEnv : public TaskEnv {
   public:
    std::uniform_real_distribution<> dis_reset;
    std::uniform_real_distribution<> dis_noise;
    std::vector<std::deque<double>> action_trace;

    ClassicControlEnv() {
        action_trace.reserve(3);
        action_trace.resize(3);
        for (size_t i = 0; i < 200; i++) {
            action_trace[0].push_back(0);
            action_trace[1].push_back(0);
            action_trace[2].push_back(0);
        }
        dis_noise = std::uniform_real_distribution<>(-M_PI, M_PI);
    }

    ~ClassicControlEnv() {}

    //! Bounds a value x between m and M
    double Bound(double x, double m, double M) {
        return std::min(std::max(x, m), M);
    }
    virtual void DisplayFunction(int, int, double){};

    //! Saves current OpenGL frame buffer as a screenshot
    void SaveScreenshotToFile(std::string filename, int window_width,
                              int window_height) {
        const int kNumberOfPixels = window_width * window_height * 3;
        unsigned char pixels[kNumberOfPixels];
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, window_width, window_height, GL_BGR_EXT,
                     GL_UNSIGNED_BYTE, pixels);
        FILE* output_file = std::fopen(filename.c_str(), "w");
        short header[] = {
            0, 2, 0, 0, 0, 0, (short)window_width, (short)window_height, 24};
        std::fwrite(&header, sizeof(header), 1, output_file);
        std::fwrite(pixels, kNumberOfPixels, 1, output_file);
        std::fclose(output_file);
    }

    //! Draws bitmap text at specified 3D coordinates
    void DrawBitmapText(char* string, float x, float y, float z) {
        char* c;
        glRasterPos3f(x, y, z);
        for (c = string; *c != ':'; c++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }

    //! Draws stroke text at specified 3D coordinates with scaling
    void DrawStrokeText(char* string, float x, float y, float z) {
        char* c;
        glPushMatrix();
        glTranslatef(x, y, z);
        glScalef(0.001f, 0.001f, z);
        for (c = string; *c != ':'; c++) {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
        }
        glPopMatrix();
    }

    //! Draws a trace of action values over time
    // action_processed should be in [-1.0,1.0]
    void DrawTrace(int idx, std::string label, double action_processed,
                   double y_action_trace) {
        double trace_x_step = 0.01;
        action_trace[idx].push_front(0.1 * action_processed);
        action_trace[idx].pop_back();
        glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
        glPointSize(1);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        double x = 0;
        for (size_t i = 0; i < action_trace[idx].size(); i++) {
            glVertex2d(x, y_action_trace + action_trace[idx][i]);
            x = x - trace_x_step;
        }
        glEnd();

        // action text
        char c[80];
        std::strcpy(c, label.c_str());
        DrawStrokeText(c, 0.05, y_action_trace, 0);
    }
    
    //! Draws the episode and step counter on the screen
    void DrawEpisodeStepCounter(int episode, int step, float x, float y) {
        glColor3f(1.0, 1.0, 1.0);
        char c[80];
        (void)episode;
        std::sprintf(c, "Step %d%s", step, ":");
        DrawStrokeText(c, x, y, 0);
    }

    //! Creates a vector of evenly spaced values between a and b
    std::vector<double> Linspace(double a, double b, size_t N) {
        double h = (b - a) / static_cast<double>(N - 1);
        std::vector<double> xs(N);
        typename std::vector<double>::iterator x;
        double val;
        for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h)
            *x = val;
        return xs;
    }
};

#endif