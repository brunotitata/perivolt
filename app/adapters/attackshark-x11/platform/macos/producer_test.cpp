#include "producer.hpp"
#include <iostream>
namespace m=perivolt::attackshark_x11::macos;
int main() {
    int checks=0, failures=0;
    auto check=[&](bool ok){++checks;if(!ok)++failures;};
    m::Discovery d; d.received_at=m::WallTime{std::chrono::seconds{42}};
    const m::EndpointMetadata candidate{0x1d57,0xfa60,0x0a,0,5};
    auto r=m::from_discovery(d,{});
    check(r.presence==m::MetadataPresence::unknown && !r.enumeration_complete);
    check(r.observation.received_at==d.received_at && !r.observation.observed_at);
    d.complete=true;
    r=m::from_discovery(d,{});
    check(r.presence==m::MetadataPresence::absent);
    d.endpoints.push_back(candidate);
    for (bool opt:{false,true}) {
        r=m::from_discovery(d,{7,opt});
        check(r.presence==m::MetadataPresence::candidate && r.candidate_count==1);
        check(r.token==m::SessionToken{7,opt});
        check(!r.read_available && !r.active && !r.observation.percent && !r.observation.live);
        check(r.observation.origin==m::Origin::metadata && r.observation.source==m::metadata_source);
        check(r.observation.reason==(opt?m::Reason::native_validation_required:m::Reason::consent_required));
        check(r.observation.charging==perivolt::attackshark_x11::Charging::unknown
            && r.observation.freshness==m::Freshness::unknown && !r.observation.observed_at);
    }
    d.endpoints.push_back(candidate);
    r=m::from_discovery(d,{8,true});
    check(r.presence==m::MetadataPresence::ambiguous && r.candidate_count==2);
    d.complete=false; d.os_error=-2;
    r=m::from_discovery(d,{8,true});
    check(r.presence==m::MetadataPresence::unknown && r.os_error==-2 && r.candidate_count==2);
    d.complete=true; d.os_error=0;d.endpoints={m::EndpointMetadata{0x1d57,0xfa60,1,2,5}};
    r=m::from_discovery(d,{});
    check(r.presence==m::MetadataPresence::unknown && r.family_endpoint_count==1 && r.candidate_count==0);
    d.endpoints={m::EndpointMetadata{0x1d57,0xfa60,0x0a,0,std::nullopt}};
    check(m::from_discovery(d,{}).candidate_count==0);
    check(m::accepts({2,true},{2,true}));
    check(!m::accepts({2,false},{2,true}));
    check(!m::accepts({3,true},{2,true}));
    check(!m::accepts({2,true},{2,false}));
    std::cout<<"SYNTHETIC metadata producer: "<<checks<<" checks, "<<failures<<" failures\n";
    return failures?1:0;
}
