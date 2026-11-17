#include "battery.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace perivolt;
int main() {
    int checks = 0;
    auto check = [&](bool ok) { ++checks; if (!ok) throw std::runtime_error("unit check " + std::to_string(checks)); };
    Battery core(60000);
    core.configure({20,5,true,true,true});
    auto view = [&] { return core.project("d","main",1000,"epoch"); };
    check(!view().present && !view().alert_eligible);
    Observation o{"d","main","source","source-epoch",1,1,{"unavailable",0,"not_reported"},"unknown","connected",900,"epoch"};
    check(core.observe(o));
    check(view().level.kind == "unavailable" && view().suppression == "unavailable");
    for (const double value : {-1.,101.,255.,1.5,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()}) {
        ++o.generation; o.level = {"reported_percent",value,""}; core.observe(o);
        check(view().level.kind == "invalid" && !view().alert_eligible);
    }
    ++o.generation; o.level = {"reported_percent",15,""}; core.observe(o);
    auto s = view(); auto n = core.evaluate(s);
    check(n.requested && !core.episode(s));
    core.delivery(s,"failed"); check(!core.episode(s));
    n = core.evaluate(s); check(n.requested);
    core.delivery(s,"unknown"); check(!core.episode(s));
    n = core.evaluate(s); core.delivery(s,"delivered"); check(core.episode(s));
    n = core.evaluate(s); check(!n.requested && n.blocked_reason == "already_notified");
    auto stale = core.project("d","main",60901,"epoch");
    core.evaluate(stale); check(core.episode(stale));
    ++o.generation; o.level = {"invalid",0,"out_of_range"}; core.observe(o);
    core.evaluate(view()); check(core.episode(view()));
    ++o.generation; o.level = {"reported_percent",15,""}; core.observe(o);
    check(!core.evaluate(view()).requested);
    check(!core.observe(o)); // equal generation does not replace state
    check(core.project("d","main",1000,"other-epoch").level_freshness == "unknown");
    core.restore_episode("d","main",false);
    core.configure({20,5,true,false,true});
    check(core.evaluate(view()).blocked_reason == "permission_denied");
    core.configure({20,5,false,true,true});
    check(core.evaluate(view()).blocked_reason == "alerts_disabled");
    core.configure({20,5,true,true,true});
    check(core.evaluate(view()).requested);
    core.restore_episode("other","main",true);
    check(!core.episode(view())); // device state isolation
    bool threw=false;
    try { core.set_threshold(0); } catch(const std::invalid_argument&) { threw=true; }
    check(threw);
    threw=false;
    try { ++o.generation; o.source="other-source"; core.observe(o); } catch(const std::invalid_argument&) { threw=true; }
    check(threw);
    std::cout << "DIRECT SYNTHETIC unit checks=" << checks << " PASS\n";
}
