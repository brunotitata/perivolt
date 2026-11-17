#include "battery.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace perivolt;
int main() { try {
    int checks=0;
    auto check=[&](bool value){++checks;if(!value)throw std::runtime_error("source/time check "+std::to_string(checks));};
    Battery core(100); core.configure({20,5,true,true,true});
    Observation o{"d","main","s","a",1,1,{"reported_percent",15,""},"discharging","connected",900,"boot"};
    auto view=[&]{return core.project("d","main",1000,"boot");};
    core.observe(o); check(core.evaluate(view()).requested); core.delivery(view(),"delivered");
    core.source_permission("s","denied"); check(!view().alert_eligible); check(core.episode(view()));
    core.source_permission("s","granted"); check(!view().alert_eligible); // grant does not renew cached reading
    ++o.generation; core.observe(o); check(view().alert_eligible); check(!core.evaluate(view()).requested);
    // permission of another source never invalidates this source
    core.source_permission("other","denied"); check(view().alert_eligible);
    // Version gate precedes range / non-finite / declared invalid diagnostics.
    for (const double value : {-1.,255.,std::numeric_limits<double>::quiet_NaN()}) {
        ++o.generation; o.payload_version=2; o.level={"reported_percent",value,""}; core.observe(o);
        check(view().level.kind=="invalid" && view().level.detail=="unsupported_version");
        check(view().charging=="unknown" && view().connection=="unknown");
        check(!view().alert_eligible);
    }
    ++o.generation; o.level={"invalid",0,"out_of_range"}; core.observe(o); check(view().level.detail=="unsupported_version");
    // Rejected generation must not overwrite current diagnostic even with incompatible payload.
    auto rejected=o; rejected.payload_version=1; rejected.level={"reported_percent",15,""}; check(!core.observe(rejected));
    check(view().level.detail=="unsupported_version");
    ++o.generation; o.payload_version=1; o.level={"invalid",0,"non_finite"}; core.observe(o); check(view().level.detail=="non_finite");
    // Temporal bases remain separate; explicit connection time is NOT battery time.
    ++o.generation; o.level={"reported_percent",15,""}; o.charging="discharging";o.connection="connected";
    o.observed_ms=std::nullopt; o.connection_observed_ms=950; core.observe(o);
    check(view().level_freshness=="unknown" && view().connection_freshness=="fresh");
    check(!view().alert_eligible);
    check(core.project("d","main",1051,"boot").connection_freshness=="stale");
    check(core.project("d","main",1000,"other-epoch").connection_freshness=="unknown");
    ++o.generation; o.connection_observed_ms=std::nullopt; core.observe(o);
    check(view().connection_freshness=="unknown"); // received time alone is never synthesized
    ++o.generation; o.observed_ms=900; o.charging_observed_ms=800;core.observe(o);
    check(view().level_freshness=="fresh" && view().charging_freshness=="stale");
    core.remove("s","d","main",++o.generation,"a");
    auto n=core.evaluate(view()); check(!n.requested && !n.rearm_applied && n.blocked_reason=="ineligible"); check(core.episode(view()));
    // Each field must survive sleep invalidation independently.
    Battery fields(100); fields.sleep(900); fields.resume(1000);
    auto independent=o; independent.source="fields"; independent.observed_ms=1100;
    independent.charging_observed_ms=1150; independent.connection_observed_ms=950;
    fields.observe(independent);
    auto fs=fields.project("d","main",1200,"boot");
    check(fs.level_freshness=="fresh" && fs.charging_freshness=="fresh");
    check(fs.connection_freshness=="unknown" && !fs.alert_eligible);
    bool threw=false;try{core.source_permission("s","guess");}catch(const std::invalid_argument&){threw=true;}check(threw);
    std::cout << "SOURCE/TIME SYNTHETIC checks=" << checks << " PASS\n";
    return 0;
} catch(const std::exception& e) {std::cerr << e.what() << "\n";return 1;} }
