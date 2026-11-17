#include "monitor.hpp"
#include <cassert>
#include <iostream>
using namespace perivolt;
using namespace perivolt::application;
int main() {
    Monitor m(100);
    assert(m.snapshot(0,"boot").devices.empty());
    assert(m.snapshot(0,"boot").control.consent==ui::Consent::off);
    m.source_status(true,ui::Permission::granted);
    auto token=m.request_read(true);
    Sample a; a.identity={"device","confirmed","",""}; a.name="Test injected only";
    a.observation.device="device"; a.observation.component="battery";
    a.observation.source="test.injected"; a.observation.source_epoch="session";
    a.observation.generation=1; a.observation.level={"reported_percent",0,""};
    a.observation.observed_ms=10; a.observation.monotonic_epoch="boot";
    a.observation.connection="connected"; a.origin=SampleOrigin::session_observation;
    assert(m.submit(token,a));
    auto s=m.snapshot(20,"boot"); assert(s.devices.size()==1);
    assert(s.devices[0].state.level.percent==0 && s.devices[0].state.level.kind=="reported_percent");
    assert(s.devices[0].age_ms==10 && s.devices[0].source=="test.injected");
    assert(s.devices[0].state.identity_confidence=="confirmed");
    assert(!s.control.active);
    assert(m.snapshot(111,"boot").devices[0].state.level_freshness=="stale");
    assert(m.snapshot(111,"boot").devices[0].age_ms==101);
    assert(!m.snapshot(20,"different").devices[0].age_ms);
    assert(!m.submit(token,a)); // duplicate does not replace age
    m.cancel(); assert(m.snapshot(20,"boot").devices.empty());
    auto next=m.request_read(true); assert(!m.submit(token,a));
    a.observation.generation=2; a.origin=SampleOrigin::synthetic;
    assert(!m.submit(next,a)); assert(m.snapshot(20,"boot").devices.empty());
    a.origin=SampleOrigin::session_observation;
    a.observation.level={"unavailable",0,"not_reported"};
    a.observation.observed_ms.reset(); assert(m.submit(next,a));
    s=m.snapshot(30,"boot"); assert(s.devices[0].state.level.kind=="unavailable");
    assert(!s.devices[0].age_ms && !s.devices[0].state.alert_eligible);
    m.source_status(true,ui::Permission::denied);
    assert(m.snapshot(30,"boot").devices.empty());
    assert(!m.submit(next,a));
    m.source_status(true,ui::Permission::granted); next=m.request_read(true);
    a.observation.generation=3; a.observation.connection="disconnected";
    a.observation.level={"reported_percent",0,""}; a.observation.observed_ms=30;
    assert(m.submit(next,a)); assert(!m.snapshot(30,"boot").devices[0].state.alert_eligible);
    m.shutdown(); assert(!m.submit(next,a)); assert(!m.request_read(true).opt_in);
    // Real Hardware mapping exercised with synthetic Discovery inputs; no native call.
    namespace mac=perivolt::attackshark_x11::macos;
    Monitor metadata_monitor;
    mac::Discovery discovery; discovery.complete=true;
    auto native=mac::from_discovery(discovery,{});
    assert(native.presence==mac::MetadataPresence::absent);
    assert(metadata_monitor.submit_metadata(native));
    assert(metadata_monitor.metadata()->enumeration_complete);
    assert(metadata_monitor.snapshot(0,"boot").devices.empty());
    discovery.endpoints.push_back({0x1d57,0xfa60,0x0a,0,5});
    native=mac::from_discovery(discovery,{});
    assert(native.presence==mac::MetadataPresence::candidate);
    // Defensively reject any percent embedded in metadata as live.
    native.observation.percent=50; native.observation.live=true; native.read_available=true;
    assert(metadata_monitor.submit_metadata(native));
    assert(!metadata_monitor.metadata()->observation.percent);
    assert(!metadata_monitor.metadata()->observation.live);
    assert(!metadata_monitor.snapshot(0,"boot").control.capability_available);
    assert(metadata_monitor.snapshot(0,"boot").devices.empty());
    auto request=metadata_monitor.request_read(true);
    assert(!metadata_monitor.submit_metadata(native));
    native=mac::from_discovery(discovery,{request.generation,request.opt_in});
    assert(metadata_monitor.submit_metadata(native));
    discovery.endpoints.push_back(discovery.endpoints[0]);
    native=mac::from_discovery(discovery,{request.generation,request.opt_in});
    assert(metadata_monitor.submit_metadata(native));
    assert(metadata_monitor.metadata()->presence==mac::MetadataPresence::ambiguous);
    metadata_monitor.cancel(); auto new_request=metadata_monitor.request_read(true);
    assert(!metadata_monitor.submit_metadata(native));
    discovery.complete=false; discovery.os_error=-1;
    native=mac::from_discovery(discovery,{new_request.generation,new_request.opt_in});
    assert(metadata_monitor.submit_metadata(native));
    assert(metadata_monitor.metadata()->presence==mac::MetadataPresence::unknown);
    assert(metadata_monitor.metadata()->os_error==-1);
    assert(metadata_monitor.snapshot(0,"boot").devices.empty());
    metadata_monitor.shutdown(); assert(!metadata_monitor.submit_metadata(native));
    std::cout<<"application integration SYNTHETIC PASS; no HID\n";
}
