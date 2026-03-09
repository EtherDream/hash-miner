# Hash Miner

A browser-based SHA-256 Proof-of-Work (PoW) demo implemented with WebGPU and WebAssembly SIMD.

## Demo

https://etherdream.github.io/hash-miner/

## Usage

`Challenge` is the puzzle input. It can be specified manually or generated randomly. For simplicity, its length is fixed at 16 bytes.

`Nonce` is the solution. Click `Start` to begin searching for a nonce such that the first `Difficulty` bits of *SHA256(Challenge || Nonce)* are all zero. The UI displays values in hexadecimal.

In `Config`, you can configure the GPU and CPU utilization. Since the GPU is typically an order of magnitude faster than the CPU, running the CPU at 100% usually doesn’t help and can make the system sluggish.

In `Debug`, you can choose legacy APIs. The GPU backend supports `WebGL2`, and the CPU backend supports non-SIMD Wasm as well as asm.js.

## History

In 2017, I wrote a WebGL2-based demo ([link](https://www.etherdream.com/funnyscript/glminer/glminer.html)). That program was put together in a rush, so it had a few issues:

- An unreasonable canvas size prevented the GPU from being fully utilized.

- The SHA-256 message schedule `W` only needs 16 elements, not 64.

- Readbacks on every frame are unnecessary; instead, use an ANY_SAMPLES_PASSED query to count discards, and only read back pixels once a match is found.

This version fixes those issues, allowing the WebGL2 backend to reach WebGPU-level performance. (On macOS the performance is similar, but on Windows WebGPU is much faster than WebGL2.)

## Project Layout

`src/assets` contains the Wasm binaries and shader files.

- [`webgl2.glsl`](src/assets/webgl2.glsl)

- [`webgpu.wgsl`](src/assets/webgpu.wgsl)

The Wasm binaries are not checked into the repository. Generate them with:

```bash
./build-wasm.sh
```

Since Emscripten no longer supports asm.js, this script currently cannot generate the asm.js build automatically. You need to extract it manually from `asmjs-tmp.js`.

The core modules are written in TypeScript and live in the `src` directory. Run `npm run dev` or `npm run build` to generate `index.js` in the `dist` directory.

## Known Issues

On Chrome, non-SIMD Wasm performance is very poor — sometimes even slower than asm.js.
