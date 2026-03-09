import * as wasm from './wasm'
import * as webgl2 from './webgl2'
import * as webgpu from './webgpu'
import {bswap} from './util'


export type SubModEventHandler = {
  onProgress(step: number, workerId?: number) : void
  onComplete(nonce: Uint32Array) : void
  onError(err: Error) : void
}

export enum CpuApi {
  WASM_SIMD,
  WASM,
  ASMJS,
}
export enum GpuApi {
  WEBGPU,
  WEBGL2,
  NONE,
}

export const events: {
  onComplete?: (nonce: Uint8Array) => void
  onError?: (err: Error) => void
} = {}

export const progress: number[] = []
export const features = {
  gpu: [] as boolean[],
  cpu: [false, false, true],
}

let isRunning: boolean
let gpuMod: typeof webgl2 | typeof webgpu | null


const eventHandler: SubModEventHandler = {
  onProgress(step, workerId) {
    if (typeof workerId !== 'undefined') {
      progress[workerId + 1] += step
    } else {
      progress[0] += step
    }
  },
  onComplete(nonce) {
    if (!isRunning) {
      return
    }
    stop()

    for (let i = 0; i < 4; i++) {
      nonce[i] = bswap(nonce[i])
    }
    const bytes = new Uint8Array(nonce.buffer)
    events.onComplete?.(bytes)
  },
  onError(err) {
    events.onError?.(err)
  },
}


async function initWasm() {
  if (!globalThis.WebAssembly) {
    return
  }
  features.cpu[CpuApi.WASM] = true

  const simdWasm = new Uint8Array([
    0,97,115,109,1,0,0,0,1,5,1,96,0,1,123,3,2,1,0,10,10,1,8,0,65,0,253,15,253,98,11
  ])
  if (WebAssembly.validate(simdWasm)) {
    features.cpu[CpuApi.WASM_SIMD] = true
  }
  wasm.init(eventHandler)
}

async function initGpu() {
  features.gpu[GpuApi.WEBGPU] = await webgpu.init(eventHandler)
  features.gpu[GpuApi.WEBGL2] = webgl2.init(eventHandler)
}

export async function init() {
  console.assert(progress.length === 0, 'already inited')

  progress.length = 1 + navigator.hardwareConcurrency
  progress.fill(0)

  await initGpu()
  initWasm()
}


export function start(challenge: Uint8Array, difficulty: number) {
  const words = new Uint32Array(challenge.buffer)
  const masks = new Uint32Array(2)

  for (let i = 0; i < 4; i++) {
    words[i] = bswap(words[i])
  }

  if (difficulty > 32) {
    masks[0] = -1
    masks[1] = -1 << (64 - difficulty)
  } else {
    masks[0] = -1 << (32 - difficulty)
  }

  progress.fill(0)
  isRunning = true

  wasm.start(words, masks)
  gpuMod?.start(words, masks)
}

export function stop() {
  wasm.stop()
  gpuMod?.stop()
  isRunning = false
}

export function setGpuApi(name: GpuApi) {
  switch (name) {
  case GpuApi.WEBGPU:
    gpuMod = webgpu
    break
  case GpuApi.WEBGL2:
    gpuMod = webgl2
    break
  default:
    gpuMod = null
    return
  }
}

export function setCpuApi(name: CpuApi) {
  wasm.setApi(name)
}

export function setGpuLoadRate(num: number) {
  gpuMod?.setLoadRate(num)
}

export function setCpuLoadRate(num: number) {
  wasm.setLoadRate(num)
}
