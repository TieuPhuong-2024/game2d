#pragma once
#include "MathTypes.h"

// PhysicsBody component for game entities
// Handles velocity, acceleration, gravity, and basic physics calculations
class PhysicsBody {
public:
    PhysicsBody();
    ~PhysicsBody();
    
    // Physics properties
    Vector2 velocity;           // Current velocity in pixels/second
    Vector2 acceleration;       // Current acceleration in pixels/second^2
    float mass;                 // Mass of the object (affects physics calculations)
    float drag;                 // Air resistance coefficient (0.0 = no drag, 1.0 = full drag)
    bool useGravity;            // Whether this body is affected by gravity
    bool isKinematic;           // If true, body is not affected by physics forces
    
    // Gravity constant (pixels/second^2) - matches Mega Man X physics feel
    static constexpr float GRAVITY = 980.0f;
    
    // Update physics simulation
    void Update(float deltaTime);
    
    // Apply force to the body (F = ma)
    void ApplyForce(const Vector2& force);
    
    // Apply impulse (instant velocity change)
    void ApplyImpulse(const Vector2& impulse);
    
    // Set velocity directly
    void SetVelocity(const Vector2& vel);
    void SetVelocity(float vx, float vy);
    
    // Get current velocity
    Vector2 GetVelocity() const;
    
    // Reset physics state
    void Reset();
    
private:
    Vector2 m_forceAccumulator;  // Accumulated forces to apply this frame
};
