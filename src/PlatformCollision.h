#pragma once
#include "MathTypes.h"
#include "CollisionBox.h"
#include "PhysicsBody.h"
#include <vector>

// Platform types
enum class PlatformType {
    SOLID,          // Standard solid platform (blocks from all sides)
    ONE_WAY,        // Can pass through from below, solid from above
    MOVING          // Platform that moves (can carry entities)
};

// Platform collision component
class Platform {
public:
    Platform();
    Platform(PlatformType type, const CollisionBox& bounds);
    ~Platform();
    
    // Platform properties
    PlatformType type;
    CollisionBox bounds;
    Vector2 position;
    
    // For moving platforms
    Vector2 velocity;           // Current movement velocity
    bool isMoving;              // Whether platform is currently moving
    
    // Check if an entity can collide with this platform
    bool CanCollideWith(const Vector2& entityPos, const Vector2& entityVelocity, const CollisionBox& entityBounds) const;
    
    // Resolve collision with an entity
    void ResolveCollision(Vector2& entityPos, PhysicsBody* entityPhysics, const CollisionBox& entityBounds);
    
    // Update platform (for moving platforms)
    void Update(float deltaTime);
    
private:
    // Helper to check if entity is above platform (for one-way platforms)
    bool IsEntityAbove(const Vector2& entityPos, const CollisionBox& entityBounds) const;
};

// Wall collision detection helper
class WallCollision {
public:
    // Check if there's a wall on the left side of the entity
    static bool CheckWallLeft(const Vector2& position, const CollisionBox& bounds, 
                             const std::vector<Platform*>& platforms, float checkDistance = 2.0f);
    
    // Check if there's a wall on the right side of the entity
    static bool CheckWallRight(const Vector2& position, const CollisionBox& bounds, 
                              const std::vector<Platform*>& platforms, float checkDistance = 2.0f);
    
    // Get the wall normal direction (for wall sliding)
    static Vector2 GetWallNormal(const Vector2& position, const CollisionBox& bounds, 
                                 const std::vector<Platform*>& platforms, bool checkLeft);
    
    // Check if entity is touching ground
    static bool CheckGround(const Vector2& position, const CollisionBox& bounds, 
                           const std::vector<Platform*>& platforms, float checkDistance = 2.0f);
};

// Moving platform attachment system
class MovingPlatformAttachment {
public:
    MovingPlatformAttachment();
    ~MovingPlatformAttachment();
    
    // Attach entity to a moving platform
    void AttachToPlatform(Platform* platform);
    
    // Detach from current platform
    void Detach();
    
    // Check if attached to a platform
    bool IsAttached() const { return m_attachedPlatform != nullptr; }
    
    // Get the attached platform
    Platform* GetAttachedPlatform() const { return m_attachedPlatform; }
    
    // Update entity position based on platform movement
    void UpdateAttachment(Vector2& entityPosition, float deltaTime);
    
private:
    Platform* m_attachedPlatform;
    Vector2 m_lastPlatformPosition;
};
