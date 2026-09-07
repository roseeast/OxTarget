#include "SpatialIndex.hpp"
#include <algorithm>
#include <cmath>

namespace ox {

SpatialIndex::SpatialIndex(float cellSize) : cellSize_(cellSize > 0.0f ? cellSize : 16.0f) {}

std::size_t SpatialIndex::Hash::operator()(const Cell& c) const {
    std::size_t h = static_cast<std::size_t>(static_cast<std::uint32_t>(c.x) * 73856093u);
    h ^= static_cast<std::size_t>(static_cast<std::uint32_t>(c.y) * 19349663u);
    h ^= static_cast<std::size_t>(static_cast<std::uint32_t>(c.z) * 83492791u);
    return h;
}

std::vector<SpatialIndex::Cell> SpatialIndex::cellsFor(const AABB& box) const {
    if (!box.valid()) return {};
    auto coord = [this](float v) { return static_cast<int>(std::floor(v / cellSize_)); };
    const Cell lo{coord(box.min.x), coord(box.min.y), coord(box.min.z)};
    const Cell hi{coord(box.max.x), coord(box.max.y), coord(box.max.z)};
    std::vector<Cell> out;
    const std::int64_t count = static_cast<std::int64_t>(hi.x - lo.x + 1) *
                               static_cast<std::int64_t>(hi.y - lo.y + 1) *
                               static_cast<std::int64_t>(hi.z - lo.z + 1);
    if (count <= 0 || count > 100000) return out;
    out.reserve(static_cast<std::size_t>(count));
    for (int z = lo.z; z <= hi.z; ++z)
        for (int y = lo.y; y <= hi.y; ++y)
            for (int x = lo.x; x <= hi.x; ++x) out.push_back({x, y, z});
    return out;
}

void SpatialIndex::upsert(Handle handle, const AABB& bounds) {
    remove(handle);
    auto occupied = cellsFor(bounds);
    if (occupied.empty()) return;
    for (const Cell& cell : occupied) cells_[cell].push_back(handle);
    memberships_.emplace(handle, std::move(occupied));
}

void SpatialIndex::remove(Handle handle) {
    const auto found = memberships_.find(handle);
    if (found == memberships_.end()) return;
    for (const Cell& cell : found->second) {
        auto bucket = cells_.find(cell);
        if (bucket == cells_.end()) continue;
        auto& values = bucket->second;
        values.erase(std::remove(values.begin(), values.end(), handle), values.end());
        if (values.empty()) cells_.erase(bucket);
    }
    memberships_.erase(found);
}

void SpatialIndex::clear() { cells_.clear(); memberships_.clear(); }

std::vector<Handle> SpatialIndex::query(const AABB& area) const {
    std::unordered_set<Handle> unique;
    for (const Cell& cell : cellsFor(area)) {
        const auto found = cells_.find(cell);
        if (found != cells_.end()) unique.insert(found->second.begin(), found->second.end());
    }
    return {unique.begin(), unique.end()};
}

} // namespace ox

