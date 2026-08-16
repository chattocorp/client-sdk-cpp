/*
 * Copyright 2026 LiveKit
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "livekit/encoded_video_source.h"

#include <limits>
#include <stdexcept>

#include "ffi.pb.h"
#include "ffi_client.h"
#include "video_frame.pb.h"
#include "video_utils.h"

namespace livekit {
namespace {

proto::VideoCodec toProto(EncodedVideoCodec codec) {
  switch (codec) {
    case EncodedVideoCodec::H264:
      return proto::VideoCodec::H264;
    case EncodedVideoCodec::H265:
      return proto::VideoCodec::H265;
    case EncodedVideoCodec::VP8:
      return proto::VideoCodec::VP8;
    case EncodedVideoCodec::VP9:
      return proto::VideoCodec::VP9;
    case EncodedVideoCodec::AV1:
      return proto::VideoCodec::AV1;
  }
  throw std::invalid_argument("EncodedVideoSource: unsupported codec");
}

proto::EncodedVideoFrameType toProto(EncodedVideoFrameType frame_type) {
  switch (frame_type) {
    case EncodedVideoFrameType::Key:
      return proto::EncodedVideoFrameType::ENCODED_VIDEO_FRAME_KEY;
    case EncodedVideoFrameType::Delta:
      return proto::EncodedVideoFrameType::ENCODED_VIDEO_FRAME_DELTA;
  }
  throw std::invalid_argument("EncodedVideoSource: unsupported frame type");
}

} // namespace

EncodedVideoSource::EncodedVideoSource(int width, int height) : width_(width), height_(height) {
  if (width <= 0 || height <= 0) {
    throw std::invalid_argument("EncodedVideoSource: dimensions must be positive");
  }

  proto::FfiRequest request;
  auto* message = request.mutable_new_video_source();
  message->set_type(proto::VideoSourceType::VIDEO_SOURCE_NATIVE_ENCODED);
  message->mutable_resolution()->set_width(static_cast<std::uint32_t>(width));
  message->mutable_resolution()->set_height(static_cast<std::uint32_t>(height));

  const proto::FfiResponse response = FfiClient::instance().sendRequest(request);
  if (!response.has_new_video_source()) {
    throw std::runtime_error("EncodedVideoSource: missing new_video_source");
  }
  handle_ = FfiHandle(response.new_video_source().source().handle().id());
}

bool EncodedVideoSource::captureFrame(const EncodedVideoFrame& frame) {
  if (!handle_) {
    return false;
  }
  if (frame.data == nullptr || frame.size == 0) {
    throw std::invalid_argument("EncodedVideoSource: payload must not be empty");
  }
  if (frame.size > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
    throw std::invalid_argument("EncodedVideoSource: payload is too large");
  }
  if (frame.width == 0 || frame.height == 0) {
    throw std::invalid_argument("EncodedVideoSource: dimensions must be positive");
  }

  proto::FfiRequest request;
  auto* message = request.mutable_capture_encoded_video_frame();
  message->set_source_handle(handle_.get());
  message->set_payload(frame.data, frame.size);
  message->set_codec(toProto(frame.codec));
  message->set_frame_type(toProto(frame.frame_type));
  message->set_timestamp_us(frame.timestamp_us);
  message->set_width(frame.width);
  message->set_height(frame.height);
  if (auto metadata = toProto(frame.metadata)) {
    message->mutable_metadata()->CopyFrom(*metadata);
  }

  const proto::FfiResponse response = FfiClient::instance().sendRequest(request);
  if (!response.has_capture_encoded_video_frame()) {
    throw std::runtime_error("EncodedVideoSource: missing capture_encoded_video_frame");
  }
  return response.capture_encoded_video_frame().accepted();
}

EncodedVideoSourceFeedback EncodedVideoSource::takeFeedback() {
  EncodedVideoSourceFeedback feedback;
  if (!handle_) {
    return feedback;
  }

  proto::FfiRequest request;
  request.mutable_get_encoded_video_source_feedback()->set_source_handle(handle_.get());
  const proto::FfiResponse response = FfiClient::instance().sendRequest(request);
  if (!response.has_get_encoded_video_source_feedback()) {
    throw std::runtime_error("EncodedVideoSource: missing get_encoded_video_source_feedback");
  }

  const auto& proto_feedback = response.get_encoded_video_source_feedback();
  feedback.keyframe_requested = proto_feedback.keyframe_requested();
  if (proto_feedback.has_rate_control()) {
    feedback.rate_control = EncodedVideoRateControl{
        proto_feedback.rate_control().target_bitrate_bps(), proto_feedback.rate_control().framerate_fps()};
  }
  return feedback;
}

} // namespace livekit
