#include "adapter.hpp"
#include <algorithm>
#include <iostream>
namespace m = perivolt::attackshark_x11::macos;
struct Fake {
    static constexpr bool synthetic_transport = true;
    std::array<std::uint8_t,5> fixture{3,0x55,0x40,1,0};
    m::ReadStatus status = m::ReadStatus::complete;
    std::size_t bytes = 5;
    unsigned reads = 0, closes = 0, cancels = 0;
    std::uint64_t clock = 0, delay = 0, requested_deadline = 0;
    bool pending = false, cancel_requested = false;
    std::uint64_t now_ms() const { return clock; }
    bool cancelled() const { return cancel_requested; }
    m::ReadResult read(std::span<std::uint8_t,5> output, std::uint64_t deadline) {
        ++reads; requested_deadline = deadline;
        clock += delay;
        if (status == m::ReadStatus::timeout) { pending=true; clock=deadline; return {status,0}; }
        std::copy(fixture.begin(),fixture.end(),output.begin());
        return {status,bytes};
    }
    void cancel_close() noexcept { ++closes; if(pending){++cancels;pending=false;} }
};
int main() {
    int checks=0, failures=0;
    auto check=[&](bool ok){++checks;if(!ok)++failures;};
    const m::EndpointMetadata endpoint{0x1d57,0xfa60,0x0a,0,5};
    const m::WallTime time{std::chrono::seconds{1234}};
    check(endpoint.candidate());
    check(m::native_read_gate({})==m::Reason::consent_required);
    check(m::native_read_gate({true,false})==m::Reason::native_validation_required);
    check(m::native_read_gate({true,true})==m::Reason::consent_required);
    for (int gate=0;gate<4;++gate) {
        Fake f; m::Consent consent{gate!=0,false}; auto ep=endpoint;
        if(gate==1) ep.usage=2;
        const auto r=m::run_fake_session(f,consent,ep,gate!=2,gate!=3,time);
        const m::Reason expected[]{m::Reason::consent_required,m::Reason::target_unverified,
                                  m::Reason::ambiguous_target,m::Reason::native_validation_required};
        check(!r.observation.percent && r.observation.reason==expected[gate] && f.reads==0 && !consent.consumed);
    }
    for(auto percent:{0,100}) {
        Fake f;f.fixture[4]=static_cast<std::uint8_t>(percent);m::Consent consent{true,false};
        const auto r=m::run_fake_session(f,consent,endpoint,true,true,time);
        check(r.observation.percent==percent && r.observation.origin==m::Origin::synthetic
            && r.observation.received_at==time && !r.observation.live
            && r.observation.protocol==perivolt::attackshark_x11::protocol_source
            && r.reports==1 && f.closes==1 && consent.consumed);
        const auto again=m::run_fake_session(f,consent,endpoint,true,true,time);
        check(!again.observation.percent && again.observation.reason==m::Reason::consent_required && f.reads==1);
        const auto cache=m::as_cache(r.observation,time+std::chrono::hours{1});
        check(cache.percent==percent && cache.origin==m::Origin::cache && !cache.live
            && cache.freshness==m::Freshness::unknown && cache.observed_at==r.observation.observed_at
            && cache.source==r.observation.source);
    }
    for(auto length:{0,1,2,3,4,6,64}) {
        Fake f;f.bytes=static_cast<std::size_t>(length);m::Consent c{true,false};
        const auto r=m::run_fake_session(f,c,endpoint,true,true,time);
        check(!r.observation.percent && r.observation.reason==m::Reason::malformed_report && f.reads==1 && f.closes==1);
    }
    for(int index:{0,1}) {
        Fake f;f.fixture[static_cast<std::size_t>(index)]=0;m::Consent c{true,false};
        const auto r=m::run_fake_session(f,c,endpoint,true,true,time);
        check(!r.observation.percent && r.observation.reason==m::Reason::malformed_report && f.reads==1);
    }
    for(auto event:{0x10,0x60,0x70,0x80}) {
        Fake f;f.fixture[2]=static_cast<std::uint8_t>(event);m::Consent c{true,false};
        const auto r=m::run_fake_session(f,c,endpoint,true,true,time);
        check(!r.observation.percent && r.observation.reason==m::Reason::report_limit && r.reports==32 && f.closes==1);
    }
    for(auto percent:{101,255}) {
        Fake f;f.fixture[4]=static_cast<std::uint8_t>(percent);m::Consent c{true,false};
        const auto r=m::run_fake_session(f,c,endpoint,true,true,time);
        check(!r.observation.percent && r.observation.reason==m::Reason::report_limit && r.reports==32);
    }
    for(auto status:{m::ReadStatus::timeout,m::ReadStatus::cancelled,m::ReadStatus::error}) {
        Fake f;f.status=status;m::Consent c{true,false};
        const auto r=m::run_fake_session(f,c,endpoint,true,true,time);
        const auto reason=status==m::ReadStatus::timeout?m::Reason::timeout:status==m::ReadStatus::cancelled?m::Reason::cancelled:m::Reason::io_error;
        check(!r.observation.percent && r.observation.reason==reason && f.closes==1 && !f.pending);
        if(status==m::ReadStatus::timeout)check(f.cancels==1 && f.requested_deadline==8000);
    }
    { Fake f; f.cancel_requested=true;m::Consent c{true,false}; const auto r=m::run_fake_session(f,c,endpoint,true,true,time);check(r.observation.reason==m::Reason::cancelled && f.reads==0 && f.closes==1); }
    { Fake f;f.delay=8000;m::Consent c{true,false};const auto r=m::run_fake_session(f,c,endpoint,true,true,time);check(!r.observation.percent && r.observation.reason==m::Reason::timeout && f.reads==1); }
    std::cout<<"SYNTHETIC macOS boundary (FAKE ONLY): "<<checks<<" checks, "<<failures<<" failures\n";
    return failures?1:0;
}
