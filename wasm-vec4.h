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

template<typename T>
concept lane_i32 = std::same_as<T, i32>;
template<typename T>
concept lane_u32 = std::same_as<T, u32>;
template<typename T>
concept lane_f32 = std::same_as<T, f32>;
template<typename T>
concept lane_int = lane_i32<T> || lane_u32<T>;
template<typename T>
concept lane_num = lane_int<T> || lane_f32<T>;

template<typename T>
class vec4;

// ------------------------------------------------------------
// vec4<bool> (mask semantics)
//   true  lane => 0xFFFF'FFFF
//   false lane => 0x0000'0000
// ------------------------------------------------------------
template<>
class vec4<bool> {
public:
  using lane_type = bool;

  vec4() = default;
  explicit vec4(bool s) : _v(splat_(s)) {}
  vec4(bool x, bool y, bool z, bool w) : _v(make_(x, y, z, w)) {}

  v128_t native() const { return _v; }

  bool x() const { return lane0_(_v); }
  bool y() const { return lane1_(_v); }
  bool z() const { return lane2_(_v); }
  bool w() const { return lane3_(_v); }

  void x(bool s) { _v = set_lane0_(_v, s); }
  void y(bool s) { _v = set_lane1_(_v, s); }
  void z(bool s) { _v = set_lane2_(_v, s); }
  void w(bool s) { _v = set_lane3_(_v, s); }

  u32 bitmask() const { return (u32) wasm_i32x4_bitmask(_v); }
  bool any() const { return bitmask() != 0; }
  bool all() const { return bitmask() == 0b1111; }

  friend vec4 operator&(vec4 a, vec4 b) { return vec4(wasm_v128_and(a._v, b._v)); }
  friend vec4 operator|(vec4 a, vec4 b) { return vec4(wasm_v128_or(a._v, b._v)); }
  friend vec4 operator^(vec4 a, vec4 b) { return vec4(wasm_v128_xor(a._v, b._v)); }
  friend vec4 operator~(vec4 a) { return vec4(wasm_v128_not(a._v)); }

  vec4& operator&=(vec4 rhs) { _v = wasm_v128_and(_v, rhs._v); return *this; }
  vec4& operator|=(vec4 rhs) { _v = wasm_v128_or(_v, rhs._v); return *this; }
  vec4& operator^=(vec4 rhs) { _v = wasm_v128_xor(_v, rhs._v); return *this; }

private:
  v128_t _v{wasm_i32x4_splat(0)};

  static constexpr i32 kTrue = -1;
  static constexpr i32 kFalse = 0;

  explicit vec4(v128_t v) : _v(v) {}

  static v128_t splat_(bool s) {
    return wasm_i32x4_splat(s ? kTrue : kFalse);
  }

  static v128_t make_(bool x, bool y, bool z, bool w) {
    return wasm_i32x4_make(
      x ? kTrue : kFalse,
      y ? kTrue : kFalse,
      z ? kTrue : kFalse,
      w ? kTrue : kFalse
    );
  }

  static bool lane0_(v128_t v) { return wasm_i32x4_extract_lane(v, 0) < 0; }
  static bool lane1_(v128_t v) { return wasm_i32x4_extract_lane(v, 1) < 0; }
  static bool lane2_(v128_t v) { return wasm_i32x4_extract_lane(v, 2) < 0; }
  static bool lane3_(v128_t v) { return wasm_i32x4_extract_lane(v, 3) < 0; }

  static v128_t set_lane0_(v128_t v, bool s) {
    return wasm_i32x4_replace_lane(v, 0, s ? kTrue : kFalse);
  }
  static v128_t set_lane1_(v128_t v, bool s) {
    return wasm_i32x4_replace_lane(v, 1, s ? kTrue : kFalse);
  }
  static v128_t set_lane2_(v128_t v, bool s) {
    return wasm_i32x4_replace_lane(v, 2, s ? kTrue : kFalse);
  }
  static v128_t set_lane3_(v128_t v, bool s) {
    return wasm_i32x4_replace_lane(v, 3, s ? kTrue : kFalse);
  }

  template<typename T>
  friend class vec4;
};

inline bool any(vec4<bool> v) { return v.any(); }
inline bool all(vec4<bool> v) { return v.all(); }

// ------------------------------------------------------------
// vec4_ops<T>
// ------------------------------------------------------------
template<typename T>
struct vec4_ops;

template<>
struct vec4_ops<i32> {
  using lane = i32;

  static v128_t splat(lane s) { return wasm_i32x4_splat(s); }
  static v128_t make(lane x, lane y, lane z, lane w) {
    return wasm_i32x4_make(x, y, z, w);
  }

  static lane x(v128_t v) { return wasm_i32x4_extract_lane(v, 0); }
  static lane y(v128_t v) { return wasm_i32x4_extract_lane(v, 1); }
  static lane z(v128_t v) { return wasm_i32x4_extract_lane(v, 2); }
  static lane w(v128_t v) { return wasm_i32x4_extract_lane(v, 3); }

  static v128_t x(v128_t v, lane s) { return wasm_i32x4_replace_lane(v, 0, s); }
  static v128_t y(v128_t v, lane s) { return wasm_i32x4_replace_lane(v, 1, s); }
  static v128_t z(v128_t v, lane s) { return wasm_i32x4_replace_lane(v, 2, s); }
  static v128_t w(v128_t v, lane s) { return wasm_i32x4_replace_lane(v, 3, s); }

  static v128_t add(v128_t a, v128_t b) { return wasm_i32x4_add(a, b); }
  static v128_t sub(v128_t a, v128_t b) { return wasm_i32x4_sub(a, b); }
  static v128_t mul(v128_t a, v128_t b) { return wasm_i32x4_mul(a, b); }

  static v128_t shl(v128_t a, u32 s) { return wasm_i32x4_shl(a, s); }
  static v128_t shr(v128_t a, u32 s) { return wasm_i32x4_shr(a, s); }

  static v128_t eq(v128_t a, v128_t b) { return wasm_i32x4_eq(a, b); }
  static v128_t ne(v128_t a, v128_t b) { return wasm_i32x4_ne(a, b); }
  static v128_t lt(v128_t a, v128_t b) { return wasm_i32x4_lt(a, b); }
  static v128_t le(v128_t a, v128_t b) { return wasm_i32x4_le(a, b); }
  static v128_t gt(v128_t a, v128_t b) { return wasm_i32x4_gt(a, b); }
  static v128_t ge(v128_t a, v128_t b) { return wasm_i32x4_ge(a, b); }
};

template<>
struct vec4_ops<u32> {
  using lane = u32;

  static v128_t splat(lane s) {
    return wasm_i32x4_splat((i32) s);
  }

  static v128_t make(lane x, lane y, lane z, lane w) {
    return wasm_i32x4_make((i32) x, (i32) y, (i32) z, (i32) w);
  }

  static lane x(v128_t v) { return (lane) wasm_i32x4_extract_lane(v, 0); }
  static lane y(v128_t v) { return (lane) wasm_i32x4_extract_lane(v, 1); }
  static lane z(v128_t v) { return (lane) wasm_i32x4_extract_lane(v, 2); }
  static lane w(v128_t v) { return (lane) wasm_i32x4_extract_lane(v, 3); }

  static v128_t x(v128_t v, lane s) {
    return wasm_i32x4_replace_lane(v, 0, (i32) s);
  }
  static v128_t y(v128_t v, lane s) {
    return wasm_i32x4_replace_lane(v, 1, (i32) s);
  }
  static v128_t z(v128_t v, lane s) {
    return wasm_i32x4_replace_lane(v, 2, (i32) s);
  }
  static v128_t w(v128_t v, lane s) {
    return wasm_i32x4_replace_lane(v, 3, (i32) s);
  }

  static v128_t add(v128_t a, v128_t b) { return wasm_i32x4_add(a, b); }
  static v128_t sub(v128_t a, v128_t b) { return wasm_i32x4_sub(a, b); }
  static v128_t mul(v128_t a, v128_t b) { return wasm_i32x4_mul(a, b); }

  static v128_t shl(v128_t a, u32 s) { return wasm_i32x4_shl(a, s); }
  static v128_t shr(v128_t a, u32 s) { return wasm_u32x4_shr(a, s); }

  static v128_t eq(v128_t a, v128_t b) { return wasm_i32x4_eq(a, b); }
  static v128_t ne(v128_t a, v128_t b) { return wasm_i32x4_ne(a, b); }
  static v128_t lt(v128_t a, v128_t b) { return wasm_u32x4_lt(a, b); }
  static v128_t le(v128_t a, v128_t b) { return wasm_u32x4_le(a, b); }
  static v128_t gt(v128_t a, v128_t b) { return wasm_u32x4_gt(a, b); }
  static v128_t ge(v128_t a, v128_t b) { return wasm_u32x4_ge(a, b); }
};

template<>
struct vec4_ops<f32> {
  using lane = f32;

  static v128_t splat(lane s) { return wasm_f32x4_splat(s); }
  static v128_t make(lane x, lane y, lane z, lane w) {
    return wasm_f32x4_make(x, y, z, w);
  }

  static lane x(v128_t v) { return wasm_f32x4_extract_lane(v, 0); }
  static lane y(v128_t v) { return wasm_f32x4_extract_lane(v, 1); }
  static lane z(v128_t v) { return wasm_f32x4_extract_lane(v, 2); }
  static lane w(v128_t v) { return wasm_f32x4_extract_lane(v, 3); }

  static v128_t x(v128_t v, lane s) { return wasm_f32x4_replace_lane(v, 0, s); }
  static v128_t y(v128_t v, lane s) { return wasm_f32x4_replace_lane(v, 1, s); }
  static v128_t z(v128_t v, lane s) { return wasm_f32x4_replace_lane(v, 2, s); }
  static v128_t w(v128_t v, lane s) { return wasm_f32x4_replace_lane(v, 3, s); }

  static v128_t add(v128_t a, v128_t b) { return wasm_f32x4_add(a, b); }
  static v128_t sub(v128_t a, v128_t b) { return wasm_f32x4_sub(a, b); }
  static v128_t mul(v128_t a, v128_t b) { return wasm_f32x4_mul(a, b); }
  static v128_t div(v128_t a, v128_t b) { return wasm_f32x4_div(a, b); }

  static v128_t eq(v128_t a, v128_t b) { return wasm_f32x4_eq(a, b); }
  static v128_t ne(v128_t a, v128_t b) { return wasm_f32x4_ne(a, b); }
  static v128_t lt(v128_t a, v128_t b) { return wasm_f32x4_lt(a, b); }
  static v128_t le(v128_t a, v128_t b) { return wasm_f32x4_le(a, b); }
  static v128_t gt(v128_t a, v128_t b) { return wasm_f32x4_gt(a, b); }
  static v128_t ge(v128_t a, v128_t b) { return wasm_f32x4_ge(a, b); }
};

// ------------------------------------------------------------
// vec4<T> numeric (partial specialization)
// ------------------------------------------------------------
template<typename T>
  requires lane_num<T>
class vec4<T> {
  using ops = vec4_ops<T>;
  v128_t _v{ops::splat((T) 0)};

public:
  using lane_type = T;

  vec4() = default;
  explicit vec4(v128_t v) : _v(v) {}
  explicit vec4(T s) : _v(ops::splat(s)) {}
  vec4(T x, T y, T z, T w) : _v(ops::make(x, y, z, w)) {}

  v128_t native() const { return _v; }

  static vec4 load(const void* p) { return vec4(wasm_v128_load(p)); }
  void store(void* p) const { wasm_v128_store(p, _v); }

  T x() const { return ops::x(_v); }
  T y() const { return ops::y(_v); }
  T z() const { return ops::z(_v); }
  T w() const { return ops::w(_v); }

  void x(T s) { _v = ops::x(_v, s); }
  void y(T s) { _v = ops::y(_v, s); }
  void z(T s) { _v = ops::z(_v, s); }
  void w(T s) { _v = ops::w(_v, s); }

  friend vec4 operator+(vec4 a, vec4 b) { return vec4(ops::add(a._v, b._v)); }
  friend vec4 operator-(vec4 a, vec4 b) { return vec4(ops::sub(a._v, b._v)); }
  friend vec4 operator*(vec4 a, vec4 b) { return vec4(ops::mul(a._v, b._v)); }

  friend vec4 operator+(vec4 a, T s) { return a + vec4(s); }
  friend vec4 operator-(vec4 a, T s) { return a - vec4(s); }
  friend vec4 operator*(vec4 a, T s) { return a * vec4(s); }

  friend vec4 operator+(T s, vec4 a) { return vec4(s) + a; }
  friend vec4 operator-(T s, vec4 a) { return vec4(s) - a; }
  friend vec4 operator*(T s, vec4 a) { return vec4(s) * a; }

  vec4& operator+=(vec4 rhs) { _v = ops::add(_v, rhs._v); return *this; }
  vec4& operator-=(vec4 rhs) { _v = ops::sub(_v, rhs._v); return *this; }
  vec4& operator*=(vec4 rhs) { _v = ops::mul(_v, rhs._v); return *this; }

  vec4& operator+=(T s) { return (*this += vec4(s)); }
  vec4& operator-=(T s) { return (*this -= vec4(s)); }
  vec4& operator*=(T s) { return (*this *= vec4(s)); }

  friend vec4 operator/(vec4 a, vec4 b) requires(lane_f32<T>) {
    return vec4(ops::div(a._v, b._v));
  }
  friend vec4 operator/(vec4 a, T s) requires(lane_f32<T>) { return a / vec4(s); }
  friend vec4 operator/(T s, vec4 a) requires(lane_f32<T>) { return vec4(s) / a; }

  vec4& operator/=(vec4 rhs) requires(lane_f32<T>) {
    _v = ops::div(_v, rhs._v);
    return *this;
  }
  vec4& operator/=(T s) requires(lane_f32<T>) { return (*this /= vec4(s)); }

  friend vec4 operator&(vec4 a, vec4 b) requires(lane_int<T>) {
    return vec4(wasm_v128_and(a._v, b._v));
  }
  friend vec4 operator|(vec4 a, vec4 b) requires(lane_int<T>) {
    return vec4(wasm_v128_or(a._v, b._v));
  }
  friend vec4 operator^(vec4 a, vec4 b) requires(lane_int<T>) {
    return vec4(wasm_v128_xor(a._v, b._v));
  }
  friend vec4 operator~(vec4 a) requires(lane_int<T>) {
    return vec4(wasm_v128_not(a._v));
  }

  vec4& operator&=(vec4 rhs) requires(lane_int<T>) {
    _v = wasm_v128_and(_v, rhs._v);
    return *this;
  }
  vec4& operator|=(vec4 rhs) requires(lane_int<T>) {
    _v = wasm_v128_or(_v, rhs._v);
    return *this;
  }
  vec4& operator^=(vec4 rhs) requires(lane_int<T>) {
    _v = wasm_v128_xor(_v, rhs._v);
    return *this;
  }

  friend vec4 operator<<(vec4 a, u32 s) requires(lane_int<T>) {
    return vec4(ops::shl(a._v, s));
  }
  friend vec4 operator>>(vec4 a, u32 s) requires(lane_int<T>) {
    return vec4(ops::shr(a._v, s));
  }

  vec4& operator<<=(u32 s) requires(lane_int<T>) {
    _v = ops::shl(_v, s);
    return *this;
  }
  vec4& operator>>=(u32 s) requires(lane_int<T>) {
    _v = ops::shr(_v, s);
    return *this;
  }

  friend vec4<bool> operator==(vec4 a, vec4 b) {
    return vec4<bool>(ops::eq(a._v, b._v));
  }
  friend vec4<bool> operator!=(vec4 a, vec4 b) {
    return vec4<bool>(ops::ne(a._v, b._v));
  }
  friend vec4<bool> operator<(vec4 a, vec4 b) {
    return vec4<bool>(ops::lt(a._v, b._v));
  }
  friend vec4<bool> operator<=(vec4 a, vec4 b) {
    return vec4<bool>(ops::le(a._v, b._v));
  }
  friend vec4<bool> operator>(vec4 a, vec4 b) {
    return vec4<bool>(ops::gt(a._v, b._v));
  }
  friend vec4<bool> operator>=(vec4 a, vec4 b) {
    return vec4<bool>(ops::ge(a._v, b._v));
  }

  friend vec4 select(vec4 a, vec4 b, vec4<bool> cond) {
    return vec4(wasm_v128_bitselect(b._v, a._v, cond._v));
  }
};

using vec4i = vec4<i32>;
using vec4u = vec4<u32>;
using vec4f = vec4<f32>;
using vec4b = vec4<bool>;

} // namespace wasm_vec4