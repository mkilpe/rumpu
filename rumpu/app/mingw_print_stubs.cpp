// Stub implementations of libstdc++ internal helpers for std::print(ostream&, ...)
// on Windows. The Fedora mingw64 cross-compiler package omits these from libstdc++.a.
// Returning nullptr from __open_terminal means "not a terminal", so vprint_unicode
// falls through to ordinary ostream write — which is the correct behaviour here.
#include <streambuf>
#include <span>
#include <system_error>

namespace std {
    void* __open_terminal(basic_streambuf<char, char_traits<char>>*) {
        return nullptr;
    }
    error_code __write_to_terminal(void*, span<char>) {
        return {};
    }
}
