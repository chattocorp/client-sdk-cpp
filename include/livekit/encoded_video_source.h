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

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "livekit/ffi_handle.h"
#include "livekit/video_source.h"
#include "livekit/visibility.h"

namespace livekit {

/// @brief Codec carried by a pre-encoded video access unit.
enum class EncodedVideoCodec { H264, H265, VP8, VP9, AV1 };

/// @brief Frame type of a pre-encoded video access unit.
enum class EncodedVideoFrameType { Key, Delta };

/// @brief One pre-encoded video access unit ready for publishing.
///
/// The payload is copied during the synchronous captureFrame() call and only
/// needs to remain valid until that call returns.
struct EncodedVideoFrame {
  const std::uint8_t* data = nullptr;
  std::size_t size = 0;
  EncodedVideoCodec codec = EncodedVideoCodec::H264;
  EncodedVideoFrameType frame_type = EncodedVideoFrameType::Delta;
  std::int64_t timestamp_us = 0;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::optional<VideoFrameMetadata> metadata;
};

/// @brief Encoder rate-control target requested by WebRTC.
struct EncodedVideoRateControl {
  std::uint64_t target_bitrate_bps = 0;
  double framerate_fps = 0.0;
};

/// @brief Pending feedback for an application-owned video encoder.
struct EncodedVideoSourceFeedback {
  bool keyframe_requested = false;
  std::optional<EncodedVideoRateControl> rate_control;
};

/// @brief Video source for access units encoded by the application.
///
/// Publish tracks created from this source with
/// VideoEncoderBackend::PreEncoded. captureFrame() and takeFeedback() are not
/// safe to call concurrently on the same source.
class LIVEKIT_API EncodedVideoSource {
public:
  /// @brief Creates a pre-encoded source with a fixed initial resolution.
  /// @param width Initial encoded width in pixels.
  /// @param height Initial encoded height in pixels.
  /// @throws std::runtime_error If the FFI source cannot be created.
  EncodedVideoSource(int width, int height);
  ~EncodedVideoSource() = default;

  EncodedVideoSource(const EncodedVideoSource&) = delete;
  EncodedVideoSource& operator=(const EncodedVideoSource&) = delete;
  EncodedVideoSource(EncodedVideoSource&&) noexcept = default;
  EncodedVideoSource& operator=(EncodedVideoSource&&) noexcept = default;

  /// @brief Returns the initial source width in pixels.
  int width() const noexcept { return width_; }

  /// @brief Returns the initial source height in pixels.
  int height() const noexcept { return height_; }

  /// @brief Returns the underlying FFI handle ID, or zero when invalid.
  std::uint64_t ffiHandleId() const noexcept { return handle_.get(); }

  /// @brief Submits one complete encoded access unit.
  /// @param frame Encoded access unit and its metadata.
  /// @return True when the passthrough encoder accepted the access unit.
  /// @throws std::invalid_argument If the payload pointer or dimensions are invalid.
  /// @throws std::runtime_error If the FFI response is invalid.
  [[nodiscard]] bool captureFrame(const EncodedVideoFrame& frame);

  /// @brief Returns and clears pending WebRTC encoder feedback.
  /// @return Pending keyframe and rate-control requests.
  /// @throws std::runtime_error If the FFI response is invalid.
  [[nodiscard]] EncodedVideoSourceFeedback takeFeedback();

private:
  FfiHandle handle_;
  int width_{0};
  int height_{0};
};

} // namespace livekit
