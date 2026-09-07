#include "PlayerTargetState.hpp"
#include <cmath>

namespace ox {
bool validUpdateRate(int milliseconds) { return milliseconds >= 25 && milliseconds <= 5000; }
bool validHysteresis(float value) { return std::isfinite(value) && value >= 0.0f && value <= 5.0f; }
}
