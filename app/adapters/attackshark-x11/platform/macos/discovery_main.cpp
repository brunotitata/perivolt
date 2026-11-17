#include "adapter.hpp"
#include <iostream>
namespace m=perivolt::attackshark_x11::macos;
namespace {
void numeric(std::optional<std::uint32_t> value){if(value)std::cout<<*value;else std::cout<<"null";}
}
int main(int argc,char**) {
    // No report-read mode, no flag that enables reading, no consent prompt here.
    if(argc!=1)return 2;
    const auto result=m::discover_metadata();
    const auto received=std::chrono::duration_cast<std::chrono::milliseconds>(result.received_at.time_since_epoch()).count();
    std::cout<<"{\"source\":\""<<m::metadata_source<<"\",\"received_wall_unix_ms\":"<<received
        <<",\"observed_wall_unix_ms\":null,\"percent\":null,\"charging\":\"unknown\","
          "\"freshness\":\"unknown\",\"live\":false,\"report_read_enabled\":false,"
          "\"reason\":\"native_validation_required\",\"complete\":"<<(result.complete?"true":"false")
        <<",\"os_error\":"<<result.os_error<<",\"endpoints\":[";
    bool first=true;
    for(const auto& e:result.endpoints){
        if(!first)std::cout<<',';
        first=false;
        std::cout<<"{\"vid\":";numeric(e.vendor);std::cout<<",\"pid\":";numeric(e.product);
        std::cout<<",\"usage_page\":";numeric(e.usage_page);std::cout<<",\"usage\":";numeric(e.usage);
        std::cout<<",\"max_input_bytes\":";numeric(e.max_input_bytes);
        std::cout<<",\"metadata_candidate_only\":"<<(e.candidate()?"true":"false")<<'}';
    }
    std::cout<<"]}\n";
    return result.complete?0:2;
}
