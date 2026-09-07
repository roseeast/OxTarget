#pragma once
#include "Vec3.hpp"

namespace ox {
struct Ray { Vec3 origin; Vec3 direction; float maxDistance{}; };
}

