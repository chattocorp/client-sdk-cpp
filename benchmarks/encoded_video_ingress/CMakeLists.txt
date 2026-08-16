# Copyright 2026 LiveKit, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0

cmake_minimum_required(VERSION 3.20)
project(EncodedVideoIngressBenchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LiveKit CONFIG REQUIRED)

add_executable(EncodedVideoIngressBenchmark main.cpp)
target_link_libraries(EncodedVideoIngressBenchmark PRIVATE LiveKit::livekit)
