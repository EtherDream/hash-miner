import {sleep} from './util'

export class LoadManager {
  _time = 0
  _busyCount = 0
  _expCallTime = 1000

  step = 0
  period = 1000

  setRate(v: number) {
    this._expCallTime = v * this.period
  }

  timeBegin() {
    this._time = performance.now()
  }

  timeEnd() {
    const t = performance.now() - this._time
    this._time = Math.max(t, 0.1)
  }

  idle() {
    // update step
    const stepPerMs = this.step / this._time
    // console.log(
    //   'step:', this.step,
    //   'time:', this._time,
    //   'stepPerMs:',stepPerMs,
    //   'expCallTime',this._expCallTime
    // )
    this.step = Math.ceil(stepPerMs * this._expCallTime)

    const remain = this.period - this._time
    if (remain > 0) {
      this._busyCount = 0
      return sleep(remain)
    }
    if (++this._busyCount > 5) {
      this._busyCount = 0
      return sleep(0)
    }
    // no sleep (sync)
  }
}