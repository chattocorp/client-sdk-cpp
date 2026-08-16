// Copyright 2026 LiveKit, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <livekit/livekit.h>

namespace {

constexpr int kWidth = 1920;
constexpr int kHeight = 1080;

void runScenario(livekit::EncodedVideoSource& source, std::size_t payload_size, std::size_t iterations) {
  std::vector<std::uint8_t> payload(payload_size, 0x55);
  // Give the synthetic payload an Annex-B NAL start code. This benchmark
  // measures ingress only and does not claim the remaining bytes are decodable.
  payload[0] = 0x00;
  payload[1] = 0x00;
  payload[2] = 0x00;
  payload[3] = 0x01;
  payload[4] = 0x65;

  livekit::EncodedVideoFrame frame;
  frame.data = payload.data();
  frame.size = payload.size();
  frame.codec = livekit::EncodedVideoCodec::H264;
  frame.frame_type = livekit::EncodedVideoFrameType::Key;
  frame.width = kWidth;
  frame.height = kHeight;

  std::size_t accepted = 0;
  const auto start = std::chrono::steady_clock::now();
  for (std::size_t index = 0; index < iterations; ++index) {
    accepted += source.captureFrame(frame) ? 1U : 0U;
  }
  const auto elapsed = std::chrono::steady_clock::now() - start;
  const double seconds = std::chrono::duration<double>(elapsed).count();
  const double frames_per_second = static_cast<double>(iterations) / seconds;
  const double mebibytes_per_second =
      static_cast<double>(iterations) * static_cast<double>(payload_size) / (1024.0 * 1024.0 * seconds);

  std::cout << payload_size << ',' << iterations << ',' << accepted << ',' << seconds << ',' << frames_per_second
            << ',' << mebibytes_per_second << '\n';
}

} // namespace

int main(int argc, char* argv[]) {
  try {
    const std::size_t iterations = argc > 1 ? static_cast<std::size_t>(std::stoull(argv[1])) : 1000;
    if (iterations == 0) {
      throw std::invalid_argument("iterations must be positive");
    }

    (void)livekit::initialize(livekit::LogLevel::Warn);
    livekit::EncodedVideoSource source(kWidth, kHeight);

    std::cout << "payload_bytes,iterations,accepted,seconds,frames_per_second,mebibytes_per_second\n";
    for (const std::size_t payload_size : {64U * 1024U, 256U * 1024U, 1024U * 1024U}) {
      runScenario(source, payload_size, iterations);
    }

    livekit::shutdown();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Encoded video ingress benchmark failed: " << error.what() << '\n';
    livekit::shutdown();
    return 1;
  }
}
