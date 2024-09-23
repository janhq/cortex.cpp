#pragma once

namespace utils {
template <typename F>
struct ScopeExit {
  ScopeExit(F&& f) : f_(std::forward<F>(f)) {}
  ~ScopeExit() { f_(); }
  F f_;
};

template <typename F>
ScopeExit<F> makeScopeExit(F&& f) {
  return ScopeExit<F>(std::forward<F>(f));
};
}  // namespace utils