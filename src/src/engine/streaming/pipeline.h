// In GStreamerPipeline.h, modify the class to provide a singleton getter.
#ifndef GSTREAMER_PIPELINE_H
#define GSTREAMER_PIPELINE_H

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

class GStreamerPipeline {
public:
    // Public static method to get the single instance.
    static GStreamerPipeline& getInstance() {
        static GStreamerPipeline instance;
        return instance;
    }

    // Delete copy constructor and assignment operator.
    GStreamerPipeline(const GStreamerPipeline&) = delete;
    void operator=(const GStreamerPipeline&) = delete;

    // Your public methods:
    bool initialize(int width, int height, int fps);
    bool pushFrame(unsigned char* data, int size);
    void shutdown();
    bool isInitialized() const { return initialized; }

private:
    // Private constructor ensures no external instantiation.
    GStreamerPipeline() : pipeline(nullptr), appsrc(nullptr), timestamp(0), initialized(false) {
        gst_init(nullptr, nullptr);
    }
    ~GStreamerPipeline() {}

    // Private member variables.
    GstElement* pipeline;
    GstElement* appsrc;
    GstClockTime timestamp;
    bool initialized;
};

#endif