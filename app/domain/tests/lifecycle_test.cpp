#include "battery.hpp"
#include <iostream>
#include <stdexcept>
using namespace perivolt;
int main() { try {
    int checks=0;
    auto check=[&](bool ok){ ++checks; if (!ok) throw std::runtime_error("lifecycle check " + std::to_string(checks)); };
    Battery core(60000); core.configure({20,5,true,true,true});
    core.register_device({"device","confirmed","receiver","1"});
    core.register_device({"other","confirmed","receiver","2"});
    Observation o{"device","main","source","a",1,1,{"reported_percent",15,""},"discharging","connected",900,"boot"};
    o.id="first"; o.endpoint="same-endpoint";
    auto view=[&]{return core.project("device","main",1000,"boot");};
    core.observe(o); check(core.evaluate(view()).requested); core.delivery(view(),"delivered");
    auto other=o; other.device="other"; other.level.percent=80; core.observe(other);
    auto component=o; component.component="secondary"; component.level.percent=70; core.observe(component);
    check(core.remove("source","device","main",2,"a")); check(!view().present);
    core.delivery(view(),"delivered"); check(core.episode(view()));
    o.generation=1; o.id="older"; check(!core.observe(o));
    o.generation=2; o.id="equal"; check(!core.observe(o)); check(!view().present);
    check(core.project("other","main",1000,"boot").level.percent==80);
    check(core.project("device","secondary",1000,"boot").level.percent==70);
    o.generation=3; check(core.observe(o)); check(view().present);
    check(!core.evaluate(view()).requested); // removal/reconnect never rearms
    check(!core.remove("source","device","main",2,"a")); check(view().present);
    core.change_source_epoch("source","a","b");
    check(view().level_freshness=="unknown");
    o.generation=100; check(!core.observe(o)); // delayed retired epoch cannot win
    o.source_epoch="b"; o.generation=0; check(core.observe(o)); check(view().alert_eligible);
    check(!core.remove("source","device","main",200,"a")); check(view().present);
    core.sleep(1100); check(view().suppression=="sleep_invalidated");
    core.resume(1200); check(view().suppression=="sleep_invalidated");
    ++o.generation; o.observed_ms=1000; check(core.observe(o));
    check(view().suppression=="sleep_invalidated"); // higher generation does not freshen pre-sleep cache
    ++o.generation; o.observed_ms=1300; check(core.observe(o));
    auto fresh=core.project("device","main",1400,"boot"); check(fresh.alert_eligible);
    check(core.episode(fresh)); check(!core.evaluate(fresh).requested);
    bool threw=false;
    try { core.change_source_epoch("source","a","c"); } catch(const std::invalid_argument&) { threw=true; }
    check(threw); check(core.project("device","main",1400,"boot").alert_eligible);
    // tombstone exists even if removal arrives before first observation
    Battery early(60000); check(early.remove("s","d","c",5,"epoch"));
    auto late=o; late.source="s"; late.device="d"; late.component="c"; late.source_epoch="epoch"; late.generation=5;
    check(!early.observe(late)); check(!early.project("d","c",1400,"boot").present);
    late.generation=6; check(early.observe(late));
    // Unknown epoch must not poison a previously unseen component.
    auto unknown=late; unknown.component="new"; unknown.source_epoch="wrong";
    check(!early.observe(unknown));
    unknown.source_epoch="epoch"; check(early.observe(unknown));
    std::cout << "LIFECYCLE SYNTHETIC checks=" << checks << " PASS\n";
} catch(const std::exception& error) { std::cerr << error.what() << "\n"; return 1; }
}
