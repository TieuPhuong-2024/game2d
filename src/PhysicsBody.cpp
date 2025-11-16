#include "PhysicsBody.h"
#include "Logger.h"

PhysicsBody::PhysicsBody()
    : velocity(0.0f, 0.0f)
    , acceleration(0.0f, 0.0f)
    , mass(1.0f)
    , drag(0.0f)
    , useGravity(true)
    , isKinematic(false)
    , m_forceAccumulator(0.0f, 0.0f) {
}

PhysicsBody::~PhysicsBody() {
}

void PhysicsBody::Update(float deltaTime) {
    // Skip physics if kinematic
    if (isKinematic) {
        return;
    }
    
    // Apply gravity if enabled
    if (useGravity) {
        m_forceAccumulator.y += mass * GRAVITY;
    }
    
    // Calculate acceleration from forces (F = ma, so a = F/m)
    if (mass > 0.0f) {
        acceleration = m_forceAccumulator / mass;
    } else {
        acceleration = Vector2::Zero();
    }
    
    // Update velocity from acceleration
    velocity += acceleration * deltaTime;
    
    // Apply drag (air resistance)
    if (drag > 0.0f) {
        float dragFactor = 1.0f - (drag * deltaTime);
        if (dragFactor < 0.0f) dragFactor = 0.0f;
        velocity *= dragFactor;
    }
    
    // Clear force accumulator for next frame
    m_forceAccumulator = Vector2::Zero();
}

void PhysicsBody::ApplyForce(const Vector2& force) {
    if (isKinematic) return;
    m_forceAccumulator += force;
}

void PhysicsBody::ApplyImpulse(const Vector2& impulse) {
    if (isKinematic) return;
    // Impulse directly changes velocity (impulse = mass * velocity change)
    if (mass > 0.0f) {
        velocity += impulse / mass;
    }
}

void PhysicsBody::SetVelocity(const Vector2& vel) {
    velocity = vel;
}

void PhysicsBody::SetVelocity(float vx, float vy) {
    velocity.x = vx;
    velocity.y = vy;
}

Vector2 PhysicsBody::GetVelocity() const {
    return velocity;
}

void PhysicsBody::Reset() {
    velocity = Vector2::Zero();
    acceleration = Vector2::Zero();
    m_forceAccumulator = Vector2::Zero();
}
