#include "pipeline.h"
#include "streaming/signaling_client.h"  
#include <iostream>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>

// Helper callback for promise fulfillment to extract the SDP offer.
static void on_offer_created(GstPromise* promise, gpointer user_data) {
    const GstStructure* reply = gst_promise_get_reply(promise);
    GstWebRTCSessionDescription* offer = nullptr;

    // Extract the offer from the reply structure.
    gst_structure_get(reply, "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &offer, NULL);
    gst_promise_unref(promise);

    if (!offer) {
        std::cerr << "[GStreamerPipeline] Failed to create offer" << std::endl;
        return;
    }

    // Convert the SDP to string.
    gchar* sdp_text = gst_sdp_message_as_text(offer->sdp);
    if (!sdp_text) {
        std::cerr << "[GStreamerPipeline] Failed to convert SDP to text" << std::endl;
        gst_webrtc_session_description_free(offer);
        return;
    }

    // Create a JSON message with the offer SDP.
    std::string offerMsg = std::string("{\"type\":\"offer\",\"sdp\":\"") +
                           sdp_text + "\"}";
    g_free(sdp_text);
    gst_webrtc_session_description_free(offer);

    std::cout << "[GStreamerPipeline] Sending SDP offer:\n" << offerMsg << std::endl;
    
    // Get the global WebSocketClient singleton instance and send the offer.
    WebSocketClient &wsClient = WebSocketClient::getInstance();
    wsClient.sendMessage(offerMsg);
}

// Callback: Called when negotiation is needed.
static void on_negotiation_needed(GstElement* webrtc, gpointer user_data) {
    std::cout << "[GStreamerPipeline] on_negotiation_needed triggered." << std::endl;
    
    // Create a promise that will be fulfilled when the offer is created
    GstPromise* promise = gst_promise_new_with_change_func(on_offer_created, webrtc, NULL);
    
    // Request the webrtcbin element to create an SDP offer
    g_signal_emit_by_name(webrtc, "create-offer", NULL, promise);
}


// Callback: Called when an ICE candidate is generated.
static void on_ice_candidate(GstElement* webrtc, guint mlineindex, gchar* candidate, gpointer user_data) {
    std::cout << "[GStreamerPipeline] on-ice-candidate triggered, mlineindex: "
              << mlineindex << " candidate: " << candidate << std::endl;
    
    // Create a JSON message with the candidate information.
    std::string candidateMsg = "{\"type\":\"ice\",\"mlineindex\":" +
                               std::to_string(mlineindex) +
                               ",\"candidate\":\"" + candidate + "\"}";
    
    // Get the WebSocket client singleton and send the ICE candidate.
    WebSocketClient &wsClient = WebSocketClient::getInstance();
    wsClient.sendMessage(candidateMsg);
}

bool GStreamerPipeline::initialize(int width, int height, int fps) {
    if (initialized) {
        return true; // Already initialized.
    }

    // Initialize GStreamer pipeline
    pipeline = gst_pipeline_new("webrtc-pipeline");
    if (!pipeline) {
        std::cerr << "[GStreamerPipeline] Failed to create pipeline." << std::endl;
        return false;
    }

    // Create elements
    appsrc = gst_element_factory_make("appsrc", "src");
    GstElement* videoconvert = gst_element_factory_make("videoconvert", "convert");
    GstElement* vp8enc = gst_element_factory_make("vp8enc", "encoder");
    GstElement* rtpvp8pay = gst_element_factory_make("rtpvp8pay", "payloader");
    GstElement* capsfilter = gst_element_factory_make("capsfilter", "rtpcaps");
    GstElement* webrtcbin = gst_element_factory_make("webrtcbin", "webrtc");

    // Check all elements were created
    if (!appsrc || !videoconvert || !vp8enc || !rtpvp8pay || !capsfilter || !webrtcbin) {
        std::cerr << "[GStreamerPipeline] Failed to create elements." << std::endl;
        return false;
    }

    // Configure appsrc
    GstCaps* srcCaps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "RGB",
        "width", G_TYPE_INT, width,
        "height", G_TYPE_INT, height,
        "framerate", GST_TYPE_FRACTION, fps, 1,
        NULL);
    g_object_set(appsrc, 
                "caps", srcCaps, 
                "format", GST_FORMAT_TIME,
                "is-live", TRUE,
                "do-timestamp", TRUE,
                NULL);
    gst_caps_unref(srcCaps);

    // Configure encoder
    g_object_set(vp8enc, 
                "deadline", 1,       // Fastest encoding
                "target-bitrate", 500000,
                NULL);

    // Configure RTP payloader
    g_object_set(rtpvp8pay, "pt", 96, NULL);  // Set payload type to 96

    // Configure caps filter
    GstCaps* rtpCaps = gst_caps_from_string("application/x-rtp,media=video,encoding-name=VP8,payload=96");
    g_object_set(capsfilter, "caps", rtpCaps, NULL);
    gst_caps_unref(rtpCaps);

    // Configure WebRTC
    g_object_set(webrtcbin, 
                "bundle-policy", GST_WEBRTC_BUNDLE_POLICY_MAX_BUNDLE,
                "stun-server", "stun://stun.l.google.com:19302",
                NULL);

    // Add all elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), 
                    appsrc, 
                    videoconvert, 
                    vp8enc, 
                    rtpvp8pay, 
                    capsfilter,
                    webrtcbin, 
                    NULL);

    // Link the pipeline up to capsfilter
    if (!gst_element_link_many(appsrc, videoconvert, vp8enc, rtpvp8pay, capsfilter, NULL)) {
        std::cerr << "[GStreamerPipeline] Failed to link pipeline elements." << std::endl;
        return false;
    }

    // Get source pad from capsfilter
    GstPad* srcPad = gst_element_get_static_pad(capsfilter, "src");
    if (!srcPad) {
        std::cerr << "[GStreamerPipeline] Failed to get source pad from capsfilter." << std::endl;
        return false;
    }

    // Get sink pad from webrtcbin (use the specific pad template)
    GstPad* sinkPad = gst_element_request_pad_simple(webrtcbin, "sink_%u");
    if (!sinkPad) {
        std::cerr << "[GStreamerPipeline] Failed to get request pad from webrtcbin." << std::endl;
        gst_object_unref(srcPad);
        return false;
    }

    // Link the pads
    GstPadLinkReturn linkRet = gst_pad_link(srcPad, sinkPad);
    if (GST_PAD_LINK_FAILED(linkRet)) {
        std::cerr << "[GStreamerPipeline] Failed to link capsfilter to webrtcbin: " << linkRet << std::endl;
        gst_object_unref(srcPad);
        gst_object_unref(sinkPad);
        return false;
    }

    // Release the pads
    gst_object_unref(srcPad);
    gst_object_unref(sinkPad);

    // Connect signals
    g_signal_connect(webrtcbin, "on-negotiation-needed", G_CALLBACK(on_negotiation_needed), this);
    g_signal_connect(webrtcbin, "on-ice-candidate", G_CALLBACK(on_ice_candidate), this);
    
    // Set the pipeline to PLAYING state
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "[GStreamerPipeline] Unable to set pipeline to PLAYING state." << std::endl;
        return false;
    }

    // Reset timestamp and mark as initialized
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

    GstBuffer* buffer = gst_buffer_new_allocate(NULL, size, NULL);
    if (!buffer) {
        std::cerr << "[GStreamerPipeline] Failed to allocate GstBuffer." << std::endl;
        return false;
    }
    gst_buffer_fill(buffer, 0, data, size);

    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30);
    timestamp += GST_BUFFER_DURATION(buffer);

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
