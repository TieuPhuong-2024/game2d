#include "CollisionSystem.h"
#include <algorithm>
#include <cmath>

CollisionSystem::CollisionSystem()
    : m_collisionCallback(nullptr) {
}

CollisionSystem::~CollisionSystem() {
    Clear();
}

void CollisionSystem::RegisterCollider(Collider* collider) {
    if (!collider) return;
    
    // Check if already registered
    auto it = std::find(m_colliders.begin(), m_colliders.end(), collider);
    if (it == m_colliders.end()) {
        m_colliders.push_back(collider);
    }
}

void CollisionSystem::UnregisterCollider(Collider* collider) {
    if (!collider) return;
    
    auto it = std::find(m_colliders.begin(), m_colliders.end(), collider);
    if (it != m_colliders.end()) {
        m_colliders.erase(it);
    }
}

void CollisionSystem::Update() {
    // Check all collider pairs for collisions
    for (size_t i = 0; i < m_colliders.size(); ++i) {
        Collider* a = m_colliders[i];
        if (!a->isEnabled) continue;
        
        for (size_t j = i + 1; j < m_colliders.size(); ++j) {
            Collider* b = m_colliders[j];
            if (!b->isEnabled) continue;
            
            // Check if these layers should collide
            if (!ShouldCollide(a, b)) continue;
            
            // Check for collision
            CollisionInfo info;
            if (CheckCollision(a, b, &info)) {
                // Invoke callback if set
                if (m_collisionCallback) {
                    m_collisionCallback(info);
                }
            }
        }
    }
}

bool CollisionSystem::CheckCollision(const Collider* a, const Collider* b, CollisionInfo* outInfo) const {
    if (!a || !b) return false;
    if (!a->isEnabled || !b->isEnabled) return false;
    
    // Check if layers should collide
    if (!ShouldCollide(a, b)) return false;
    
    // Perform AABB collision test
    bool colliding = a->box.Intersects(a->position, b->box, b->position);
    
    if (colliding && outInfo) {
        CalculateCollisionInfo(a, b, *outInfo);
    }
    
    return colliding;
}

std::vector<Collider*> CollisionSystem::QueryPoint(const Vector2& point, CollisionLayer mask) {
    std::vector<Collider*> results;
    
    for (Collider* collider : m_colliders) {
        if (!collider->isEnabled) continue;
        if (!HasLayer(mask, collider->layer)) continue;
        
        if (collider->box.Contains(collider->position, point)) {
            results.push_back(collider);
        }
    }
    
    return results;
}

std::vector<Collider*> CollisionSystem::QueryArea(const Rect& area, CollisionLayer mask) {
    std::vector<Collider*> results;
    
    for (Collider* collider : m_colliders) {
        if (!collider->isEnabled) continue;
        if (!HasLayer(mask, collider->layer)) continue;
        
        Rect colliderBounds = collider->box.GetBounds(collider->position);
        if (area.Intersects(colliderBounds)) {
            results.push_back(collider);
        }
    }
    
    return results;
}

Collider* CollisionSystem::Raycast(const Vector2& start, const Vector2& end, CollisionLayer mask, Vector2* hitPoint) {
    Collider* closestHit = nullptr;
    float closestDistance = FLT_MAX;
    Vector2 closestPoint;
    
    Vector2 direction = end - start;
    float rayLength = direction.Length();
    
    if (rayLength < 0.001f) return nullptr;
    
    direction.Normalize();
    
    for (Collider* collider : m_colliders) {
        if (!collider->isEnabled) continue;
        if (!HasLayer(mask, collider->layer)) continue;
        
        // Simple ray-AABB intersection test
        Vector2 boxMin = collider->box.GetMin(collider->position);
        Vector2 boxMax = collider->box.GetMax(collider->position);
        
        float tmin = 0.0f;
        float tmax = rayLength;
        
        // X axis
        if (std::abs(direction.x) > 0.001f) {
            float tx1 = (boxMin.x - start.x) / direction.x;
            float tx2 = (boxMax.x - start.x) / direction.x;
            tmin = (std::max)(tmin, (std::min)(tx1, tx2));
            tmax = (std::min)(tmax, (std::max)(tx1, tx2));
        } else {
            if (start.x < boxMin.x || start.x > boxMax.x) continue;
        }
        
        // Y axis
        if (std::abs(direction.y) > 0.001f) {
            float ty1 = (boxMin.y - start.y) / direction.y;
            float ty2 = (boxMax.y - start.y) / direction.y;
            tmin = (std::max)(tmin, (std::min)(ty1, ty2));
            tmax = (std::min)(tmax, (std::max)(ty1, ty2));
        } else {
            if (start.y < boxMin.y || start.y > boxMax.y) continue;
        }
        
        // Check if ray intersects
        if (tmin <= tmax && tmin < closestDistance) {
            closestDistance = tmin;
            closestHit = collider;
            closestPoint = start + direction * tmin;
        }
    }
    
    if (hitPoint && closestHit) {
        *hitPoint = closestPoint;
    }
    
    return closestHit;
}

void CollisionSystem::SetCollisionCallback(CollisionCallback callback) {
    m_collisionCallback = callback;
}

void CollisionSystem::Clear() {
    m_colliders.clear();
    m_collisionCallback = nullptr;
}

bool CollisionSystem::ShouldCollide(const Collider* a, const Collider* b) const {
    // Check if A's mask includes B's layer
    bool aMaskIncludesB = HasLayer(a->mask, b->layer);
    
    // Check if B's mask includes A's layer
    bool bMaskIncludesA = HasLayer(b->mask, a->layer);
    
    // Both must allow collision with each other
    return aMaskIncludesB && bMaskIncludesA;
}

void CollisionSystem::CalculateCollisionInfo(const Collider* a, const Collider* b, CollisionInfo& info) const {
    info.entityA = a->entity;
    info.entityB = b->entity;
    info.layerA = a->layer;
    info.layerB = b->layer;
    
    // Calculate penetration vector
    info.penetration = a->box.GetPenetration(a->position, b->box, b->position);
    
    // Calculate contact point (midpoint of overlapping region)
    Vector2 aCenter = a->box.GetCenter(a->position);
    Vector2 bCenter = b->box.GetCenter(b->position);
    info.contactPoint = (aCenter + bCenter) * 0.5f;
    
    // Calculate normal (direction from A to B)
    Vector2 centerDiff = bCenter - aCenter;
    float length = centerDiff.Length();
    if (length > 0.001f) {
        info.normal = centerDiff / length;
    } else {
        info.normal = Vector2(0.0f, -1.0f); // Default to up
    }
}
