#include <stdint.h>
#include "wasm-vec4.h"

using wasm_vec4::u32;
using wasm_vec4::vec4u;


#define EXPORT        extern "C" __attribute__((used))


extern "C" {
  void trace(int v);
  void debug();
}

/* Elementary functions used by SHA256 */
#define Ch(x, y, z) ((x & (y ^ z)) ^ z)
#define Maj(x, y, z)((x & (y | z)) | (y & z))
#define SHR(x, n)   (x >> n)
#define ROTR(x, n)  ((x >> n) | (x << (32 - n)))
#define S0(x)       (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x)       (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x)       (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define s1(x)       (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

/* SHA256 round function */
#define RND(a, b, c, d, e, f, g, h, k)  \
  t0 = h + S1(e) + Ch(e, f, g) + k;     \
  t1 = S0(a) + Maj(a, b, c);            \
  d += t0;                              \
  h  = t0 + t1;


constexpr u32 K[64] = {
  0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
  0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
  0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
  0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
  0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
  0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
  0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
  0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
  0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
  0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
  0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
  0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
  0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
  0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
  0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
  0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2,
};


inline int checkResult(u32 v0, u32 v1, u32 mask0, u32 mask1) {
  if ((v0 & mask0) == 0 && (v1 & mask1) == 0) {
    return 0;
  }
  return -1;
}

inline int checkResult(vec4u v0, vec4u v1, u32 mask0, u32 mask1) {
  if ((v0.x() & mask0) == 0 && (v1.x() & mask1) == 0) {
    return 0;
  }
  if ((v0.y() & mask0) == 0 && (v1.y() & mask1) == 0) {
    return 1;
  }
  if ((v0.z() & mask0) == 0 && (v1.z() & mask1) == 0) {
    return 2;
  }
  if ((v0.w() & mask0) == 0 && (v1.w() & mask1) == 0) {
    return 3;
  }
  return -1;
}


EXPORT u32 search(
    u32 w0, u32 w1, u32 w2, u32 w3,   // challenge
    u32 w4, u32 w5, u32 w6, u32 w7,   // nonce
    u32 mask0,      u32 mask1,        // difficulty
    u32 step
) {
#ifdef __wasm_simd128__
  using T = vec4u;
#else
  using T = u32;
#endif

  T W[16], S[8];
  T t0, t1;

  const u32 w7_end = w7 + step;

  do {
    S[0] = T(0x6A09E667);
    S[1] = T(0xBB67AE85);
    S[2] = T(0x3C6EF372);
    S[3] = T(0xA54FF53A);
    S[4] = T(0x510E527F);
    S[5] = T(0x9B05688C);
    S[6] = T(0x1F83D9AB);
    S[7] = T(0x5BE0CD19);

    W[0] = T(w0);
    W[1] = T(w1);
    W[2] = T(w2);
    W[3] = T(w3);
    W[4] = T(w4);
    W[5] = T(w5);
    W[6] = T(w6);

#ifdef __wasm_simd128__
    W[7] = vec4u(w7, w7 + 1, w7 + 2, w7 + 3);
#else
    W[7] = w7;
#endif

    // padding
    W[8] = T(0x80000000);
    W[9] = T(0);
    W[10] = T(0);
    W[11] = T(0);
    W[12] = T(0);
    W[13] = T(0);
    W[14] = T(0);

    // input bit len
    W[15] = T(256);

#if 1
    #pragma clang loop unroll(full)
    for (int t = 0; t < 64; t++) {
      int i = t & 15;

      if (t >= 16) {
        W[i] += W[(i + 9) & 15] + s1(W[(i + 14) & 15]) + s0(W[(i + 1) & 15]);
      }
      RND(
        S[(0 - t) & 7], S[(1 - t) & 7], S[(2 - t) & 7], S[(3 - t) & 7],
        S[(4 - t) & 7], S[(5 - t) & 7], S[(6 - t) & 7], S[(7 - t) & 7],
        W[i] + K[t]
      )
    }
#else
    // performance test
    #include "unroll.c"
#endif

    int offset = checkResult(
      S[0] + 0x6A09E667,
      S[1] + 0xBB67AE85,
      mask0, mask1
    );
    if (offset >= 0) {
      return w7 + offset;
    }
    w7 += sizeof(T) / 4;

  } while (w7 < w7_end);

  return 0;
}
