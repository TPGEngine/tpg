// Include additional headers for timestamp generation.
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
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
static void on_offer_created(GstPromise* promise, gpointer user_data) {
  GstElement* webrtc = (GstElement*)user_data;
  const GstStructure* reply = gst_promise_get_reply(promise);
  GstWebRTCSessionDescription* offer = nullptr;

  // Extract the offer from the reply structure.
  gst_structure_get(reply, "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION,
                    &offer, NULL);
  gst_promise_unref(promise);

  if (!offer) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to create offer" << std::endl;
    return;
  }

  // **The key missing step: set the local description before sending the offer.**
  g_signal_emit_by_name(webrtc, "set-local-description", offer, NULL);

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
  candidateJson["sdpMid"] = "video0";

  // Serialize JSON to a string.
  std::string candidateMsg = candidateJson.dump();

  // Get the WebSocket client singleton and send the ICE candidate.
  WebSocketClient& wsClient = WebSocketClient::getInstance();
  wsClient.sendMessage(candidateMsg);
}

bool GStreamerPipeline::initialize(int width, int height, int fps) {
  if (initialized) {
    return true;  // Already initialized.
  }

  // Initialize GStreamer pipeline
  pipeline = gst_pipeline_new("webrtc-pipeline");
  if (!pipeline) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to create pipeline."
              << std::endl;
    return false;
  }

  // Create elements
  appsrc = gst_element_factory_make("appsrc", "src");
  GstElement* videoconvert =
      gst_element_factory_make("videoconvert", "convert");
  GstElement* vp8enc = gst_element_factory_make("vp8enc", "encoder");
  GstElement* rtpvp8pay =
      gst_element_factory_make("rtpvp8pay", "payloader");
  GstElement* capsfilter =
      gst_element_factory_make("capsfilter", "rtpcaps");
  GstElement* webrtcbin =
      gst_element_factory_make("webrtcbin", "webrtc");

  // Store webrtcbin element for later use in signaling
  webrtcbinElement = webrtcbin;

  // Check all elements were created
  if (!appsrc || !videoconvert || !vp8enc || !rtpvp8pay ||
      !capsfilter || !webrtcbin) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to create elements."
              << std::endl;
    return false;
  }

  // Configure appsrc
  GstCaps* srcCaps = gst_caps_new_simple(
      "video/x-raw", "format", G_TYPE_STRING, "RGB", "width",
      G_TYPE_INT, width, "height", G_TYPE_INT, height, "framerate",
      GST_TYPE_FRACTION, fps, 1, NULL);
  g_object_set(appsrc, "caps", srcCaps, "format", GST_FORMAT_TIME,
               "is-live", TRUE, "do-timestamp", TRUE, NULL);
  gst_caps_unref(srcCaps);

  // Configure encoder
  g_object_set(vp8enc, "deadline", 1,             // Fastest encoding
               "target-bitrate", 500000, NULL);

  // Configure RTP payloader
  g_object_set(rtpvp8pay, "pt", 96, NULL);  // Set payload type to 96

  // Configure caps filter
  GstCaps* rtpCaps = gst_caps_from_string(
      "application/x-rtp,media=video,encoding-name=VP8,payload=96");
  g_object_set(capsfilter, "caps", rtpCaps, NULL);
  gst_caps_unref(rtpCaps);

  // Configure WebRTC
  g_object_set(webrtcbin, "bundle-policy",
               GST_WEBRTC_BUNDLE_POLICY_MAX_BUNDLE, "stun-server",
               "stun://stun.l.google.com:19302", NULL);

  // Add all elements to the pipeline
  gst_bin_add_many(GST_BIN(pipeline), appsrc, videoconvert, vp8enc,
                   rtpvp8pay, capsfilter, webrtcbin, NULL);

  // Link the pipeline up to capsfilter
  if (!gst_element_link_many(appsrc, videoconvert, vp8enc, rtpvp8pay,
                             capsfilter, NULL)) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to link pipeline elements."
              << std::endl;
    return false;
  }

  // Get source pad from capsfilter
  GstPad* srcPad = gst_element_get_static_pad(capsfilter, "src");
  if (!srcPad) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to get source pad from "
                 "capsfilter."
              << std::endl;
    return false;
  }

  // Get sink pad from webrtcbin (use the specific pad template)
  GstPad* sinkPad = gst_element_request_pad_simple(webrtcbin, "sink_%u");
  if (!sinkPad) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to get request pad from "
                 "webrtcbin."
              << std::endl;
    gst_object_unref(srcPad);
    return false;
  }

  // Link the pads
  GstPadLinkReturn linkRet = gst_pad_link(srcPad, sinkPad);
  if (GST_PAD_LINK_FAILED(linkRet)) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Failed to link capsfilter to webrtcbin: "
              << linkRet << std::endl;
    gst_object_unref(srcPad);
    gst_object_unref(sinkPad);
    return false;
  }

  // Add probe *after* linking, but *before* unreffing
  if (sinkPad) {
    gst_pad_add_probe(sinkPad, GST_PAD_PROBE_TYPE_BUFFER,
                      [](GstPad* pad, GstPadProbeInfo* info,
                         gpointer user_data) -> GstPadProbeReturn {
                        GstBuffer* buffer = GST_PAD_PROBE_INFO_BUFFER(info);
                        if (buffer) {
                          g_print("RTP buffer received on pad %s\n",
                                  GST_PAD_NAME(pad));
                          // Optionally, you could inspect buffer timestamps,
                          // sizes, etc.
                        }
                        return GST_PAD_PROBE_OK;
                      },
                      NULL, NULL);
  }

  // Release the pads
  gst_object_unref(srcPad);
  gst_object_unref(sinkPad);

  // Connect signals
  g_signal_connect(webrtcbin, "on-negotiation-needed",
                   G_CALLBACK(on_negotiation_needed), this);
  g_signal_connect(webrtcbin, "on-ice-candidate",
                   G_CALLBACK(on_ice_candidate), this);

  // Set the pipeline to PLAYING state
  GstStateChangeReturn ret =
      gst_element_set_state(pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    std::cerr << get_current_timestamp()
              << " [GStreamerPipeline] Unable to set pipeline to PLAYING "
                 "state."
              << std::endl;
    return false;
  }

  // Reset timestamp and mark as initialized
  timestamp = 0;
  initialized = true;

  std::cout << get_current_timestamp()
            << " [GStreamerPipeline] Pipeline initialized successfully."
            << std::endl;
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

void GStreamerPipeline::handleSignalingMessage(const std::string& message) {
    try {
        // Parse the incoming message as JSON
        json jsonMsg = json::parse(message);

        // Check if the message has a type field
        if (!jsonMsg.contains("type")) {
            std::cerr << get_current_timestamp() 
                     << " [GStreamerPipeline] Received message without 'type' field." << std::endl;
            return;
        }

        std::string msgType = jsonMsg["type"];

        if (msgType == "answer") {
            std::cout << get_current_timestamp() 
                     << " [GStreamerPipeline] Processing SDP Answer..." << std::endl;

            // Check for SDP field
            if (!jsonMsg.contains("sdp") || !jsonMsg["sdp"].is_string()) {
                std::cerr << get_current_timestamp() 
                         << " [GStreamerPipeline] Answer message missing valid SDP field." << std::endl;
                return;
            }

            // Extract SDP string
            std::string sdp_text = jsonMsg["sdp"].get<std::string>();

            // Create new SDP message
            GstSDPMessage* sdp_msg = nullptr;
            GstSDPResult sdp_result = gst_sdp_message_new(&sdp_msg);
            if (sdp_result != GST_SDP_OK) {
                std::cerr << get_current_timestamp() 
                         << " [GStreamerPipeline] Failed to create SDP message structure." << std::endl;
                return;
            }

            // Parse SDP text into message
            sdp_result = gst_sdp_message_parse_buffer(
                (const guint8*)sdp_text.c_str(), 
                sdp_text.length(), 
                sdp_msg
            );
            if (sdp_result != GST_SDP_OK) {
                std::cerr << get_current_timestamp() 
                         << " [GStreamerPipeline] Failed to parse SDP answer text." << std::endl;
                gst_sdp_message_free(sdp_msg);
                return;
            }

            // Create WebRTC session description for the answer
            GstWebRTCSessionDescription* answer_desc = 
                gst_webrtc_session_description_new(
                    GST_WEBRTC_SDP_TYPE_ANSWER, 
                    sdp_msg
                );
            if (!answer_desc) {
                std::cerr << get_current_timestamp() 
                         << " [GStreamerPipeline] Failed to create WebRTC session description for answer." << std::endl;
                gst_sdp_message_free(sdp_msg);
                return;
            }

            // Note on thread safety:
            // If the WebSocket callback is executing on a different thread than the GMainLoop,
            // these GStreamer calls might need to be marshalled to the main GStreamer thread
            // using g_main_context_invoke or similar for thread safety.

            // Set the remote description
            if (webrtcbinElement) {
                std::cout << get_current_timestamp() 
                         << " [GStreamerPipeline] Setting remote description (Answer)." << std::endl;
                GstPromise* promise = gst_promise_new();
                g_signal_emit_by_name(webrtcbinElement, "set-remote-description", answer_desc, promise);
                gst_promise_unref(promise);
            } else {
                std::cerr << get_current_timestamp() 
                         << " [GStreamerPipeline] Error: webrtcbin element is NULL when trying to set remote description." << std::endl;
            }

            // Cleanup
            gst_webrtc_session_description_free(answer_desc);
        }
        else if (msgType == "ice-candidate") {
            std::cout << get_current_timestamp() 
                     << " [GStreamerPipeline] Received ICE Candidate:\n" 
                     << jsonMsg.dump(2) << std::endl;
            
            // Check for required fields with correct nesting
            if (!jsonMsg.contains("candidate") || !jsonMsg["candidate"].is_object()) {
                std::cerr << get_current_timestamp() 
                         << " [GStreamerPipeline] Invalid or missing 'candidate' object in ICE message." << std::endl;
                return;
            }

            const json& candidateObj = jsonMsg["candidate"];

            // Extract the candidate string from the nested structure
            if (!candidateObj.contains("candidate") || !candidateObj["candidate"].is_string()) {
                std::cerr << get_current_timestamp() 
                         << " [GStreamerPipeline] Invalid or missing 'candidate' string in ICE candidate object." << std::endl;
                return;
            }
            std::string candidate_str = candidateObj["candidate"].get<std::string>();

            // Extract the mlineindex from the nested structure
            if (!candidateObj.contains("sdpMLineIndex") || !candidateObj["sdpMLineIndex"].is_number_integer()) {
                std::cerr << get_current_timestamp() 
                         << " [GStreamerPipeline] Invalid or missing 'sdpMLineIndex' field in ICE candidate object." << std::endl;
                return;
            }
            guint mlineindex = candidateObj["sdpMLineIndex"].get<guint>();

            // Log the candidate details for debugging
            std::cout << get_current_timestamp() 
                     << " [GStreamerPipeline] Processing ICE Candidate:"
                     << " mlineindex=" << mlineindex
                     << ", candidate=" << candidate_str << std::endl;

            // Note on thread safety:
            // If the WebSocket callback is executing on a different thread than the GMainLoop,
            // these GStreamer calls might need to be marshalled to the main GStreamer thread
            // using g_main_context_invoke or similar for thread safety.

            // Add the ICE candidate to webrtcbin
            if (webrtcbinElement) {
                std::cout << get_current_timestamp() 
                         << " [GStreamerPipeline] Adding ICE candidate to webrtcbin..." << std::endl;
                g_signal_emit_by_name(webrtcbinElement, "add-ice-candidate", mlineindex, candidate_str.c_str());
            } else {
                std::cerr << get_current_timestamp() 
                         << " [GStreamerPipeline] Error: webrtcbin element is NULL when trying to add ICE candidate." << std::endl;
            }
        }
        else {
            std::cout << get_current_timestamp() 
                     << " [GStreamerPipeline] Received unknown message type: " 
                     << msgType << std::endl;
        }
    }
    catch (const json::parse_error& e) {
        std::cerr << get_current_timestamp() 
                 << " [GStreamerPipeline] Failed to parse signaling message as JSON: " 
                 << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << get_current_timestamp() 
                 << " [GStreamerPipeline] Error handling signaling message: " 
                 << e.what() << std::endl;
    }
}
