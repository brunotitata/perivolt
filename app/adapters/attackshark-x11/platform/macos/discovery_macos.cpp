#if !defined(__APPLE__) || !defined(__MACH__)
#error This translation unit requires the Apple macOS SDK; do not build on Windows.
#endif
#include "adapter.hpp"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <limits>

namespace perivolt::attackshark_x11::macos {
namespace {
struct Object {
    io_object_t value = IO_OBJECT_NULL;
    explicit Object(io_object_t object=IO_OBJECT_NULL):value(object){}
    Object(const Object&)=delete;
    Object& operator=(const Object&)=delete;
    ~Object(){if(value!=IO_OBJECT_NULL)IOObjectRelease(value);}
};
std::optional<std::uint32_t> number(io_registry_entry_t service, CFStringRef key) {
    CFTypeRef property=IORegistryEntryCreateCFProperty(service,key,kCFAllocatorDefault,0);
    if(!property)return {};
    std::int64_t value=0;
    const bool ok=CFGetTypeID(property)==CFNumberGetTypeID()
        && CFNumberGetValue(static_cast<CFNumberRef>(property),kCFNumberSInt64Type,&value);
    CFRelease(property);
    if(!ok || value<0 || value>std::numeric_limits<std::uint32_t>::max())return {};
    return static_cast<std::uint32_t>(value);
}
}
Discovery discover_metadata() {
    Discovery result;
    result.received_at=std::chrono::system_clock::now();
    auto matching=IOServiceMatching("IOHIDDevice");
    if(!matching){result.os_error=-1;return result;}
    io_iterator_t iterator=IO_OBJECT_NULL;
    // API consumes matching. Registry enumeration only; no IOServiceOpen,
    // IOHIDDeviceOpen, callbacks, reports, event taps, runloops or notifications.
    const auto status=IOServiceGetMatchingServices(kIOMainPortDefault,matching,&iterator);
    if(status!=KERN_SUCCESS){result.os_error=status;return result;}
    Object owned_iterator(iterator);
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds{2};
    for(unsigned i=0;i<512;++i) {
        if(std::chrono::steady_clock::now()>=deadline){result.os_error=-2;return result;}
        Object service(IOIteratorNext(iterator));
        if(service.value==IO_OBJECT_NULL){result.complete=IOIteratorIsValid(iterator);return result;}
        const auto vendor=number(service.value,CFSTR(kIOHIDVendorIDKey));
        const auto product=number(service.value,CFSTR(kIOHIDProductIDKey));
        if(vendor!=0x1d57 || product!=0xfa60)continue;
        result.endpoints.push_back({vendor,product,
            number(service.value,CFSTR(kIOHIDPrimaryUsagePageKey)),
            number(service.value,CFSTR(kIOHIDPrimaryUsageKey)),
            number(service.value,CFSTR(kIOHIDMaxInputReportSizeKey))});
    }
    result.os_error=-3; // bounded enumeration stopped; do not claim absence/uniqueness
    return result;
}
}
