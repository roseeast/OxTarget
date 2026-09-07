#pragma once

#include "Math/AABB.hpp"
#include "Types.hpp"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ox {

class SpatialIndex {
public:
    explicit SpatialIndex(float cellSize = 16.0f);
    void upsert(Handle handle, const AABB& bounds);
    void remove(Handle handle);
    void clear();
    std::vector<Handle> query(const AABB& area) const;
    float cellSize() const { return cellSize_; }

private:
    struct Cell { int x{}, y{}, z{}; bool operator==(const Cell& o) const { return x == o.x && y == o.y && z == o.z; } };
    struct Hash { std::size_t operator()(const Cell& c) const; };
    std::vector<Cell> cellsFor(const AABB& box) const;
    float cellSize_;
    std::unordered_map<Cell, std::vector<Handle>, Hash> cells_;
    std::unordered_map<Handle, std::vector<Cell>> memberships_;
};

} // namespace ox

