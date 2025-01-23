#ifdef EXT_V_ENABLE
#pragma once

#include "arch.h"
#include "instr_trace.h"
#include <simobject.h>
#include "types.h"

using namespace vortex;

class VectorUnit : public SimObject<VectorUnit> {
public:
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

  VectorUnit(const SimContext& ctx,
             const char* name,
             const Arch &arch);

  ~VectorUnit();

  void reset();

  void tick();

  void load();

  void store();

  void execute();

  const PerfStats& perf_stats() const;

private:

  class Impl;
  Impl* impl_;
};
#endif