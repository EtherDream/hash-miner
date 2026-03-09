#pragma once
#include <cstdint>
#include <concepts>
#include <type_traits>
#include <wasm_simd128.h>

//
// WGSL-like Wasm SIMD vectors
//
namespace wasm_vec4 {

using i32 = std::int32_t;
using u32 = std::uint32_t;
using f32 = float;

template<class T>
concept lane_i32 = std::same_as<T, i32>;
template<class T>
concept lane_u32 = std::same_as<T, u32>;
template<class T>
concept lane_f32 = std::same_as<T, f32>;
template<class T>
concept lane_int = lane_i32<T> || lane_u32<T>;
template<class T>
concept lane_supported = lane_int<T> || lane_f32<T>;

struct bvec4 {
  v128_t m{};

  constexpr bvec4() = default;
  explicit constexpr bvec4(v128_t v) noexcept : m(v) {}

  v128_t native() const noexcept {
    return m;
  }
  friend bvec4 operator&(bvec4 a, bvec4 b) noexcept { return bvec4(wasm_v128_and(a.m, b.m)); }
  friend bvec4 operator|(bvec4 a, bvec4 b) noexcept { return bvec4(wasm_v128_or(a.m, b.m)); }
  friend bvec4 operator^(bvec4 a, bvec4 b) noexcept { return bvec4(wasm_v128_xor(a.m, b.m)); }
  friend bvec4 operator~(bvec4 a) noexcept { return bvec4(wasm_v128_not(a.m)); }

  bool any() const noexcept { return wasm_i32x4_bitmask(m) != 0; }
  bool all() const noexcept { return wasm_i32x4_bitmask(m) == 0b1111; }
};

// Ops traits: specialize once per lane type.
template<lane_supported T>
struct vec4_ops;

// ---- i32 ----
template<>
struct vec4_ops<i32> {
  using lane = i32;

  static v128_t splat(lane s) noexcept {
    return wasm_i32x4_splat(s);
  }
  static v128_t make(lane x, lane y, lane z, lane w) noexcept {
    return wasm_i32x4_make(x, y, z, w);
  }

  static lane x(v128_t v) noexcept { return wasm_i32x4_extract_lane(v, 0); }
  static lane y(v128_t v) noexcept { return wasm_i32x4_extract_lane(v, 1); }
  static lane z(v128_t v) noexcept { return wasm_i32x4_extract_lane(v, 2); }
  static lane w(v128_t v) noexcept { return wasm_i32x4_extract_lane(v, 3); }

  static v128_t x(v128_t v, lane s) noexcept { return wasm_i32x4_replace_lane(v, 0, s); }
  static v128_t y(v128_t v, lane s) noexcept { return wasm_i32x4_replace_lane(v, 1, s); }
  static v128_t z(v128_t v, lane s) noexcept { return wasm_i32x4_replace_lane(v, 2, s); }
  static v128_t w(v128_t v, lane s) noexcept { return wasm_i32x4_replace_lane(v, 3, s); }

  static v128_t add(v128_t a, v128_t b) noexcept { return wasm_i32x4_add(a, b); }
  static v128_t sub(v128_t a, v128_t b) noexcept { return wasm_i32x4_sub(a, b); }
  static v128_t mul(v128_t a, v128_t b) noexcept { return wasm_i32x4_mul(a, b); }

  static v128_t shl(v128_t a, u32 s) noexcept { return wasm_i32x4_shl(a, s); }
  static v128_t shr(v128_t a, u32 s) noexcept { return wasm_i32x4_shr(a, s); } // arithmetic

  static v128_t eq(v128_t a, v128_t b) noexcept { return wasm_i32x4_eq(a, b); }
  static v128_t ne(v128_t a, v128_t b) noexcept { return wasm_i32x4_ne(a, b); }
  static v128_t lt(v128_t a, v128_t b) noexcept { return wasm_i32x4_lt(a, b); }
  static v128_t le(v128_t a, v128_t b) noexcept { return wasm_i32x4_le(a, b); }
  static v128_t gt(v128_t a, v128_t b) noexcept { return wasm_i32x4_gt(a, b); }
  static v128_t ge(v128_t a, v128_t b) noexcept { return wasm_i32x4_ge(a, b); }
};

// ---- u32 ----
template<>
struct vec4_ops<u32> {
  using lane = u32;

  static v128_t splat(lane s) noexcept {
    return wasm_i32x4_splat((i32) s);
  }
  static v128_t make(lane x, lane y, lane z, lane w) noexcept {
    return wasm_i32x4_make((i32) x, (i32) y, (i32) z, (i32) w);
  }

  static lane x(v128_t v) noexcept { return (lane)wasm_i32x4_extract_lane(v, 0); }
  static lane y(v128_t v) noexcept { return (lane)wasm_i32x4_extract_lane(v, 1); }
  static lane z(v128_t v) noexcept { return (lane)wasm_i32x4_extract_lane(v, 2); }
  static lane w(v128_t v) noexcept { return (lane)wasm_i32x4_extract_lane(v, 3); }

  static v128_t x(v128_t v, lane s) noexcept { return wasm_i32x4_replace_lane(v, 0, (i32)s); }
  static v128_t y(v128_t v, lane s) noexcept { return wasm_i32x4_replace_lane(v, 1, (i32)s); }
  static v128_t z(v128_t v, lane s) noexcept { return wasm_i32x4_replace_lane(v, 2, (i32)s); }
  static v128_t w(v128_t v, lane s) noexcept { return wasm_i32x4_replace_lane(v, 3, (i32)s); }

  // mod 2^32 arithmetic; i32x4 ops are fine bitwise
  static v128_t add(v128_t a, v128_t b) noexcept { return wasm_i32x4_add(a, b); }
  static v128_t sub(v128_t a, v128_t b) noexcept { return wasm_i32x4_sub(a, b); }
  static v128_t mul(v128_t a, v128_t b) noexcept { return wasm_i32x4_mul(a, b); }

  static v128_t shl(v128_t a, u32 s) noexcept { return wasm_i32x4_shl(a, s); }
  static v128_t shr(v128_t a, u32 s) noexcept { return wasm_u32x4_shr(a, s); } // logical

  static v128_t eq(v128_t a, v128_t b) noexcept { return wasm_i32x4_eq(a, b); }
  static v128_t ne(v128_t a, v128_t b) noexcept { return wasm_i32x4_ne(a, b); }
  static v128_t lt(v128_t a, v128_t b) noexcept { return wasm_u32x4_lt(a, b); }
  static v128_t le(v128_t a, v128_t b) noexcept { return wasm_u32x4_le(a, b); }
  static v128_t gt(v128_t a, v128_t b) noexcept { return wasm_u32x4_gt(a, b); }
  static v128_t ge(v128_t a, v128_t b) noexcept { return wasm_u32x4_ge(a, b); }
};

// ---- f32 ----
template<>
struct vec4_ops<f32> {
  using lane = f32;

  static v128_t splat(lane s) noexcept {
    return wasm_f32x4_splat(s);
  }
  static v128_t make(lane x, lane y, lane z, lane w) noexcept {
    return wasm_f32x4_make(x, y, z, w);
  }

  static lane x(v128_t v) noexcept { return wasm_f32x4_extract_lane(v, 0); }
  static lane y(v128_t v) noexcept { return wasm_f32x4_extract_lane(v, 1); }
  static lane z(v128_t v) noexcept { return wasm_f32x4_extract_lane(v, 2); }
  static lane w(v128_t v) noexcept { return wasm_f32x4_extract_lane(v, 3); }

  static v128_t x(v128_t v, lane s) noexcept { return wasm_f32x4_replace_lane(v, 0, s); }
  static v128_t y(v128_t v, lane s) noexcept { return wasm_f32x4_replace_lane(v, 1, s); }
  static v128_t z(v128_t v, lane s) noexcept { return wasm_f32x4_replace_lane(v, 2, s); }
  static v128_t w(v128_t v, lane s) noexcept { return wasm_f32x4_replace_lane(v, 3, s); }

  static v128_t add(v128_t a, v128_t b) noexcept { return wasm_f32x4_add(a, b); }
  static v128_t sub(v128_t a, v128_t b) noexcept { return wasm_f32x4_sub(a, b); }
  static v128_t mul(v128_t a, v128_t b) noexcept { return wasm_f32x4_mul(a, b); }
  static v128_t div(v128_t a, v128_t b) noexcept { return wasm_f32x4_div(a, b); }

  static v128_t eq(v128_t a, v128_t b) noexcept { return wasm_f32x4_eq(a, b); }
  static v128_t ne(v128_t a, v128_t b) noexcept { return wasm_f32x4_ne(a, b); }
  static v128_t lt(v128_t a, v128_t b) noexcept { return wasm_f32x4_lt(a, b); }
  static v128_t le(v128_t a, v128_t b) noexcept { return wasm_f32x4_le(a, b); }
  static v128_t gt(v128_t a, v128_t b) noexcept { return wasm_f32x4_gt(a, b); }
  static v128_t ge(v128_t a, v128_t b) noexcept { return wasm_f32x4_ge(a, b); }
};

template<lane_supported T>
class vec4 {
  using ops = vec4_ops<T>;

  v128_t _v{ops::splat((T) 0)};

public:
  using lane_type = T;

  vec4() noexcept = default;

  explicit vec4(v128_t v) noexcept
    : _v(v) {}

  explicit vec4(T s) noexcept
    : _v(ops::splat(s)) {}

  vec4(T x, T y, T z, T w) noexcept
    : _v(ops::make(x, y, z, w)) {}

  v128_t native() const noexcept {
    return _v;
  }
  static vec4 load(const void* p) noexcept {
    return vec4(wasm_v128_load(p));
  }
  void store(void* p) const noexcept {
    wasm_v128_store(p, _v);
  }

  // fixed lanes only (lane must be immediate)
  T x() const noexcept { return ops::x(_v); }
  T y() const noexcept { return ops::y(_v); }
  T z() const noexcept { return ops::z(_v); }
  T w() const noexcept { return ops::w(_v); }

  void x(T s) noexcept { _v = ops::x(_v, s); }
  void y(T s) noexcept { _v = ops::y(_v, s); }
  void z(T s) noexcept { _v = ops::z(_v, s); }
  void w(T s) noexcept { _v = ops::w(_v, s); }

  T r() const noexcept { return x(); }
  T g() const noexcept { return y(); }
  T b() const noexcept { return z(); }
  T a() const noexcept { return w(); }

  void r(T s) noexcept { x(s); }
  void g(T s) noexcept { y(s); }
  void b(T s) noexcept { z(s); }
  void a(T s) noexcept { w(s); }

  // ---- arithmetic vec-vec ----
  [[nodiscard]] friend vec4 operator+(vec4 a, vec4 b) noexcept { return vec4(ops::add(a._v, b._v)); }
  [[nodiscard]] friend vec4 operator-(vec4 a, vec4 b) noexcept { return vec4(ops::sub(a._v, b._v)); }
  [[nodiscard]] friend vec4 operator*(vec4 a, vec4 b) noexcept { return vec4(ops::mul(a._v, b._v)); }

  // f32 only: division
  [[nodiscard]] friend vec4 operator/(vec4 a, vec4 b) noexcept requires(lane_f32<T>) {
    return vec4(ops::div(a._v, b._v));
  }

  // ---- arithmetic vec-scalar broadcast ----
  [[nodiscard]] friend vec4 operator+(vec4 a, T s) noexcept { return a + vec4(s); }
  [[nodiscard]] friend vec4 operator-(vec4 a, T s) noexcept { return a - vec4(s); }
  [[nodiscard]] friend vec4 operator*(vec4 a, T s) noexcept { return a * vec4(s); }

  [[nodiscard]] friend vec4 operator+(T s, vec4 a) noexcept { return vec4(s) + a; }
  [[nodiscard]] friend vec4 operator-(T s, vec4 a) noexcept { return vec4(s) - a; }
  [[nodiscard]] friend vec4 operator*(T s, vec4 a) noexcept { return vec4(s) * a; }

  [[nodiscard]] friend vec4 operator/(vec4 a, T s) noexcept requires(lane_f32<T>) { return a / vec4(s); }
  [[nodiscard]] friend vec4 operator/(T s, vec4 a) noexcept requires(lane_f32<T>) { return vec4(s) / a; }

  vec4& operator+=(vec4 rhs) noexcept {
    _v = ops::add(_v, rhs._v);
    return *this;
  }

  vec4& operator-=(vec4 rhs) noexcept {
    _v = ops::sub(_v, rhs._v);
    return *this;
  }

  vec4& operator*=(vec4 rhs) noexcept {
    _v = ops::mul(_v, rhs._v);
    return *this;
  }

  vec4& operator/=(vec4 rhs) noexcept requires(lane_f32<T>) {
    _v = ops::div(_v, rhs._v);
    return *this;
  }

  vec4& operator+=(T s) noexcept { return (*this += vec4(s)); }
  vec4& operator-=(T s) noexcept { return (*this -= vec4(s)); }
  vec4& operator*=(T s) noexcept { return (*this *= vec4(s)); }
  vec4& operator/=(T s) noexcept requires(lane_f32<T>) { return (*this /= vec4(s)); }

  // ---- bitwise (ints only) ----
  [[nodiscard]] friend vec4 operator&(vec4 a, vec4 b) noexcept requires(lane_int<T>) { return vec4(wasm_v128_and(a._v, b._v)); }
  [[nodiscard]] friend vec4 operator|(vec4 a, vec4 b) noexcept requires(lane_int<T>) { return vec4(wasm_v128_or(a._v, b._v)); }
  [[nodiscard]] friend vec4 operator^(vec4 a, vec4 b) noexcept requires(lane_int<T>) { return vec4(wasm_v128_xor(a._v, b._v)); }
  [[nodiscard]] friend vec4 operator~(vec4 a) noexcept requires(lane_int<T>) { return vec4(wasm_v128_not(a._v)); }

  [[nodiscard]] friend vec4 operator&(vec4 a, T s) noexcept requires(lane_int<T>) { return a & vec4(s); }
  [[nodiscard]] friend vec4 operator|(vec4 a, T s) noexcept requires(lane_int<T>) { return a | vec4(s); }
  [[nodiscard]] friend vec4 operator^(vec4 a, T s) noexcept requires(lane_int<T>) { return a ^ vec4(s); }

  vec4& operator&=(vec4 rhs) noexcept requires(lane_int<T>) {
    _v = wasm_v128_and(_v, rhs._v);
    return *this;
  }

  vec4& operator|=(vec4 rhs) noexcept requires(lane_int<T>) {
    _v = wasm_v128_or(_v, rhs._v);
    return *this;
  }

  vec4& operator^=(vec4 rhs) noexcept requires(lane_int<T>) {
    _v = wasm_v128_xor(_v, rhs._v);
    return *this;
  }

  vec4& operator&=(T s) noexcept requires(lane_int<T>) { return (*this &= vec4(s)); }
  vec4& operator|=(T s) noexcept requires(lane_int<T>) { return (*this |= vec4(s)); }
  vec4& operator^=(T s) noexcept requires(lane_int<T>) { return (*this ^= vec4(s)); }

  // ---- shifts (ints only; WGSL: i32 arithmetic, u32 logical) ----
  [[nodiscard]] friend vec4 operator<<(vec4 a, u32 s) noexcept requires(lane_int<T>) { return vec4(ops::shl(a._v, s)); }
  [[nodiscard]] friend vec4 operator>>(vec4 a, u32 s) noexcept requires(lane_int<T>) { return vec4(ops::shr(a._v, s)); }

  vec4& operator<<=(u32 s) noexcept requires(lane_int<T>) {
    _v = ops::shl(_v, s);
    return *this;
  }

  vec4& operator>>=(u32 s) noexcept requires(lane_int<T>) {
    _v = ops::shr(_v, s);
    return *this;
  }

  // ---- comparisons -> bvec4 mask (WGSL vec4<bool> analogue) ----
  [[nodiscard]] friend bvec4 operator==(vec4 a, vec4 b) noexcept { return bvec4(ops::eq(a._v, b._v)); }
  [[nodiscard]] friend bvec4 operator!=(vec4 a, vec4 b) noexcept { return bvec4(ops::ne(a._v, b._v)); }
  [[nodiscard]] friend bvec4 operator< (vec4 a, vec4 b) noexcept { return bvec4(ops::lt(a._v, b._v)); }
  [[nodiscard]] friend bvec4 operator<=(vec4 a, vec4 b) noexcept { return bvec4(ops::le(a._v, b._v)); }
  [[nodiscard]] friend bvec4 operator> (vec4 a, vec4 b) noexcept { return bvec4(ops::gt(a._v, b._v)); }
  [[nodiscard]] friend bvec4 operator>=(vec4 a, vec4 b) noexcept { return bvec4(ops::ge(a._v, b._v)); }

  [[nodiscard]] friend bvec4 operator==(vec4 a, T s) noexcept { return a == vec4(s); }
  [[nodiscard]] friend bvec4 operator!=(vec4 a, T s) noexcept { return a != vec4(s); }
  [[nodiscard]] friend bvec4 operator< (vec4 a, T s) noexcept { return a <  vec4(s); }
  [[nodiscard]] friend bvec4 operator<=(vec4 a, T s) noexcept { return a <= vec4(s); }
  [[nodiscard]] friend bvec4 operator> (vec4 a, T s) noexcept { return a >  vec4(s); }
  [[nodiscard]] friend bvec4 operator>=(vec4 a, T s) noexcept { return a >= vec4(s); }

  // ---- WGSL-like select(a, b, cond): cond ? b : a ----
  [[nodiscard]] friend vec4 select(vec4 a, vec4 b, bvec4 cond) noexcept {
    return vec4(wasm_v128_bitselect(b._v, a._v, cond.m));
  }
};

// Convenience aliases
using vec4i = vec4<i32>;
using vec4u = vec4<u32>;
using vec4f = vec4<f32>;

} // end namespace