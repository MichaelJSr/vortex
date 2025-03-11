#ifdef EXT_V_ENABLE

#include "vec_unit.h"
#include "emulator.h"

using namespace vortex;

class VecUnit::Impl {
public:
    Impl(VecUnit* simobject, const Arch& /*arch*/)
        : simobject_(simobject)
        , num_lanes_(1) // Should the vec_unit have more than 1 lane?
        , pending_reqs_(1)
    {
        this->clear();
    }

    ~Impl() {}

    void clear() {
        pending_reqs_.clear();
        perf_stats_ = PerfStats();
    }

    void tick() {
        // Handle memory response
        for (uint32_t t = 0; t < num_lanes_; ++t) {
            auto& mem_rsp_port = simobject_->MemRsps.at(t);
            if (mem_rsp_port.empty())
                continue;
            
            auto& mem_rsp = mem_rsp_port.front();
            auto& entry = pending_reqs_.at(mem_rsp.tag);
            auto trace = entry.trace;
            
            assert(entry.count);
            --entry.count;
            if (0 == entry.count) {
                simobject_->Output.push(trace, (vl_ / num_lanes_) * 3);
                pending_reqs_.release(mem_rsp.tag);
            }
            mem_rsp_port.pop();
        }
        
        for (int i = 0, n = pending_reqs_.size(); i < n; ++i) {
            if (pending_reqs_.contains(i))
                perf_stats_.latency += pending_reqs_.at(i).count;
        }
        
        if (simobject_->Input.empty())
            return;
        
        auto trace = simobject_->Input.front();
        
        if (pending_reqs_.full()) {
            if (!trace->log_once(true)) {
                DT(3, "*** VecUnit queue stall: " << *trace);
            }
            ++perf_stats_.stalls;
            return;
        } else {
            trace->log_once(false);
        }
        
        auto trace_data = std::dynamic_pointer_cast<TraceData>(trace->data);
        uint32_t addr_count = 0;
        for (auto& mem_addr : trace_data->mem_addrs) {
            addr_count += mem_addr.size();
        }
        
        if (addr_count != 0) {
            auto tag = pending_reqs_.allocate({trace, addr_count});
            for (uint32_t t = 0; t < num_lanes_; ++t) {
                if (!trace->tmask.test(t))
                    continue;
                
                auto& mem_req_port = simobject_->MemReqs.at(t);
                for (auto& mem_addr : trace_data->mem_addrs.at(t)) {
                    MemReq mem_req;
                    mem_req.addr  = mem_addr.addr;
                    mem_req.write = (trace->lsu_type == LsuType::STORE);
                    mem_req.tag   = tag;
                    mem_req.cid   = trace->cid;
                    mem_req.uuid  = trace->uuid;
                    mem_req_port.push(mem_req, (vl_ / num_lanes_));
                    DT(3, "VecUnit mem-req: addr=0x" << std::hex << mem_addr.addr << ", tag=" << tag << ", tid=" << t << ", " << trace);
                    ++perf_stats_.reads;
                }
            }
        } else {
            simobject_->Output.push(trace, 1);
        }
        
        simobject_->Input.pop();
    }

    const PerfStats& perf_stats() const {
        return perf_stats_;
    }

private:

    struct pending_req_t {
        instr_trace_t* trace;
        uint32_t count;
    };

    VecUnit* simobject_;
    std::vector<std::vector<Byte>>  vreg_file_;
    vtype_t                         vtype_;
    uint32_t                        vl_;
    Word                            vlmax_;
    uint32_t                        num_lanes_;
    HashTable<pending_req_t>        pending_reqs_;
    PerfStats                       perf_stats_;
};

VecUnit::VecUnit(const SimContext& ctx,
                 const char* name,
                 const Arch &arch)
    : SimObject<VecUnit>(ctx, name)
    , MemReqs(1, this)
    , MemRsps(1, this)
    , Input(this)
    , Output(this)
    , impl_(new Impl(this, arch))
{}

VecUnit::~VecUnit() {
    delete impl_;
}

void VecUnit::reset() {
    impl_->clear();
}

void VecUnit::tick() {
    impl_->tick();
}

const VecUnit::PerfStats& VecUnit::perf_stats() const {
    return impl_->perf_stats();
}
#endif