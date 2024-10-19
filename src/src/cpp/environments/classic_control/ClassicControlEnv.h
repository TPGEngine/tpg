#ifndef ClassicControlEnv_h
#define ClassicControlEnv_h

#include <GL/gl.h>
#include <GL/glut.h>
#include <TaskEnv.h>

using namespace std;

class ClassicControlEnv : public TaskEnv {
   public:
    uniform_real_distribution<> disReset;
    uniform_real_distribution<> disNoise;
    vector<deque<double> > actionTrace;
    ClassicControlEnv() {
        actionTrace.reserve(3);
        actionTrace.resize(3);
        for (size_t i = 0; i < 200; i++) {
            actionTrace[0].push_back(0);
            actionTrace[1].push_back(0);
            actionTrace[2].push_back(0);
        }
        disNoise = uniform_real_distribution<>(-M_PI, M_PI);
    }
    ~ClassicControlEnv() {}

    double bound(double x, double m, double M) { return min(max(x, m), M); }
    virtual void display_function(int, int, double) {};

    /**************************************************************************/
    void saveScreenshotToFile(std::string filename, int windowWidth,
                              int windowHeight) {
        const int numberOfPixels = windowWidth * windowHeight * 3;
        unsigned char pixels[numberOfPixels];
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_FRONT);
        glReadPixels(0, 0, windowWidth, windowHeight, GL_BGR_EXT,
                     GL_UNSIGNED_BYTE, pixels);
        FILE *outputFile = fopen(filename.c_str(), "w");
        short header[] = {
            0, 2, 0, 0, 0, 0, (short)windowWidth, (short)windowHeight, 24};
        fwrite(&header, sizeof(header), 1, outputFile);
        fwrite(pixels, numberOfPixels, 1, outputFile);
        fclose(outputFile);
    }

    void drawBitmapText(char *string, float x, float y, float z) {
        char *c;
        glRasterPos3f(x, y, z);
        for (c = string; *c != ':'; c++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }

    void drawStrokeText(char *string, float x, float y, float z) {
        char *c;
        glPushMatrix();
        glTranslatef(x, y, z);
        glScalef(0.001f, 0.001f, z);
        for (c = string; *c != ':'; c++) {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
        }
        glPopMatrix();
    }

    // actionProcessed should be in [-1.0,1.0]
    void drawTrace(int idx, string label, double actionProcessed,
                   double yActionTrace) {
        double traceXStep = 0.01;
        actionTrace[idx].push_front(0.1 * actionProcessed);
        actionTrace[idx].pop_back();
        glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
        glPointSize(1);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        double x = 0;
        for (size_t i = 0; i < actionTrace[idx].size(); i++) {
            glVertex2d(x, yActionTrace + actionTrace[idx][i]);
            x = x - traceXStep;
        }
        glEnd();

        // action text
        char c[80];
        strcpy(c, label.c_str());
        drawStrokeText(c, 0.05, yActionTrace, 0);
    }

    void drawEpisodeStepCounter(int episode, int step, float x, float y) {
        glColor3f(1.0, 1.0, 1.0);
        char c[80];
        (void)episode;
        sprintf(c, "Step %d%s", step, ":");
        drawStrokeText(c, x, y, 0);
    }

    vector<double> linspace(double a, double b, size_t N) {
        double h = (b - a) / static_cast<double>(N - 1);
        vector<double> xs(N);
        typename vector<double>::iterator x;
        double val;
        for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h) *x = val;
        return xs;
    }
};

#endif