#pragma once

#include <cstdint>

namespace RPGEngine {
namespace Tilemap {

/**
 * Tile flags
 */
enum TileFlags : uint32_t {
    None        = 0,
    Solid       = 1 << 0,  // Tile is solid (blocks movement)
    Animated    = 1 << 1,  // Tile is animated
    Flipped_H   = 1 << 2,  // Tile is horizontally flipped
    Flipped_V   = 1 << 3,  // Tile is vertically flipped
    Rotated_90  = 1 << 4,  // Tile is rotated 90 degrees
    Rotated_180 = 1 << 5,  // Tile is rotated 180 degrees
    Rotated_270 = 1 << 6,  // Tile is rotated 270 degrees
    Trigger     = 1 << 7,  // Tile is a trigger
    Water       = 1 << 8,  // Tile is water
    Lava        = 1 << 9,  // Tile is lava
    Damage      = 1 << 10, // Tile causes damage
    Heal        = 1 << 11, // Tile heals
    Slippery    = 1 << 12, // Tile is slippery
    Slow        = 1 << 13, // Tile slows movement
    Fast        = 1 << 14, // Tile speeds up movement
    Custom1     = 1 << 15, // Custom flag 1
    Custom2     = 1 << 16, // Custom flag 2
    Custom3     = 1 << 17, // Custom flag 3
    Custom4     = 1 << 18  // Custom flag 4
};

/**
 * Tile structure
 * Represents a single tile in a tilemap
 */
struct Tile {
    uint32_t id;       // Tile ID
    uint32_t flags;    // Tile flags
    
    Tile() : id(0), flags(TileFlags::None) {}
    Tile(uint32_t id, uint32_t flags = TileFlags::None) : id(id), flags(flags) {}
    
    bool isSolid() const { return (flags & TileFlags::Solid) != 0; }
    bool isAnimated() const { return (flags & TileFlags::Animated) != 0; }
    bool isFlippedH() const { return (flags & TileFlags::Flipped_H) != 0; }
    bool isFlippedV() const { return (flags & TileFlags::Flipped_V) != 0; }
    bool isRotated90() const { return (flags & TileFlags::Rotated_90) != 0; }
    bool isRotated180() const { return (flags & TileFlags::Rotated_180) != 0; }
    bool isRotated270() const { return (flags & TileFlags::Rotated_270) != 0; }
    bool isTrigger() const { return (flags & TileFlags::Trigger) != 0; }
    bool isWater() const { return (flags & TileFlags::Water) != 0; }
    bool isLava() const { return (flags & TileFlags::Lava) != 0; }
    bool isDamage() const { return (flags & TileFlags::Damage) != 0; }
    bool isHeal() const { return (flags & TileFlags::Heal) != 0; }
    bool isSlippery() const { return (flags & TileFlags::Slippery) != 0; }
    bool isSlow() const { return (flags & TileFlags::Slow) != 0; }
    bool isFast() const { return (flags & TileFlags::Fast) != 0; }
    
    void setFlag(TileFlags flag, bool value) {
        if (value) {
            flags |= flag;
        } else {
            flags &= ~flag;
        }
    }
};

} // namespace Tilemap
} // namespace RPGEngine