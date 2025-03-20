// Include additional headers for timestamp generation.
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <gst/gst.h>
#include <gst/base/gstbytereader.h>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>
#include <nlohmann/json.hpp>

#include "pipeline.h"
#include "streaming/signaling_client.h"

using json = nlohmann::json;

// Helper function to return the current UTC time in ISO 8601 format.
static std::string get_current_timestamp() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_time =
      std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  // Format using GMT (UTC)
  oss << std::put_time(std::gmtime(&now_time),
                       "[%Y-%m-%dT%H:%M:%SZ]");
  return oss.str();
}

// Helper callback for promise fulfillment to extract the SDP offer.
static void on_offer_created(GstPromise* promise,
                             gpointer user_data) {
  const GstStructure* reply = gst_promise_get_reply(promise);
  GstWebRTCSessionDescription* offer = nullptr;

  // Extract the offer from the reply structure.
  gst_structure_get(reply, "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION,
                    &offer, NULL);
  gst_promise_unref(promise);

  if (!offer) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to create offer"
              << std::endl;
    return;
  }

  // Convert the SDP to string.
  gchar* sdp_text = gst_sdp_message_as_text(offer->sdp);
  if (!sdp_text) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to convert SDP to text"
              << std::endl;
    gst_webrtc_session_description_free(offer);
    return;
  }

  // Use nlohmann::json to build the offer message.
  json offerJson;
  offerJson["type"] = "offer";
  offerJson["sdp"] = std::string(sdp_text);

  // Serialize JSON to a string.
  std::string offerMsg = offerJson.dump();

  std::cout << get_current_timestamp()
            << " [GStreamerPipeline] Sending SDP offer:\n"
            << offerMsg << std::endl;

  // Cleanup.
  g_free(sdp_text);
  gst_webrtc_session_description_free(offer);

  // Get the global WebSocketClient singleton instance and send the offer.
  WebSocketClient& wsClient = WebSocketClient::getInstance();
  wsClient.sendMessage(offerMsg);
}

// Callback: Triggered when negotiation is needed (create SDP offer).
static void on_negotiation_needed(GstElement* webrtc,
                                  gpointer user_data) {
  std::cout << get_current_timestamp()
            << " [GStreamerPipeline] on_negotiation_needed triggered."
            << std::endl;

  // Create a promise that will be fulfilled when the offer is created.
  GstPromise* promise = gst_promise_new_with_change_func(
      on_offer_created, webrtc, NULL);

  // Request the webrtcbin element to create an SDP offer.
  g_signal_emit_by_name(webrtc, "create-offer", NULL, promise);
}

// Callback: Called when an ICE candidate is generated.
static void on_ice_candidate(GstElement* webrtc, guint mlineindex,
                             gchar* candidate,
                             gpointer user_data) {
  if (!candidate)
    return;  // Ensure candidate is valid.

  std::cout << get_current_timestamp()
            << " [GStreamerPipeline] on-ice-candidate triggered, "
               "mlineindex: "
            << mlineindex << " candidate: " << candidate << std::endl;

  // Build the candidate message using nlohmann::json.
  json candidateJson;
  candidateJson["type"] = "ice-candidate";
  candidateJson["sdpMLineIndex"] = mlineindex;
  candidateJson["candidate"] = std::string(candidate);

  // Serialize JSON to a string.
  std::string candidateMsg = candidateJson.dump();

  // Get the WebSocket client singleton and send the ICE candidate.
  WebSocketClient& wsClient = WebSocketClient::getInstance();
  wsClient.sendMessage(candidateMsg);
}

bool GStreamerPipeline::initialize(int width, int height, int fps) {
  if (initialized) {
    return true; // Already initialized.
  }

  // Create a new pipeline.
  pipeline = gst_pipeline_new("webrtc-pipeline");
  if (!pipeline) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to create pipeline." << std::endl;
    return false;
  }

  // Create elements.
  // Use videotestsrc instead of appsrc for testing.
  GstElement* source = gst_element_factory_make("videotestsrc", "source");
  GstElement* videoconvert = gst_element_factory_make("videoconvert", "convert");
  GstElement* vp8enc = gst_element_factory_make("vp8enc", "encoder");
  GstElement* rtpvp8pay = gst_element_factory_make("rtpvp8pay", "payloader");
  GstElement* capsfilter = gst_element_factory_make("capsfilter", "rtpcaps");
  GstElement* webrtcbin = gst_element_factory_make("webrtcbin", "webrtc");

  if (!source || !videoconvert || !vp8enc || !rtpvp8pay || !capsfilter || !webrtcbin) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to create elements." << std::endl;
    return false;
  }

  // Configure videotestsrc.
  g_object_set(source, "is-live", TRUE, "pattern", 0, NULL);

  // Configure the VP8 encoder.
  g_object_set(vp8enc, "deadline", 1, "target-bitrate", 500000, NULL);

  // Configure RTP payloader.
  g_object_set(rtpvp8pay, "pt", 96, NULL);

  // Configure caps filter.
  GstCaps* rtpCaps = gst_caps_from_string(
      "application/x-rtp,media=video,encoding-name=VP8,payload=96");
  g_object_set(capsfilter, "caps", rtpCaps, NULL);
  gst_caps_unref(rtpCaps);

  // Configure WebRTC.
  g_object_set(webrtcbin,
               "bundle-policy", GST_WEBRTC_BUNDLE_POLICY_MAX_BUNDLE,
               "stun-server", "stun://stun.l.google.com:19302",
               NULL);

  // Add all elements to the pipeline.
  gst_bin_add_many(GST_BIN(pipeline), source, videoconvert, vp8enc,
                   rtpvp8pay, capsfilter, webrtcbin, NULL);

  // Link the source elements up to the capsfilter.
  if (!gst_element_link_many(source, videoconvert, vp8enc, rtpvp8pay, capsfilter, NULL)) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to link pipeline elements." << std::endl;
    return false;
  }

  // *** Attach a pad probe to capsfilter's src pad ***
  GstPad* srcPad = gst_element_get_static_pad(capsfilter, "src");
  if (!srcPad) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to get source pad from capsfilter." << std::endl;
    return false;
  }
  gst_pad_add_probe(srcPad, GST_PAD_PROBE_TYPE_BUFFER,
      [](GstPad* pad, GstPadProbeInfo* info, gpointer user_data) -> GstPadProbeReturn {
        GstBuffer* buffer = GST_PAD_PROBE_INFO_BUFFER(info);
        if (buffer) {
          g_print("Buffer on pad %s, size: %u\n", GST_PAD_NAME(pad), gst_buffer_get_size(buffer));
        }
        return GST_PAD_PROBE_OK;
      },
      NULL, NULL);
  // You can unref srcPad after adding the probe.
  gst_object_unref(srcPad);

  // Request a sink pad from webrtcbin.
  GstPad* sinkPad = gst_element_request_pad_simple(webrtcbin, "sink_%u");
  if (!sinkPad) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to get request pad from webrtcbin." << std::endl;
    return false;
  }

  // Link capsfilter's output to webrtcbin's sink pad.
  GstPadLinkReturn linkRet = gst_pad_link(srcPad, sinkPad);
  if (GST_PAD_LINK_FAILED(linkRet)) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to link capsfilter to webrtcbin: "
              << linkRet << std::endl;
    gst_object_unref(sinkPad);
    return false;
  }
  gst_object_unref(sinkPad);

  // Connect signals for negotiation and ICE candidates.
  g_signal_connect(webrtcbin, "on-negotiation-needed",
                   G_CALLBACK(on_negotiation_needed), this);
  g_signal_connect(webrtcbin, "on-ice-candidate",
                   G_CALLBACK(on_ice_candidate), this);

  // Set the pipeline state to PLAYING.
  GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Unable to set pipeline to PLAYING state." << std::endl;
    return false;
  }

  timestamp = 0;
  initialized = true;

  std::cout << get_current_timestamp()
            << " [GStreamerPipeline] Pipeline initialized successfully with videotestsrc." << std::endl;
  return true;
}

bool GStreamerPipeline::pushFrame(unsigned char* data, int size) {
  if (!initialized) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Pipeline not initialized."
              << std::endl;
    return false;
  }

  GstBuffer* buffer = gst_buffer_new_allocate(NULL, size, NULL);
  if (!buffer) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to allocate GstBuffer."
              << std::endl;
    return false;
  }
  gst_buffer_fill(buffer, 0, data, size);

  GST_BUFFER_PTS(buffer) = timestamp;
  GST_BUFFER_DURATION(buffer) =
      gst_util_uint64_scale_int(1, GST_SECOND, 30);
  timestamp += GST_BUFFER_DURATION(buffer);

  GstFlowReturn flowRet;
  g_signal_emit_by_name(appsrc, "push-buffer", buffer, &flowRet);
  gst_buffer_unref(buffer);

  std::cout << get_current_timestamp()
            << " [GStreamerPipeline] Pushing frame at timestamp: "
            << timestamp << std::endl;
  if (flowRet != GST_FLOW_OK) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] push-buffer returned error: "
              << flowRet << std::endl;
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
    std::cout << get_current_timestamp()
              << " [GStreamerPipeline] Pipeline shut down."
              << std::endl;
  }
}
