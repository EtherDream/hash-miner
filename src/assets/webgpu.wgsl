struct Params {
  step: u32,
  w5: u32,
  _rev1: u32,
  _rev2: u32,
};

@group(0) @binding(0)
var<uniform> input: Params;

@group(0) @binding(1)
var<storage, read_write> output : array<u32, 2>;


const K = array<u32, 64>(
  0x428A2F98u, 0x71374491u, 0xB5C0FBCFu, 0xE9B5DBA5u, 0x3956C25Bu, 0x59F111F1u, 0x923F82A4u, 0xAB1C5ED5u,
  0xD807AA98u, 0x12835B01u, 0x243185BEu, 0x550C7DC3u, 0x72BE5D74u, 0x80DEB1FEu, 0x9BDC06A7u, 0xC19BF174u,
  0xE49B69C1u, 0xEFBE4786u, 0x0FC19DC6u, 0x240CA1CCu, 0x2DE92C6Fu, 0x4A7484AAu, 0x5CB0A9DCu, 0x76F988DAu,
  0x983E5152u, 0xA831C66Du, 0xB00327C8u, 0xBF597FC7u, 0xC6E00BF3u, 0xD5A79147u, 0x06CA6351u, 0x14292967u,
  0x27B70A85u, 0x2E1B2138u, 0x4D2C6DFCu, 0x53380D13u, 0x650A7354u, 0x766A0ABBu, 0x81C2C92Eu, 0x92722C85u,
  0xA2BFE8A1u, 0xA81A664Bu, 0xC24B8B70u, 0xC76C51A3u, 0xD192E819u, 0xD6990624u, 0xF40E3585u, 0x106AA070u,
  0x19A4C116u, 0x1E376C08u, 0x2748774Cu, 0x34B0BCB5u, 0x391C0CB3u, 0x4ED8AA4Au, 0x5B9CCA4Fu, 0x682E6FF3u,
  0x748F82EEu, 0x78A5636Fu, 0x84C87814u, 0x8CC70208u, 0x90BEFFFAu, 0xA4506CEBu, 0xBEF9A3F7u, 0xC67178F2u
);

// inline
fn rotr(x: u32, n: u32) -> u32 {
  return (x >> n) | (x << (32 - n));
}
fn Sigma0(x: u32) -> u32 {
  return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}
fn Sigma1(x: u32) -> u32 {
  return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}
fn sigma0(x: u32) -> u32 {
  return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}
fn sigma1(x: u32) -> u32 {
  return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}
fn Ch(x: u32, y: u32, z: u32) -> u32 {
  return (x & y) ^ (~x & z);
}
fn Maj(x: u32, y: u32, z: u32) -> u32 {
  return (x & y) ^ (x & z) ^ (y & z);
}


@compute @workgroup_size(__WORKGROUP_SIZE__)
fn main(
  @builtin(global_invocation_id) gid : vec3<u32>
) {
  let thread_id = gid.x;

  var W: array<u32, 16>;
  var S: array<u32, 8>;
  var tmp: u32;

  for (var i = input.step; i > 0; i--) {
    S[0] = 0x6A09E667;
    S[1] = 0xBB67AE85;
    S[2] = 0x3C6EF372;
    S[3] = 0xA54FF53A;
    S[4] = 0x510E527F;
    S[5] = 0x9B05688C;
    S[6] = 0x1F83D9AB;
    S[7] = 0x5BE0CD19;

    // challenge
    W[0] = __W0__;
    W[1] = __W1__;
    W[2] = __W2__;
    W[3] = __W3__;

    // nonce
    W[4] = __W4__;
    W[5] = input.w5;
    W[6] = i;
    W[7] = thread_id;

    // padding
    W[8] = 0x80000000;  // 1 << 31
    W[9] = 0;
    W[10] = 0;
    W[11] = 0;
    W[12] = 0;
    W[13] = 0;
    W[14] = 0;
    W[15] = 256;        // input bit len

    // 0..15
    tmp = S[7] + Sigma1(S[4]) + Ch(S[4], S[5], S[6]) + W[ 0] + K[ 0]; S[3] += tmp; S[7] = tmp + Sigma0(S[0]) + Maj(S[0], S[1], S[2]);
    tmp = S[6] + Sigma1(S[3]) + Ch(S[3], S[4], S[5]) + W[ 1] + K[ 1]; S[2] += tmp; S[6] = tmp + Sigma0(S[7]) + Maj(S[7], S[0], S[1]);
    tmp = S[5] + Sigma1(S[2]) + Ch(S[2], S[3], S[4]) + W[ 2] + K[ 2]; S[1] += tmp; S[5] = tmp + Sigma0(S[6]) + Maj(S[6], S[7], S[0]);
    tmp = S[4] + Sigma1(S[1]) + Ch(S[1], S[2], S[3]) + W[ 3] + K[ 3]; S[0] += tmp; S[4] = tmp + Sigma0(S[5]) + Maj(S[5], S[6], S[7]);
    tmp = S[3] + Sigma1(S[0]) + Ch(S[0], S[1], S[2]) + W[ 4] + K[ 4]; S[7] += tmp; S[3] = tmp + Sigma0(S[4]) + Maj(S[4], S[5], S[6]);
    tmp = S[2] + Sigma1(S[7]) + Ch(S[7], S[0], S[1]) + W[ 5] + K[ 5]; S[6] += tmp; S[2] = tmp + Sigma0(S[3]) + Maj(S[3], S[4], S[5]);
    tmp = S[1] + Sigma1(S[6]) + Ch(S[6], S[7], S[0]) + W[ 6] + K[ 6]; S[5] += tmp; S[1] = tmp + Sigma0(S[2]) + Maj(S[2], S[3], S[4]);
    tmp = S[0] + Sigma1(S[5]) + Ch(S[5], S[6], S[7]) + W[ 7] + K[ 7]; S[4] += tmp; S[0] = tmp + Sigma0(S[1]) + Maj(S[1], S[2], S[3]);
    tmp = S[7] + Sigma1(S[4]) + Ch(S[4], S[5], S[6]) + W[ 8] + K[ 8]; S[3] += tmp; S[7] = tmp + Sigma0(S[0]) + Maj(S[0], S[1], S[2]);
    tmp = S[6] + Sigma1(S[3]) + Ch(S[3], S[4], S[5]) + W[ 9] + K[ 9]; S[2] += tmp; S[6] = tmp + Sigma0(S[7]) + Maj(S[7], S[0], S[1]);
    tmp = S[5] + Sigma1(S[2]) + Ch(S[2], S[3], S[4]) + W[10] + K[10]; S[1] += tmp; S[5] = tmp + Sigma0(S[6]) + Maj(S[6], S[7], S[0]);
    tmp = S[4] + Sigma1(S[1]) + Ch(S[1], S[2], S[3]) + W[11] + K[11]; S[0] += tmp; S[4] = tmp + Sigma0(S[5]) + Maj(S[5], S[6], S[7]);
    tmp = S[3] + Sigma1(S[0]) + Ch(S[0], S[1], S[2]) + W[12] + K[12]; S[7] += tmp; S[3] = tmp + Sigma0(S[4]) + Maj(S[4], S[5], S[6]);
    tmp = S[2] + Sigma1(S[7]) + Ch(S[7], S[0], S[1]) + W[13] + K[13]; S[6] += tmp; S[2] = tmp + Sigma0(S[3]) + Maj(S[3], S[4], S[5]);
    tmp = S[1] + Sigma1(S[6]) + Ch(S[6], S[7], S[0]) + W[14] + K[14]; S[5] += tmp; S[1] = tmp + Sigma0(S[2]) + Maj(S[2], S[3], S[4]);
    tmp = S[0] + Sigma1(S[5]) + Ch(S[5], S[6], S[7]) + W[15] + K[15]; S[4] += tmp; S[0] = tmp + Sigma0(S[1]) + Maj(S[1], S[2], S[3]);

    // message schedule 1
    W[ 0] += sigma1(W[14]) + W[ 9] + sigma0(W[ 1]);
    W[ 1] += sigma1(W[15]) + W[10] + sigma0(W[ 2]);
    W[ 2] += sigma1(W[ 0]) + W[11] + sigma0(W[ 3]);
    W[ 3] += sigma1(W[ 1]) + W[12] + sigma0(W[ 4]);
    W[ 4] += sigma1(W[ 2]) + W[13] + sigma0(W[ 5]);
    W[ 5] += sigma1(W[ 3]) + W[14] + sigma0(W[ 6]);
    W[ 6] += sigma1(W[ 4]) + W[15] + sigma0(W[ 7]);
    W[ 7] += sigma1(W[ 5]) + W[ 0] + sigma0(W[ 8]);
    W[ 8] += sigma1(W[ 6]) + W[ 1] + sigma0(W[ 9]);
    W[ 9] += sigma1(W[ 7]) + W[ 2] + sigma0(W[10]);
    W[10] += sigma1(W[ 8]) + W[ 3] + sigma0(W[11]);
    W[11] += sigma1(W[ 9]) + W[ 4] + sigma0(W[12]);
    W[12] += sigma1(W[10]) + W[ 5] + sigma0(W[13]);
    W[13] += sigma1(W[11]) + W[ 6] + sigma0(W[14]);
    W[14] += sigma1(W[12]) + W[ 7] + sigma0(W[15]);
    W[15] += sigma1(W[13]) + W[ 8] + sigma0(W[ 0]);

    // 16..31
    tmp = S[7] + Sigma1(S[4]) + Ch(S[4], S[5], S[6]) + W[ 0] + K[16]; S[3] += tmp; S[7] = tmp + Sigma0(S[0]) + Maj(S[0], S[1], S[2]);
    tmp = S[6] + Sigma1(S[3]) + Ch(S[3], S[4], S[5]) + W[ 1] + K[17]; S[2] += tmp; S[6] = tmp + Sigma0(S[7]) + Maj(S[7], S[0], S[1]);
    tmp = S[5] + Sigma1(S[2]) + Ch(S[2], S[3], S[4]) + W[ 2] + K[18]; S[1] += tmp; S[5] = tmp + Sigma0(S[6]) + Maj(S[6], S[7], S[0]);
    tmp = S[4] + Sigma1(S[1]) + Ch(S[1], S[2], S[3]) + W[ 3] + K[19]; S[0] += tmp; S[4] = tmp + Sigma0(S[5]) + Maj(S[5], S[6], S[7]);
    tmp = S[3] + Sigma1(S[0]) + Ch(S[0], S[1], S[2]) + W[ 4] + K[20]; S[7] += tmp; S[3] = tmp + Sigma0(S[4]) + Maj(S[4], S[5], S[6]);
    tmp = S[2] + Sigma1(S[7]) + Ch(S[7], S[0], S[1]) + W[ 5] + K[21]; S[6] += tmp; S[2] = tmp + Sigma0(S[3]) + Maj(S[3], S[4], S[5]);
    tmp = S[1] + Sigma1(S[6]) + Ch(S[6], S[7], S[0]) + W[ 6] + K[22]; S[5] += tmp; S[1] = tmp + Sigma0(S[2]) + Maj(S[2], S[3], S[4]);
    tmp = S[0] + Sigma1(S[5]) + Ch(S[5], S[6], S[7]) + W[ 7] + K[23]; S[4] += tmp; S[0] = tmp + Sigma0(S[1]) + Maj(S[1], S[2], S[3]);
    tmp = S[7] + Sigma1(S[4]) + Ch(S[4], S[5], S[6]) + W[ 8] + K[24]; S[3] += tmp; S[7] = tmp + Sigma0(S[0]) + Maj(S[0], S[1], S[2]);
    tmp = S[6] + Sigma1(S[3]) + Ch(S[3], S[4], S[5]) + W[ 9] + K[25]; S[2] += tmp; S[6] = tmp + Sigma0(S[7]) + Maj(S[7], S[0], S[1]);
    tmp = S[5] + Sigma1(S[2]) + Ch(S[2], S[3], S[4]) + W[10] + K[26]; S[1] += tmp; S[5] = tmp + Sigma0(S[6]) + Maj(S[6], S[7], S[0]);
    tmp = S[4] + Sigma1(S[1]) + Ch(S[1], S[2], S[3]) + W[11] + K[27]; S[0] += tmp; S[4] = tmp + Sigma0(S[5]) + Maj(S[5], S[6], S[7]);
    tmp = S[3] + Sigma1(S[0]) + Ch(S[0], S[1], S[2]) + W[12] + K[28]; S[7] += tmp; S[3] = tmp + Sigma0(S[4]) + Maj(S[4], S[5], S[6]);
    tmp = S[2] + Sigma1(S[7]) + Ch(S[7], S[0], S[1]) + W[13] + K[29]; S[6] += tmp; S[2] = tmp + Sigma0(S[3]) + Maj(S[3], S[4], S[5]);
    tmp = S[1] + Sigma1(S[6]) + Ch(S[6], S[7], S[0]) + W[14] + K[30]; S[5] += tmp; S[1] = tmp + Sigma0(S[2]) + Maj(S[2], S[3], S[4]);
    tmp = S[0] + Sigma1(S[5]) + Ch(S[5], S[6], S[7]) + W[15] + K[31]; S[4] += tmp; S[0] = tmp + Sigma0(S[1]) + Maj(S[1], S[2], S[3]);

    // message schedule 2
    W[ 0] += sigma1(W[14]) + W[ 9] + sigma0(W[ 1]);
    W[ 1] += sigma1(W[15]) + W[10] + sigma0(W[ 2]);
    W[ 2] += sigma1(W[ 0]) + W[11] + sigma0(W[ 3]);
    W[ 3] += sigma1(W[ 1]) + W[12] + sigma0(W[ 4]);
    W[ 4] += sigma1(W[ 2]) + W[13] + sigma0(W[ 5]);
    W[ 5] += sigma1(W[ 3]) + W[14] + sigma0(W[ 6]);
    W[ 6] += sigma1(W[ 4]) + W[15] + sigma0(W[ 7]);
    W[ 7] += sigma1(W[ 5]) + W[ 0] + sigma0(W[ 8]);
    W[ 8] += sigma1(W[ 6]) + W[ 1] + sigma0(W[ 9]);
    W[ 9] += sigma1(W[ 7]) + W[ 2] + sigma0(W[10]);
    W[10] += sigma1(W[ 8]) + W[ 3] + sigma0(W[11]);
    W[11] += sigma1(W[ 9]) + W[ 4] + sigma0(W[12]);
    W[12] += sigma1(W[10]) + W[ 5] + sigma0(W[13]);
    W[13] += sigma1(W[11]) + W[ 6] + sigma0(W[14]);
    W[14] += sigma1(W[12]) + W[ 7] + sigma0(W[15]);
    W[15] += sigma1(W[13]) + W[ 8] + sigma0(W[ 0]);

    // 32..47
    tmp = S[7] + Sigma1(S[4]) + Ch(S[4], S[5], S[6]) + W[ 0] + K[32]; S[3] += tmp; S[7] = tmp + Sigma0(S[0]) + Maj(S[0], S[1], S[2]);
    tmp = S[6] + Sigma1(S[3]) + Ch(S[3], S[4], S[5]) + W[ 1] + K[33]; S[2] += tmp; S[6] = tmp + Sigma0(S[7]) + Maj(S[7], S[0], S[1]);
    tmp = S[5] + Sigma1(S[2]) + Ch(S[2], S[3], S[4]) + W[ 2] + K[34]; S[1] += tmp; S[5] = tmp + Sigma0(S[6]) + Maj(S[6], S[7], S[0]);
    tmp = S[4] + Sigma1(S[1]) + Ch(S[1], S[2], S[3]) + W[ 3] + K[35]; S[0] += tmp; S[4] = tmp + Sigma0(S[5]) + Maj(S[5], S[6], S[7]);
    tmp = S[3] + Sigma1(S[0]) + Ch(S[0], S[1], S[2]) + W[ 4] + K[36]; S[7] += tmp; S[3] = tmp + Sigma0(S[4]) + Maj(S[4], S[5], S[6]);
    tmp = S[2] + Sigma1(S[7]) + Ch(S[7], S[0], S[1]) + W[ 5] + K[37]; S[6] += tmp; S[2] = tmp + Sigma0(S[3]) + Maj(S[3], S[4], S[5]);
    tmp = S[1] + Sigma1(S[6]) + Ch(S[6], S[7], S[0]) + W[ 6] + K[38]; S[5] += tmp; S[1] = tmp + Sigma0(S[2]) + Maj(S[2], S[3], S[4]);
    tmp = S[0] + Sigma1(S[5]) + Ch(S[5], S[6], S[7]) + W[ 7] + K[39]; S[4] += tmp; S[0] = tmp + Sigma0(S[1]) + Maj(S[1], S[2], S[3]);
    tmp = S[7] + Sigma1(S[4]) + Ch(S[4], S[5], S[6]) + W[ 8] + K[40]; S[3] += tmp; S[7] = tmp + Sigma0(S[0]) + Maj(S[0], S[1], S[2]);
    tmp = S[6] + Sigma1(S[3]) + Ch(S[3], S[4], S[5]) + W[ 9] + K[41]; S[2] += tmp; S[6] = tmp + Sigma0(S[7]) + Maj(S[7], S[0], S[1]);
    tmp = S[5] + Sigma1(S[2]) + Ch(S[2], S[3], S[4]) + W[10] + K[42]; S[1] += tmp; S[5] = tmp + Sigma0(S[6]) + Maj(S[6], S[7], S[0]);
    tmp = S[4] + Sigma1(S[1]) + Ch(S[1], S[2], S[3]) + W[11] + K[43]; S[0] += tmp; S[4] = tmp + Sigma0(S[5]) + Maj(S[5], S[6], S[7]);
    tmp = S[3] + Sigma1(S[0]) + Ch(S[0], S[1], S[2]) + W[12] + K[44]; S[7] += tmp; S[3] = tmp + Sigma0(S[4]) + Maj(S[4], S[5], S[6]);
    tmp = S[2] + Sigma1(S[7]) + Ch(S[7], S[0], S[1]) + W[13] + K[45]; S[6] += tmp; S[2] = tmp + Sigma0(S[3]) + Maj(S[3], S[4], S[5]);
    tmp = S[1] + Sigma1(S[6]) + Ch(S[6], S[7], S[0]) + W[14] + K[46]; S[5] += tmp; S[1] = tmp + Sigma0(S[2]) + Maj(S[2], S[3], S[4]);
    tmp = S[0] + Sigma1(S[5]) + Ch(S[5], S[6], S[7]) + W[15] + K[47]; S[4] += tmp; S[0] = tmp + Sigma0(S[1]) + Maj(S[1], S[2], S[3]);

    // message schedule 3
    W[ 0] += sigma1(W[14]) + W[ 9] + sigma0(W[ 1]);
    W[ 1] += sigma1(W[15]) + W[10] + sigma0(W[ 2]);
    W[ 2] += sigma1(W[ 0]) + W[11] + sigma0(W[ 3]);
    W[ 3] += sigma1(W[ 1]) + W[12] + sigma0(W[ 4]);
    W[ 4] += sigma1(W[ 2]) + W[13] + sigma0(W[ 5]);
    W[ 5] += sigma1(W[ 3]) + W[14] + sigma0(W[ 6]);
    W[ 6] += sigma1(W[ 4]) + W[15] + sigma0(W[ 7]);
    W[ 7] += sigma1(W[ 5]) + W[ 0] + sigma0(W[ 8]);
    W[ 8] += sigma1(W[ 6]) + W[ 1] + sigma0(W[ 9]);
    W[ 9] += sigma1(W[ 7]) + W[ 2] + sigma0(W[10]);
    W[10] += sigma1(W[ 8]) + W[ 3] + sigma0(W[11]);
    W[11] += sigma1(W[ 9]) + W[ 4] + sigma0(W[12]);
    W[12] += sigma1(W[10]) + W[ 5] + sigma0(W[13]);
    W[13] += sigma1(W[11]) + W[ 6] + sigma0(W[14]);
    W[14] += sigma1(W[12]) + W[ 7] + sigma0(W[15]);
    W[15] += sigma1(W[13]) + W[ 8] + sigma0(W[ 0]);

    // 48..63
    tmp = S[7] + Sigma1(S[4]) + Ch(S[4], S[5], S[6]) + W[ 0] + K[48]; S[3] += tmp; S[7] = tmp + Sigma0(S[0]) + Maj(S[0], S[1], S[2]);
    tmp = S[6] + Sigma1(S[3]) + Ch(S[3], S[4], S[5]) + W[ 1] + K[49]; S[2] += tmp; S[6] = tmp + Sigma0(S[7]) + Maj(S[7], S[0], S[1]);
    tmp = S[5] + Sigma1(S[2]) + Ch(S[2], S[3], S[4]) + W[ 2] + K[50]; S[1] += tmp; S[5] = tmp + Sigma0(S[6]) + Maj(S[6], S[7], S[0]);
    tmp = S[4] + Sigma1(S[1]) + Ch(S[1], S[2], S[3]) + W[ 3] + K[51]; S[0] += tmp; S[4] = tmp + Sigma0(S[5]) + Maj(S[5], S[6], S[7]);
    tmp = S[3] + Sigma1(S[0]) + Ch(S[0], S[1], S[2]) + W[ 4] + K[52]; S[7] += tmp; S[3] = tmp + Sigma0(S[4]) + Maj(S[4], S[5], S[6]);
    tmp = S[2] + Sigma1(S[7]) + Ch(S[7], S[0], S[1]) + W[ 5] + K[53]; S[6] += tmp; S[2] = tmp + Sigma0(S[3]) + Maj(S[3], S[4], S[5]);
    tmp = S[1] + Sigma1(S[6]) + Ch(S[6], S[7], S[0]) + W[ 6] + K[54]; S[5] += tmp; S[1] = tmp + Sigma0(S[2]) + Maj(S[2], S[3], S[4]);
    tmp = S[0] + Sigma1(S[5]) + Ch(S[5], S[6], S[7]) + W[ 7] + K[55]; S[4] += tmp; S[0] = tmp + Sigma0(S[1]) + Maj(S[1], S[2], S[3]);
    tmp = S[7] + Sigma1(S[4]) + Ch(S[4], S[5], S[6]) + W[ 8] + K[56]; S[3] += tmp; S[7] = tmp + Sigma0(S[0]) + Maj(S[0], S[1], S[2]);
    tmp = S[6] + Sigma1(S[3]) + Ch(S[3], S[4], S[5]) + W[ 9] + K[57]; S[2] += tmp; S[6] = tmp + Sigma0(S[7]) + Maj(S[7], S[0], S[1]);
    tmp = S[5] + Sigma1(S[2]) + Ch(S[2], S[3], S[4]) + W[10] + K[58]; S[1] += tmp; S[5] = tmp + Sigma0(S[6]) + Maj(S[6], S[7], S[0]);
    tmp = S[4] + Sigma1(S[1]) + Ch(S[1], S[2], S[3]) + W[11] + K[59]; S[0] += tmp; S[4] = tmp + Sigma0(S[5]) + Maj(S[5], S[6], S[7]);
    tmp = S[3] + Sigma1(S[0]) + Ch(S[0], S[1], S[2]) + W[12] + K[60]; S[7] += tmp; S[3] = tmp + Sigma0(S[4]) + Maj(S[4], S[5], S[6]);
    tmp = S[2] + Sigma1(S[7]) + Ch(S[7], S[0], S[1]) + W[13] + K[61]; S[6] += tmp; S[2] = tmp + Sigma0(S[3]) + Maj(S[3], S[4], S[5]);
    tmp = S[1] + Sigma1(S[6]) + Ch(S[6], S[7], S[0]) + W[14] + K[62]; S[5] += tmp; S[1] = tmp + Sigma0(S[2]) + Maj(S[2], S[3], S[4]);
    tmp = S[0] + Sigma1(S[5]) + Ch(S[5], S[6], S[7]) + W[15] + K[63]; S[4] += tmp; S[0] = tmp + Sigma0(S[1]) + Maj(S[1], S[2], S[3]);

    if (0 == ((S[0] + 0x6A09E667) & __MASK0__) &&
        0 == ((S[1] + 0xBB67AE85) & __MASK1__)
    ) {
      output[0] = i;    // i >= 1
      output[1] = thread_id;
      return;
    }
  }
}
