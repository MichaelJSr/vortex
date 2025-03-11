#ifdef EXT_V_ENABLE
#pragma once

#include "arch.h"
#include "instr.h"
#include "instr_trace.h"
#include <simobject.h>
#include "types.h"

namespace vortex {

class VecUnit : public SimObject<VecUnit> {
public:
  struct TraceData : public ITraceData {
    using Ptr = std::shared_ptr<TraceData>;
    std::vector<std::vector<mem_addr_size_t>> mem_addrs;
    TraceData(uint32_t num_lanes) : mem_addrs(num_lanes) {
        for (uint32_t i = 0; i < num_lanes; ++i) {
            mem_addrs.at(i).reserve(4);
        }
    }
  };

  struct PerfStats {
    uint64_t reads;
    uint64_t writes;
    uint64_t latency;
    uint64_t stalls;

    PerfStats()
      : reads(0)
      , writes(0)
      , latency(0)
      , stalls(0)
    {}

    PerfStats& operator+=(const PerfStats& rhs) {
      this->reads   += rhs.reads;
      this->writes  += rhs.writes;
      this->latency += rhs.latency;
      this->stalls  += rhs.stalls;
      return *this;
    }
  };

  std::vector<SimPort<MemReq>> MemReqs;
  std::vector<SimPort<MemRsp>> MemRsps;

  SimPort<instr_trace_t*> Input;
  SimPort<instr_trace_t*> Output;

  VecUnit(const SimContext& ctx,
          const char* name,
          const Arch &arch);

  ~VecUnit();

  void reset();

  void tick();

  void load(const Instr &instr, uint32_t wid, std::vector<reg_data_t[3]> &rsdata);

  void store(const Instr &instr, uint32_t wid, std::vector<reg_data_t[3]> &rsdata);

  void execute(const Instr &instr, uint32_t wid, std::vector<reg_data_t[3]> &rsdata, std::vector<reg_data_t> &rddata);

  const PerfStats& perf_stats() const;

private:

  class Impl;
  Impl* impl_;
};

}
#endif