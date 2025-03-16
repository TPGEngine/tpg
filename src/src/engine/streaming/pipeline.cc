#include "pipeline.h"
#include <iostream>

bool GStreamerPipeline::initialize(int width, int height, int fps) {
    if (initialized) {
        return true; // Already initialized.
    }

    // Create the pipeline element.
    pipeline = gst_pipeline_new("webrtc-pipeline");
    if (!pipeline) {
        std::cerr << "[GStreamerPipeline] Failed to create pipeline." << std::endl;
        return false;
    }

    // Create individual elements
    appsrc   = gst_element_factory_make("appsrc", "mysource");
    GstElement* videoconvert = gst_element_factory_make("videoconvert", "convert");
    GstElement* vp8enc       = gst_element_factory_make("vp8enc", "encoder");
    GstElement* rtpvp8pay    = gst_element_factory_make("rtpvp8pay", "rtppay");
    GstElement* webrtcbin    = gst_element_factory_make("webrtcbin", "sendrecv");
    // GstElement* fakesink = gst_element_factory_make("fakesink", "sink");


    if (!appsrc || !videoconvert || !vp8enc || !rtpvp8pay || !webrtcbin) {
        std::cerr << "[GStreamerPipeline] Failed to create one or more elements." << std::endl;
        return false;
    }

    // Add all elements into the pipeline.
    gst_bin_add_many(GST_BIN(pipeline), appsrc, videoconvert, vp8enc, rtpvp8pay, webrtcbin, NULL);

    // Link appsrc -> videoconvert -> vp8enc -> rtpvp8pay
    if (!gst_element_link_many(appsrc, videoconvert, vp8enc, rtpvp8pay, NULL)) {
        std::cerr << "[GStreamerPipeline] Failed to link source to rtpvp8pay." << std::endl;
        return false;
    }

    //Link rtpvp8pay to webrtcbin. Note: webrtcbin uses dynamic pads, so we must request one.
    GstPad* srcPad = gst_element_get_static_pad(rtpvp8pay, "src");
    GstPad* sinkPad = gst_element_request_pad_simple(webrtcbin, "sink_%u");
    if (gst_pad_link(srcPad, sinkPad) != GST_PAD_LINK_OK) {
        std::cerr << "[GStreamerPipeline] Failed to link rtpvp8pay to webrtcbin." << std::endl;
        gst_object_unref(srcPad);
        gst_object_unref(sinkPad);
        return false;
    }
    gst_object_unref(srcPad);
    gst_object_unref(sinkPad);

    // Set appsrc properties.
    // Create caps: video/x-raw in RGB format.
    GstCaps* caps = gst_caps_new_simple("video/x-raw",
                                        "format", G_TYPE_STRING, "RGB",
                                        "width", G_TYPE_INT, width,
                                        "height", G_TYPE_INT, height,
                                        "framerate", GST_TYPE_FRACTION, fps, 1,
                                        NULL);
    g_object_set(appsrc, "caps", caps, "format", GST_FORMAT_TIME, NULL);
    gst_caps_unref(caps);

    // Set the pipeline to PLAYING state.
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "[GStreamerPipeline] Unable to set pipeline to PLAYING state." << std::endl;
        return false;
    }

    // Reset timestamp and mark as initialized.
    timestamp = 0;
    initialized = true;

    std::cout << "[GStreamerPipeline] Pipeline initialized successfully." << std::endl;
    return true;
}

bool GStreamerPipeline::pushFrame(unsigned char* data, int size) {
    if (!initialized) {
        std::cerr << "[GStreamerPipeline] Pipeline not initialized." << std::endl;
        return false;
    }

    // Allocate a new GstBuffer with the frame data.
    GstBuffer* buffer = gst_buffer_new_allocate(NULL, size, NULL);
    if (!buffer) {
        std::cerr << "[GStreamerPipeline] Failed to allocate GstBuffer." << std::endl;
        return false;
    }
    gst_buffer_fill(buffer, 0, data, size);

    // Set the presentation timestamp (PTS) and duration.
    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30);  // assuming 30 fps
    timestamp += GST_BUFFER_DURATION(buffer);

    // Push the buffer into appsrc.
    GstFlowReturn flowRet;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &flowRet);
    gst_buffer_unref(buffer);

    std::cout << "[GStreamerPipeline] Pushing frame at timestamp: " << timestamp << std::endl;

    if (flowRet != GST_FLOW_OK) {
        std::cerr << "[GStreamerPipeline] push-buffer returned error: " << flowRet << std::endl;
        return false;
    }

    return true;
}

void GStreamerPipeline::shutdown() {
    if (initialized) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
        appsrc = nullptr;
        initialized = false;
        std::cout << "[GStreamerPipeline] Pipeline shut down." << std::endl;
    }
}
