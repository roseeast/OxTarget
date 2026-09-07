#include "ColAndreasAdapter.hpp"
#include "platform/samp/SampAdapter.hpp"

namespace ox {
std::optional<Intersection> ColAndreasAdapter::castRay(const Ray& ray) { return adapter_.worldRay(ray); }
}
