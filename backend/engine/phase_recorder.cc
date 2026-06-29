#include "backend/engine/phase_recorder.h"

#include <cstdint>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {

void PhaseRecorder::Record(absl::string_view name, int64_t duration_us) {
  phases_.emplace_back(std::string(name), duration_us);
}

void PhaseRecorder::ToProto(bigquery_emulator::v1::PhaseTimings* out) const {
  if (out == nullptr) {
    return;
  }
  out->clear_phases();
  for (const auto& [name, us] : phases_) {
    auto* phase = out->add_phases();
    phase->set_name(name);
    phase->set_duration_us(us);
  }
}

PhaseRecorder::ScopedTimer::ScopedTimer(PhaseRecorder* recorder,
                                        absl::string_view name)
    : recorder_(recorder), name_(name), start_(absl::Now()) {}

PhaseRecorder::ScopedTimer::~ScopedTimer() {
  if (recorder_ == nullptr) {
    return;
  }
  const absl::Duration elapsed = absl::Now() - start_;
  recorder_->Record(name_, absl::ToInt64Microseconds(elapsed));
}

}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
