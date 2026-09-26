#pragma once

#include "shadowrun/native_port.hpp"

namespace shadowrun::native {

// NativePortShell is the historical name of the controller that already owns
// native scene state, input transitions, and software-frame composition.  The
// runtime boundary is now named after what it is: the native game controller.
// Keep the alias while the source files and focused tests are migrated by
// subsystem; this header prevents frontends from growing another state owner.
using NativeGameController = NativePortShell;

} // namespace shadowrun::native
