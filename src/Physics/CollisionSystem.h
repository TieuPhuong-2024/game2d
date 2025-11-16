#pragma once
#include "../Utils/MathTypes.h"
#include "CollisionBox.h"
#include <vector>
#include <functional>

// Collision layers for filtering
enum class CollisionLayer {
    NONE = 0,
    PLAYER = 1 << 0,        // 1
    ENEMY = 1 << 1,         // 2
    PROJECTILE = 1 << 2,    // 4
    ENVIRONMENT = 1 << 3,   // 8
    COLLECTIBLE = 1 << 4,   // 16
    ALL = 0xFFFFFFFF        // All layers
};

// Bitwise operators for collision layers
inline CollisionLayer operator|(CollisionLayer a, CollisionLayer b) {
    return static_cast<CollisionLayer>(static_cast<int>(a) | static_cast<int>(b));
}

inline CollisionLayer operator&(CollisionLayer a, CollisionLayer b) {
    return static_cast<CollisionLayer>(static_cast<int>(a) & static_cast<int>(b));
}

inline bool HasLayer(CollisionLayer mask, CollisionLayer layer) {
    return (static_cast<int>(mask) & static_cast<int>(layer)) != 0;
}

// Collision information structure
struct CollisionInfo {
    void* entityA;              // First entity involved in collision
    void* entityB;              // Second entity involved in collision
    Vector2 penetration;        // Penetration vector (how much to separate)
    Vector2 contactPoint;       // Point of contact
    Vector2 normal;             // Collision normal (direction of separation)
    CollisionLayer layerA;      // Layer of entity A
    CollisionLayer layerB;      // Layer of entity B
};

// Collider component that can be attached to entities
struct Collider {
    CollisionBox box;           // The collision box
    CollisionLayer layer;       // What layer this collider is on
    CollisionLayer mask;        // What layers this collider can collide with
    void* entity;               // Pointer to the owning entity
    Vector2 position;           // Current position
    bool isTrigger;             // If true, collision is detected but no physics response
    bool isEnabled;             // If false, collider is ignored
    
    Collider()
        : layer(CollisionLayer::NONE)
        , mask(CollisionLayer::ALL)
        , entity(nullptr)
        , position(0.0f, 0.0f)
        , isTrigger(false)
        , isEnabled(true) {
    }
};

// Collision callback types
using CollisionCallback = std::function<void(const CollisionInfo&)>;

// CollisionSystem manages all collision detection and response
class CollisionSystem {
public:
    CollisionSystem();
    ~CollisionSystem();
    
    // Register a collider with the system
    void RegisterCollider(Collider* collider);
    
    // Unregister a collider from the system
    void UnregisterCollider(Collider* collider);
    
    // Update all collisions
    void Update();
    
    // Check collision between two specific colliders
    bool CheckCollision(const Collider* a, const Collider* b, CollisionInfo* outInfo = nullptr) const;
    
    // Query collisions at a specific position
    std::vector<Collider*> QueryPoint(const Vector2& point, CollisionLayer mask = CollisionLayer::ALL);
    
    // Query collisions in a rectangular area
    std::vector<Collider*> QueryArea(const Rect& area, CollisionLayer mask = CollisionLayer::ALL);
    
    // Raycast from start to end, returns first collider hit
    Collider* Raycast(const Vector2& start, const Vector2& end, CollisionLayer mask = CollisionLayer::ALL, Vector2* hitPoint = nullptr);
    
    // Set collision callback for when collisions occur
    void SetCollisionCallback(CollisionCallback callback);
    
    // Clear all registered colliders
    void Clear();
    
    // Get all active colliders
    const std::vector<Collider*>& GetColliders() const { return m_colliders; }
    
private:
    std::vector<Collider*> m_colliders;
    CollisionCallback m_collisionCallback;
    
    // Helper to check if two layers should collide
    bool ShouldCollide(const Collider* a, const Collider* b) const;
    
    // Helper to calculate collision info
    void CalculateCollisionInfo(const Collider* a, const Collider* b, CollisionInfo& info) const;
};
