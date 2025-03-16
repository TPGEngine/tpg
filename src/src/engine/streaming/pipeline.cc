#include "pipeline.h"
#include <iostream>

GStreamerPipeline::GStreamerPipeline() : pipeline(nullptr), appsrc(nullptr), timestamp(0) {
    // Initialize GStreamer (if not already done)
    gst_init(nullptr, nullptr);
}

GStreamerPipeline::~GStreamerPipeline() {
    shutdown();
}

bool GStreamerPipeline::initialize(int width, int height, int fps) {
    // Create elements: appsrc, videoconvert, encoder, RTP payloader, and webrtcbin.
    appsrc = gst_element_factory_make("appsrc", "mysource");
    GstElement* videoconvert = gst_element_factory_make("videoconvert", "convert");
    GstElement* vp8enc = gst_element_factory_make("vp8enc", "encoder");
    GstElement* rtpvp8pay = gst_element_factory_make("rtpvp8pay", "rtppay");
    GstElement* webrtcbin = gst_element_factory_make("webrtcbin", "sendrecv"); // will handle signaling later

    if (!appsrc || !videoconvert || !vp8enc || !rtpvp8pay || !webrtcbin) {
        std::cerr << "Failed to create one or more GStreamer elements." << std::endl;
        return false;
    }

    // Create a pipeline and add elements.
    pipeline = gst_pipeline_new("webrtc-pipeline");
    gst_bin_add_many(GST_BIN(pipeline), appsrc, videoconvert, vp8enc, rtpvp8pay, webrtcbin, NULL);

    // Link appsrc -> videoconvert -> vp8enc -> rtpvp8pay.
    if (!gst_element_link_many(appsrc, videoconvert, vp8enc, rtpvp8pay, NULL)) {
        std::cerr << "Failed to link source through encoder before webrtcbin." << std::endl;
        return false;
    }

    // For webrtcbin, we must dynamically link its sink pad.
    GstPad *rtpSrcPad = gst_element_get_static_pad(rtpvp8pay, "src");
    GstPad *webrtcSinkPad = gst_element_request_pad_simple(webrtcbin, "sink_%u");
    if (gst_pad_link(rtpSrcPad, webrtcSinkPad) != GST_PAD_LINK_OK) {
        std::cerr << "Failed to link rtp payloader to webrtcbin." << std::endl;
        gst_object_unref(rtpSrcPad);
        gst_object_unref(webrtcSinkPad);
        return false;
    }
    gst_object_unref(rtpSrcPad);
    gst_object_unref(webrtcSinkPad);

    // Configure appsrc with the correct caps (raw RGB image)
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
        std::cerr << "Unable to set pipeline to playing state." << std::endl;
        return false;
    }

    timestamp = 0;
    return true;
}

bool GStreamerPipeline::pushFrame(unsigned char* data, int size) {
    if (!pipeline)
        return false;

    // Create a new GstBuffer with the provided frame data.
    GstBuffer* buffer = gst_buffer_new_allocate(NULL, size, NULL);
    gst_buffer_fill(buffer, 0, data, size);

    // Set the PTS and duration for 30fps playback.
    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30);
    timestamp += GST_BUFFER_DURATION(buffer);

    GstFlowReturn flow_ret;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &flow_ret);
    gst_buffer_unref(buffer);
    return (flow_ret == GST_FLOW_OK);
}

void GStreamerPipeline::shutdown() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
        appsrc = nullptr;
    }
}
