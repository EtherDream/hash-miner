import WASM_SIMD_B64 from './assets/sha256-simd.wasm'
import WASM_B64 from './assets/sha256.wasm'
import ASMJS_CODE from './assets/asmjs.js'
import {LoadManager} from './load-manager'
import {randU32} from './util'
import type {SubModEventHandler} from './index'

type u32 = number

type WasmExport = {
  search(
    w0: u32, w1: u32, w2: u32, w3: u32,
    w4: u32, w5: u32, w6: u32, w7: u32,
    mask0: u32,
    mask1: u32,
    step: u32
  ) : u32
}

//
// page to worker message
//
const enum ReqMsgType {
  LOAD_RATE,
  SET_API,
  START,
  STOP,
}
type ReqMsg = {
  type: ReqMsgType.SET_API
  file: BufferSource
  id: number
} | {
  type: ReqMsgType.START
  words: Uint32Array
  masks: Uint32Array  
} | {
  type: ReqMsgType.LOAD_RATE
  rate: number
} | {
  type: ReqMsgType.STOP
}

//
// worker to page message
//
const enum ResMsgType {
  PROGRESS,
  COMPLETE,
}
type ResMsg = {
  type: ResMsgType.PROGRESS
  step: number
} | {
  type: ResMsgType.COMPLETE
  nonce: Uint32Array
}

const WASM_FILES: BufferSource[] = []


function workerEnv() {
  const loadmgr = new LoadManager()

  const wasmObjs: WasmExport[] = []
  let isRunning: boolean
  let wasmObj: WasmExport

  const ffi = {
    env: {
      trace: console.log,
      debug: () => { debugger },
    }
  }

  function loadWasm(id: number, wasm: BufferSource) {
    if (!wasmObjs[id]) {
      if (wasm) {
        const module = new WebAssembly.Module(wasm)
        const instance = new WebAssembly.Instance(module, ffi)
        wasmObjs[id] = instance.exports as WasmExport
      } else {
        const code = 'return (' + ASMJS_CODE + ')()'
        wasmObjs[id] = Function(code)()
      }
    }
    wasmObj = wasmObjs[id]
  }

  async function run(words: Uint32Array, masks: Uint32Array) {
    isRunning = true
    loadmgr.step = 1024 * 1024

    // challenge
    const w0 = words[0]
    const w1 = words[1]
    const w2 = words[2]
    const w3 = words[3]

    // nonce
    const w4 = randU32()
    const w5 = randU32()
    let w6 = 0
    let w7 = 1

    do {
      loadmgr.timeBegin()
      const w7hit = wasmObj.search(
        w0, w1, w2, w3,       // challenge
        w4, w5, w6, w7,       // nonce
        masks[0], masks[1],   // difficulty
        loadmgr.step
      )
      loadmgr.timeEnd()

      const msg: ResMsg = {
        type: ResMsgType.PROGRESS,
        step: w7hit ? (w7hit - w7) : loadmgr.step,
      }
      postMessage(msg)

      if (w7hit) {
        const msg: ResMsg = {
          type: ResMsgType.COMPLETE,
          nonce: Uint32Array.of(w4, w5, w6, w7hit),
        }
        postMessage(msg)
        break
      }
      await loadmgr.idle()

      if ((w7 += loadmgr.step) > 0xFFFFFFFF) {
        w7 = 1
        w6++
      }
    } while (isRunning)
  }

  self.onmessage = (e) => {
    const $: ReqMsg = e.data

    switch ($.type) {
    case ReqMsgType.LOAD_RATE:
      loadmgr.setRate($.rate)
      break
    case ReqMsgType.SET_API:
      loadWasm($.id, $.file)
      break
    case ReqMsgType.START:
      run($.words, $.masks)
      break
    case ReqMsgType.STOP:
      isRunning = false
      break
    }
  }
}
if (!globalThis.document) {
  workerEnv()
}


declare global {
  interface Worker {
    id: number
  }
}
const workers: Worker[] = []

let events: SubModEventHandler


export function start(words: Uint32Array, masks: Uint32Array) {
  sendMsgToWorker({
    type: ReqMsgType.START,
    words,
    masks,
  })
}

export function init(handler: SubModEventHandler) {
  events = handler

  // @ts-ignore
  WASM_FILES[0] = Uint8Array.fromBase64(WASM_SIMD_B64)
  // @ts-ignore
  WASM_FILES[1] = Uint8Array.fromBase64(WASM_B64)

  for (let i = 0; i < navigator.hardwareConcurrency; i++) {
    const url = import.meta.url
    const worker = new Worker(url, {
      type: 'module'
    })
    worker.onmessage = onWorkerMsg
    worker.id = i
    workers[i] = worker
  }
}

export function stop() {
  sendMsgToWorker({
    type: ReqMsgType.STOP,
  })
}

export function setApi(id: number) {
  sendMsgToWorker({
    type: ReqMsgType.SET_API,
    file: WASM_FILES[id] || '', // wasm or asmjs
    id,
  })
}

export function setLoadRate(rate: number) {
  sendMsgToWorker({
    type: ReqMsgType.LOAD_RATE,
    rate,
  })
}

function onWorkerMsg(this: Worker, e: MessageEvent) {
  const $: ResMsg = e.data

  switch ($.type) {
    case ResMsgType.PROGRESS:
    events.onProgress!($.step, this.id)
    break
  case ResMsgType.COMPLETE:
    events.onComplete!($.nonce)
    break
  }
}

function sendMsgToWorker(msg: ReqMsg) {
  for (const worker of workers) {
    worker.postMessage(msg)
  }
}
