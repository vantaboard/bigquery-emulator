#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_PHASE_RECORDER_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_PHASE_RECORDER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "absl/time/time.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {

// PhaseRecorder collects per-query phase wall times in microseconds.
class PhaseRecorder {
 public:
  void Record(absl::string_view name, int64_t duration_us);
  void ToProto(bigquery_emulator::v1::PhaseTimings* out) const;

  class ScopedTimer {
   public:
    ScopedTimer(PhaseRecorder* recorder, absl::string_view name);
    ~ScopedTimer();

   private:
    PhaseRecorder* recorder_;
    std::string name_;
    absl::Time start_;
  };

 private:
  std::vector<std::pair<std::string, int64_t>> phases_;
};

using PhaseRecorderPtr = std::shared_ptr<PhaseRecorder>;

}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_PHASE_RECORDER_H_
