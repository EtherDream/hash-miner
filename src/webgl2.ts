import FRAG_SHADER from './assets/webgl2.glsl'
import {WebGL2 as GL} from './webapi-const'
import {randU32, sleep, fillTmpl} from './util'
import {LoadManager} from './load-manager'
import type {SubModEventHandler} from './index'


const MAGIC_CODE = 0x19260817
const texW = 1024
const texH = 1024
const thread = texW * texH

let events: SubModEventHandler
let isRunning: boolean

const loadmgr = new LoadManager()
loadmgr.period = 200


function createWebGl() {
  const canvas = new OffscreenCanvas(texW, texH)
  const ctx = canvas.getContext('webgl2', {
    powerPreference: 'high-performance',
  })
  return {canvas, ctx}
}

export function init(handler: SubModEventHandler) {
  events = handler
  const {ctx} = createWebGl()
  return !!ctx
}

export async function start(words: Uint32Array, masks: Uint32Array) {
  try {
    await startImpl(words, masks)
  } catch (err: any) {
    events.onError(err)
  }
  isRunning = false
}

async function startImpl(words: Uint32Array, masks: Uint32Array) {
  const {canvas, ctx: gl} = createWebGl()
  if (!gl) {
    throw Error('webgl error')
  }
  loadmgr.step = 128
  isRunning = true

  canvas.addEventListener('webglcontextlost', () => {
    isRunning = false
    events.onError(Error('webgl contextlost'))
  })

  // setTimeout(() => {
  //   console.log('mock contextlost')
  //   gl.getExtension("WEBGL_lose_context")!.loseContext()
  // }, 3000)

  const vertexData = new Float32Array([
    -1, +1, // left top
    -1, -1, // left bottom
    +1, +1, // right top
    +1, -1, // right bottom
  ])
  gl.bindBuffer(GL.ARRAY_BUFFER, gl.createBuffer())
  gl.bufferData(GL.ARRAY_BUFFER, vertexData, GL.STATIC_DRAW)

  const program = gl.createProgram()!
  const VERTEX_SHADER = `\
#version 300 es
in vec2 v_pos;
void main() {
gl_Position = vec4(v_pos, 0., 1.);
}`
  const vertexShader = createShader(gl, VERTEX_SHADER, GL.VERTEX_SHADER)

  const w4 = randU32()

  const code = fillTmpl(FRAG_SHADER, {
    'TEX_W': texW,
    'W0': words[0],
    'W1': words[1],
    'W2': words[2],
    'W3': words[3],
    'W4': w4,
    'MASK0': masks[0],
    'MASK1': masks[1],
    'MAGIC_CODE': MAGIC_CODE,
  })
  // console.log(code)
  const fragShader = createShader(gl, code, GL.FRAGMENT_SHADER)

  // const dbg = gl.getExtension('WEBGL_debug_shaders')
  // if (dbg) {
  //   const s = dbg.getTranslatedShaderSource(fragShader)
  //   console.log(s)
  // } else {
  //   console.warn('WEBGL_debug_shaders not available')
  // }

  gl.attachShader(program, fragShader)
  gl.attachShader(program, vertexShader)

  await sleep(0)
  gl.linkProgram(program)
  gl.useProgram(program)

  const posHandle = 0
  gl.vertexAttribPointer(posHandle, 2 /*vec2*/, GL.FLOAT, false, 0, 0)
  gl.enableVertexAttribArray(posHandle)

  const fbo = gl.createFramebuffer()!
  const tex = gl.createTexture()!

  gl.bindTexture(GL.TEXTURE_2D, tex)
  gl.texStorage2D(GL.TEXTURE_2D, 1, GL.RGBA32UI, texW, texH)
  gl.bindFramebuffer(GL.FRAMEBUFFER, fbo)
  gl.framebufferTexture2D(GL.FRAMEBUFFER, GL.COLOR_ATTACHMENT0, GL.TEXTURE_2D, tex, 0)

  // gl.disable(GL.DEPTH_TEST)
  // gl.disable(GL.STENCIL_TEST)
  // gl.depthMask(false)
  // gl.disable(GL.CULL_FACE)
  // gl.disable(GL.BLEND)

  const inStep = gl.getUniformLocation(program, 'in_step')!
  const inW5 = gl.getUniformLocation(program, 'in_w5')!
  const query = gl.createQuery()

  LOOP:
  for (let w5 = 0; w5 < 0xFFFFFFFF; w5++) {
    gl.beginQuery(GL.ANY_SAMPLES_PASSED, query)
    gl.uniform1ui(inW5, w5)
    gl.uniform1ui(inStep, loadmgr.step)
    gl.drawArrays(GL.TRIANGLE_STRIP, 0, 4)
    gl.endQuery(GL.ANY_SAMPLES_PASSED)

    loadmgr.timeBegin()
    for (;;) {
      await sleep(5)
      if (!isRunning) {
        break LOOP
      }
      const done = gl.getQueryParameter(query, GL.QUERY_RESULT_AVAILABLE)
      if (done) {
        break
      }
    }
    loadmgr.timeEnd()

    const found = gl.getQueryParameter(query, GL.QUERY_RESULT)
    if (found) {
      const arr = new Uint32Array(thread * 4)
      gl.readPixels(0, 0, texW, texH, GL.RGBA_INTEGER, GL.UNSIGNED_INT, arr)

      const index = arr.indexOf(MAGIC_CODE)
      console.assert(index >= 0, 'webgl error')

      const w6 = arr[index + 1]
      const w7 = arr[index + 2]

      const nonce = Uint32Array.of(w4, w5, w6, w7)
      events.onComplete(nonce)
      break
    }

    events.onProgress(thread * loadmgr.step)
    await loadmgr.idle()
  }
}

export function stop() {
  isRunning = false
}

export function setLoadRate(rate: number) {
  loadmgr.setRate(rate)
}


function createShader(gl: WebGL2RenderingContext, code: string, type: number) {
  const shader = gl.createShader(type)
  if (!shader) {
    throw Error('createShader failed')
  }
  gl.shaderSource(shader, code)
  gl.compileShader(shader)

  if (!gl.getShaderParameter(shader, GL.COMPILE_STATUS)) {
    const msg = gl.getShaderInfoLog(shader)!
    throw Error(msg)
  }
  return shader
}