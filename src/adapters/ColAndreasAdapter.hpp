#pragma once
#include "core/CollisionBackend.hpp"

namespace ox {
class SampAdapter;
class ColAndreasAdapter final : public ICollisionBackend {
public:
    explicit ColAndreasAdapter(SampAdapter& adapter) : adapter_(adapter) {}
    std::optional<Intersection> castRay(const Ray& ray) override;
    std::string_view name() const override { return "ColAndreas Pawn bridge (optional)"; }
private:
    SampAdapter& adapter_;
};
}
