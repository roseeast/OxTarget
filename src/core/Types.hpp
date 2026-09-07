#pragma once

#include <cstdint>
#include <limits>
#include <string>

namespace ox {

using Handle = std::uint32_t;
constexpr Handle InvalidHandle = 0;

enum class HitType : std::uint8_t { None, World, Entity };
enum class EntityType : std::uint8_t {
    None, Player, Vehicle, Object, PlayerObject, Actor,
    DynamicObject, DynamicActor, DynamicPickup, Point
};
enum class ViewSource : std::uint8_t { Auto, Camera, Aim, Facing };

enum Mask : std::uint32_t {
    MaskWorld = 1u << 0u,
    MaskPlayer = 1u << 1u,
    MaskVehicle = 1u << 2u,
    MaskObject = 1u << 3u,
    MaskPlayerObject = 1u << 4u,
    MaskActor = 1u << 5u,
    MaskDynamicObject = 1u << 6u,
    MaskDynamicActor = 1u << 7u,
    MaskDynamicPickup = 1u << 8u,
    MaskAll = (1u << 9u) - 1u
};

inline constexpr std::uint32_t maskFor(EntityType type) {
    switch (type) {
        case EntityType::Player: return MaskPlayer;
        case EntityType::Vehicle: return MaskVehicle;
        case EntityType::Object: return MaskObject;
        case EntityType::PlayerObject: return MaskPlayerObject;
        case EntityType::Actor: return MaskActor;
        case EntityType::DynamicObject: return MaskDynamicObject;
        case EntityType::DynamicActor: return MaskDynamicActor;
        case EntityType::DynamicPickup: return MaskDynamicPickup;
        case EntityType::Point: return MaskObject;
        default: return 0;
    }
}

struct EntityKey {
    EntityType type{EntityType::None};
    int id{-1};
    int owner{-1};
};

enum Zone : std::uint8_t {
    ZoneAll = 0,
    ZoneFront = 1,
    ZoneRear = 2,
    ZoneSides = 3,
    ZoneLeft = 4,
    ZoneRight = 5,
    ZoneTop = 6,
    ZoneBottom = 7,
    // Aliases for vehicle semantics:
    ZoneHood = ZoneFront,
    ZoneTrunk = ZoneRear,
    ZoneDoors = ZoneSides,
    ZoneDoorDriver = ZoneLeft,
    ZoneDoorPassenger = ZoneRight
};
using VehicleZone = Zone;

struct Option {
    Handle target{InvalidHandle};
    std::string name;
    std::string icon;
    int data{};
    bool enabled{true};
    std::uint8_t zone{ZoneAll};
};

} // namespace ox

