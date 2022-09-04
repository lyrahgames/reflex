#pragma once
#include <libviewer/utility.hpp>

namespace viewer {

struct async_cio_state : future<string> {
  async_cio_state() = default;
  async_cio_state(future<string>&& f) : future<string>{move(f)} {}

  bool available() { return future_status::ready == wait_for(0s); }
};

inline mutex async_cio_mutex{};

inline auto async_line_read(string_view prompt = "\n>>> ") -> async_cio_state {
  string s{prompt};
  return async(launch::async, [prompt = move(s)]() {
    scoped_lock lock{async_cio_mutex};
    cout << prompt << flush;
    string line;
    getline(cin, line);
    return line;
  });
}

}  // namespace viewer
