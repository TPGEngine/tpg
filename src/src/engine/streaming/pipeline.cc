#include "pipeline.h"
#include "streaming/signaling_client.h"  
#include <iostream>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>

// Helper callback for promise fulfillment to extract the SDP offer.
static void on_offer_created(GstPromise* promise, gpointer user_data) {
    GstElement* webrtc = GST_ELEMENT(user_data);
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
    
    // user_data is the GStreamerPipeline instance
    GStreamerPipeline* pipeline = static_cast<GStreamerPipeline*>(user_data);
    
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

    // Create a simpler pipeline without linking to webrtcbin
    char pipelineStr[1024];
    snprintf(pipelineStr, sizeof(pipelineStr),
        "appsrc name=src format=time is-live=true do-timestamp=true ! "
        "video/x-raw,format=RGB,width=%d,height=%d,framerate=%d/1 ! "
        "videoconvert ! "
        "vp8enc deadline=1 target-bitrate=500000 cpu-used=4 ! "
        "rtpvp8pay pt=96 ! "
        "application/x-rtp,media=video,encoding-name=VP8,payload=96 ! "
        "queue name=vidqueue",
        width, height, fps);

    std::cout << "[GStreamerPipeline] Creating pipeline: " << pipelineStr << std::endl;
    
    // Create the pipeline from the string description
    GError* error = NULL;
    pipeline = gst_parse_launch(pipelineStr, &error);
    
    if (error) {
        std::cerr << "[GStreamerPipeline] Failed to create pipeline: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }

    // Get references to named elements
    appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "src");
    GstElement* queue = gst_bin_get_by_name(GST_BIN(pipeline), "vidqueue");
    
    if (!appsrc || !queue) {
        std::cerr << "[GStreamerPipeline] Failed to get pipeline elements" << std::endl;
        return false;
    }

    // Create and add webrtcbin
    GstElement* webrtcbin = gst_element_factory_make("webrtcbin", "webrtc");
    if (!webrtcbin) {
        std::cerr << "[GStreamerPipeline] Failed to create webrtcbin element" << std::endl;
        return false;
    }
    
    // Configure webrtcbin
    g_object_set(webrtcbin, 
                "bundle-policy", GST_WEBRTC_BUNDLE_POLICY_MAX_BUNDLE,
                "stun-server", "stun://stun.l.google.com:19302",
                NULL);
    
    // Add webrtcbin to pipeline
    gst_bin_add(GST_BIN(pipeline), webrtcbin);
    
    // CRITICAL: Get source pad from queue
    GstPad* srcPad = gst_element_get_static_pad(queue, "src");
    if (!srcPad) {
        std::cerr << "[GStreamerPipeline] Failed to get src pad from queue" << std::endl;
        return false;
    }
    
    // CRITICAL: Get request pad from webrtcbin
    GstPad* sinkPad = gst_element_request_pad_simple(webrtcbin, "sink_%u");
    if (!sinkPad) {
        std::cerr << "[GStreamerPipeline] Failed to get request pad from webrtcbin" << std::endl;
        gst_object_unref(srcPad);
        return false;
    }
    
    // Print pad names for debugging
    gchar* srcPadName = gst_pad_get_name(srcPad);
    gchar* sinkPadName = gst_pad_get_name(sinkPad);
    std::cout << "[GStreamerPipeline] Linking " << srcPadName << " to " << sinkPadName << std::endl;
    g_free(srcPadName);
    g_free(sinkPadName);
    
    // Link pads manually
    GstPadLinkReturn ret = gst_pad_link(srcPad, sinkPad);
    if (GST_PAD_LINK_FAILED(ret)) {
        std::cerr << "[GStreamerPipeline] Failed to link queue to webrtcbin, error code: " << ret << std::endl;
        
        // Get pad caps for debugging
        GstCaps* srcCaps = gst_pad_get_current_caps(srcPad);
        if (srcCaps) {
            gchar* capsStr = gst_caps_to_string(srcCaps);
            std::cout << "[GStreamerPipeline] Source pad caps: " << capsStr << std::endl;
            g_free(capsStr);
            gst_caps_unref(srcCaps);
        }
        
        gst_object_unref(srcPad);
        gst_object_unref(sinkPad);
        return false;
    }
    
    // Connect signals
    g_signal_connect(webrtcbin, "on-negotiation-needed", 
                    G_CALLBACK(on_negotiation_needed), webrtcbin);
    g_signal_connect(webrtcbin, "on-ice-candidate", 
                    G_CALLBACK(on_ice_candidate), webrtcbin);
    
    // Clean up references
    gst_object_unref(srcPad);
    gst_object_unref(sinkPad);
    gst_object_unref(queue);
    
    // Set the pipeline to PLAYING state
    GstStateChangeReturn stateRet = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (stateRet == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "[GStreamerPipeline] Unable to set pipeline to PLAYING state" << std::endl;
        return false;
    }

    // Reset timestamp and mark as initialized
    timestamp = 0;
    initialized = true;

    std::cout << "[GStreamerPipeline] Pipeline initialized successfully" << std::endl;
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
