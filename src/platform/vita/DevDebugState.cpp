#include "platform/vita/DevDebugState.h"

namespace {
    bool gCoordinatesHud = true; // default ON for development
}

namespace DevDebugState {
    bool coordinatesHudEnabled() { return gCoordinatesHud; }
    void setCoordinatesHudEnabled(bool enabled) { gCoordinatesHud = enabled; }
    void toggleCoordinatesHud() { gCoordinatesHud = !gCoordinatesHud; }
}
