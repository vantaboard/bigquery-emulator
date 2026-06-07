#include "backend/engine/semantic/script/script_driver.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

ScriptDriver::ScriptDriver() = default;

ScriptDriver::ScriptDriver(FrameStack* external_variables)
    : external_variables_(external_variables) {}

ScriptDriver::~ScriptDriver() = default;

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
