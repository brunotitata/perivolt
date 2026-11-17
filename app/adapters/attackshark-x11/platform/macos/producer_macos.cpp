#if !defined(__APPLE__) || !defined(__MACH__)
#error This translation unit requires macOS and the Apple SDK.
#endif
#include "producer.hpp"
namespace perivolt::attackshark_x11::macos {
NativeSample poll_native_metadata(SessionToken token) {
    return from_discovery(discover_metadata(),token);
}
}
