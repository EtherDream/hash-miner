export function sleep(ms: number) {
  return new Promise(fn => {
    setTimeout(fn, ms)
  })
}

export function randU32() {
  return Math.random() * 0xFFFFFFFF >>> 0
}

export function bswap(n: number) {
  return (
    ((n & 0x0000FF) << 24) |
    ((n & 0x00FF00) <<  8) |
    ((n & 0xFF0000) >>  8) |
    (n >>> 24)
  ) >>> 0
}

export function fillTmpl(
  tmpl: string,
  params: { [key: string] : number }
) {
  return tmpl.replace(/__(\w+)__/g, (_, $1) => {
    return params[$1] + ''
  })
}