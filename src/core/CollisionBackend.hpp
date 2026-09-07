#pragma once

#include "Intersection.hpp"
#include <optional>
#include <string_view>

namespace ox {
class ICollisionBackend {
public:
    virtual ~ICollisionBackend() = default;
    virtual std::optional<Intersection> castRay(const Ray& ray) = 0;
    virtual std::string_view name() const = 0;
};

class FallbackCollisionBackend final : public ICollisionBackend {
public:
    std::optional<Intersection> castRay(const Ray&) override { return std::nullopt; }
    std::string_view name() const override { return "Fallback (entity bounds only)"; }
};
}

