# Encoded Video Ingress Benchmark

Measures the synchronous C++/protobuf/Rust FFI cost of submitting pre-encoded
video access units. It reuses one payload buffer and reports accepted frames,
frames per second, and payload throughput for representative access-unit sizes.

This is deliberately an ingress microbenchmark. It does not publish a track,
encode video, packetize RTP, or measure network throughput.

## Building

Install the SDK, then configure this standalone benchmark against it:

```bash
./build.sh release --bundle --prefix "$PWD/local-install"
cmake -S benchmarks/encoded_video_ingress \
  -B benchmarks/encoded_video_ingress/build \
  -DCMAKE_PREFIX_PATH="$PWD/local-install"
cmake --build benchmarks/encoded_video_ingress/build --config Release
```

On Windows, use `build.cmd release`, install the build with CMake, and ensure
the installed SDK DLL directory is on `PATH` when running the executable.

The optional first argument selects the number of submissions per payload size
(default: 1000).
