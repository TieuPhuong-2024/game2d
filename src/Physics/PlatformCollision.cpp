#include "PlatformCollision.h"
#include <cmath>

// Platform implementation
Platform::Platform()
    : type(PlatformType::SOLID)
    , position(0.0f, 0.0f)
    , velocity(0.0f, 0.0f)
    , isMoving(false) {
}

Platform::Platform(PlatformType t, const CollisionBox& b)
    : type(t)
    , bounds(b)
    , position(0.0f, 0.0f)
    , velocity(0.0f, 0.0f)
    , isMoving(false) {
}

Platform::~Platform() {
}

bool Platform::CanCollideWith(const Vector2& entityPos, const Vector2& entityVelocity, const CollisionBox& entityBounds) const {
    // Check if entity intersects with platform
    if (!bounds.Intersects(position, entityBounds, entityPos)) {
        return false;
    }
    
    // For one-way platforms, only collide if entity is moving down and is above the platform
    if (type == PlatformType::ONE_WAY) {
        // Entity must be moving downward (or stationary)
        if (entityVelocity.y < 0.0f) {
            return false;
        }
        
        // Entity's bottom must be at or above platform's top
        float entityBottom = entityBounds.GetBottom(entityPos);
        float platformTop = bounds.GetTop(position);
        
        // Allow some tolerance for landing
        return entityBottom <= platformTop + 5.0f;
    }
    
    return true;
}

void Platform::ResolveCollision(Vector2& entityPos, PhysicsBody* entityPhysics, const CollisionBox& entityBounds) {
    if (!bounds.Intersects(position, entityBounds, entityPos)) {
        return;
    }
    
    // Get penetration vector
    Vector2 penetration = bounds.GetPenetration(position, entityBounds, entityPos);
    
    // For one-way platforms, only resolve vertical collision from above
    if (type == PlatformType::ONE_WAY) {
        if (penetration.y < 0.0f) {
            // Push entity up
            entityPos.y += penetration.y;
            
            // Stop downward velocity
            if (entityPhysics && entityPhysics->velocity.y > 0.0f) {
                entityPhysics->velocity.y = 0.0f;
            }
        }
        return;
    }
    
    // For solid platforms, resolve collision in the direction of least penetration
    entityPos += penetration;
    
    // Adjust velocity based on collision direction
    if (entityPhysics) {
        // Vertical collision
        if (std::abs(penetration.y) > 0.001f) {
            if (penetration.y < 0.0f && entityPhysics->velocity.y > 0.0f) {
                // Hit from below (ceiling)
                entityPhysics->velocity.y = 0.0f;
            } else if (penetration.y > 0.0f && entityPhysics->velocity.y < 0.0f) {
                // Hit from above (floor)
                entityPhysics->velocity.y = 0.0f;
            }
        }
        
        // Horizontal collision
        if (std::abs(penetration.x) > 0.001f) {
            if ((penetration.x < 0.0f && entityPhysics->velocity.x > 0.0f) ||
                (penetration.x > 0.0f && entityPhysics->velocity.x < 0.0f)) {
                entityPhysics->velocity.x = 0.0f;
            }
        }
    }
}

void Platform::Update(float deltaTime) {
    if (isMoving) {
        position += velocity * deltaTime;
    }
}

bool Platform::IsEntityAbove(const Vector2& entityPos, const CollisionBox& entityBounds) const {
    float entityBottom = entityBounds.GetBottom(entityPos);
    float platformTop = bounds.GetTop(position);
    return entityBottom <= platformTop;
}

// WallCollision implementation
bool WallCollision::CheckWallLeft(const Vector2& position, const CollisionBox& bounds, 
                                  const std::vector<Platform*>& platforms, float checkDistance) {
    // Create a small check box to the left of the entity
    Vector2 checkPos = position;
    checkPos.x -= checkDistance;
    
    CollisionBox checkBox(0.0f, bounds.height * 0.25f, 2.0f, bounds.height * 0.5f);
    
    for (Platform* platform : platforms) {
        if (platform->type == PlatformType::ONE_WAY) continue;
        
        if (platform->bounds.Intersects(platform->position, checkBox, checkPos)) {
            return true;
        }
    }
    
    return false;
}

bool WallCollision::CheckWallRight(const Vector2& position, const CollisionBox& bounds, 
                                   const std::vector<Platform*>& platforms, float checkDistance) {
    // Create a small check box to the right of the entity
    Vector2 checkPos = position;
    checkPos.x += bounds.width + checkDistance;
    
    CollisionBox checkBox(0.0f, bounds.height * 0.25f, 2.0f, bounds.height * 0.5f);
    
    for (Platform* platform : platforms) {
        if (platform->type == PlatformType::ONE_WAY) continue;
        
        if (platform->bounds.Intersects(platform->position, checkBox, checkPos)) {
            return true;
        }
    }
    
    return false;
}

Vector2 WallCollision::GetWallNormal(const Vector2& position, const CollisionBox& bounds, 
                                     const std::vector<Platform*>& platforms, bool checkLeft) {
    if (checkLeft) {
        return CheckWallLeft(position, bounds, platforms) ? Vector2(1.0f, 0.0f) : Vector2::Zero();
    } else {
        return CheckWallRight(position, bounds, platforms) ? Vector2(-1.0f, 0.0f) : Vector2::Zero();
    }
}

bool WallCollision::CheckGround(const Vector2& position, const CollisionBox& bounds, 
                                const std::vector<Platform*>& platforms, float checkDistance) {
    // Create a small check box below the entity
    Vector2 checkPos = position;
    checkPos.y += bounds.height;
    
    CollisionBox checkBox(bounds.width * 0.25f, 0.0f, bounds.width * 0.5f, checkDistance);
    
    for (Platform* platform : platforms) {
        if (platform->bounds.Intersects(platform->position, checkBox, checkPos)) {
            return true;
        }
    }
    
    return false;
}

// MovingPlatformAttachment implementation
MovingPlatformAttachment::MovingPlatformAttachment()
    : m_attachedPlatform(nullptr)
    , m_lastPlatformPosition(0.0f, 0.0f) {
}

MovingPlatformAttachment::~MovingPlatformAttachment() {
}

void MovingPlatformAttachment::AttachToPlatform(Platform* platform) {
    if (!platform) return;
    
    m_attachedPlatform = platform;
    m_lastPlatformPosition = platform->position;
}

void MovingPlatformAttachment::Detach() {
    m_attachedPlatform = nullptr;
}

void MovingPlatformAttachment::UpdateAttachment(Vector2& entityPosition, float deltaTime) {
    if (!m_attachedPlatform) return;
    
    // Calculate platform movement since last frame
    Vector2 platformDelta = m_attachedPlatform->position - m_lastPlatformPosition;
    
    // Move entity with the platform
    entityPosition += platformDelta;
    
    // Update last platform position
    m_lastPlatformPosition = m_attachedPlatform->position;
}
