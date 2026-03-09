import COMPUTE_SHADER from './assets/webgpu.wgsl'
import {GPUBufferUsage, GPUMapMode} from './webapi-const'
import {randU32, fillTmpl} from './util'
import {LoadManager} from './load-manager'
import type {SubModEventHandler} from './index'


let events: SubModEventHandler
let isRunning: boolean

const loadmgr = new LoadManager()


async function createDevice() {
  const adapter = await navigator.gpu.requestAdapter({
    powerPreference: 'high-performance',
  })
  if (adapter) {
    return await adapter.requestDevice()
  }
}

export async function init(handler: SubModEventHandler) {
  events = handler

  if (!navigator.gpu) {
    return false
  }
  return !!await createDevice()
}

export async function start(words: Uint32Array, masks: Uint32Array) {
  try {
    await startImpl(words, masks)
  } catch (err: any) {
    events.onError(err)
  }
}

async function startImpl(words: Uint32Array, masks: Uint32Array) {
  const gpu = await createDevice()
  if (!gpu) {
    throw Error('device error')
  }

  gpu.addEventListener('uncapturederror', e => {
    if (!isRunning) {
      return
    }
    isRunning = false
    events.onError(e.error as Error)
  })

  gpu.lost.then(info => {
    if (!isRunning) {
      return
    }
    isRunning = false
    events.onError(Error(info.message))
  })

  isRunning = true
  loadmgr.step = 512

  const WORKGROUP_SIZE = 64
  const WORKGROUP_NUM = 2048

  const w4 = randU32()

  const code = fillTmpl(COMPUTE_SHADER, {
    'W0': words[0],
    'W1': words[1],
    'W2': words[2],
    'W3': words[3],
    'W4': w4,
    'MASK0': masks[0],
    'MASK1': masks[1],
    'WORKGROUP_SIZE': WORKGROUP_SIZE,
  })
  const module = gpu.createShaderModule({code})
  // await module.getCompilationInfo()

  const pipeline = gpu.createComputePipeline({
    layout: 'auto',
    compute: {
      module,
    }
  })
  const inputGpuBuf = gpu.createBuffer({
    size: 16,
    usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
  })
  const outputGpuBuf = gpu.createBuffer({
    size: 8,
    usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC,
  })
  const bindGroup = gpu.createBindGroup({
    layout: pipeline.getBindGroupLayout(0),
    entries: [
      { binding: 0, resource: { buffer: inputGpuBuf } },
      { binding: 1, resource: { buffer: outputGpuBuf } },
    ],
  })

  const readbackGpuBuf = gpu.createBuffer({
    size: 8,
    usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.MAP_READ,
  })

  const enum ID {
    STEP,
    W5,
  }
  const paramU32 = new Uint32Array(4)

  for (let w5 = 0; w5 < 0xFFFFFFFF; w5++) {
    paramU32[ID.STEP] = loadmgr.step
    paramU32[ID.W5] = w5
    gpu.queue.writeBuffer(inputGpuBuf, 0, paramU32)

    const encoder = gpu.createCommandEncoder()
    const pass = encoder.beginComputePass()
    pass.setPipeline(pipeline)
    pass.setBindGroup(0, bindGroup)
    pass.dispatchWorkgroups(WORKGROUP_NUM)
    pass.end()

    encoder.copyBufferToBuffer(outputGpuBuf, 0, readbackGpuBuf, 0, 8)
    gpu.queue.submit([encoder.finish()])

    loadmgr.timeBegin()
    await readbackGpuBuf.mapAsync(GPUMapMode.READ)
    loadmgr.timeEnd()

    if (!isRunning) {
      break
    }
    events.onProgress(WORKGROUP_SIZE * WORKGROUP_NUM * loadmgr.step)

    const result = new Uint32Array(readbackGpuBuf.getMappedRange())
    if (result[0]) {
      const [w6, w7] = result
      const nonce = Uint32Array.of(w4, w5, w6, w7)
      events.onComplete(nonce)
      isRunning = false
      break
    }
    readbackGpuBuf.unmap()

    await loadmgr.idle()
  }

  isRunning = false
  gpu.destroy()
}

export function stop() {
  isRunning = false
}

export function setLoadRate(rate: number) {
  loadmgr.setRate(rate)
}
