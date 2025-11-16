#pragma once
#include "../Utils/MathTypes.h"
#include "../Physics/PhysicsBody.h"
#include "../Physics/CollisionBox.h"
#include "../Animation/Animator.h"
#include "../Input/InputManager.h"

// Player character (X) with movement, combat, and special abilities
class Player {
public:
    Player();
    ~Player();
    
    // Initialization
    void Initialize(const Vector2& startPosition);
    void SetAnimator(Animator* animator);
    void SetInputManager(InputManager* inputManager);
    
    // Update and rendering
    void Update(float deltaTime);
    Rect GetCurrentSpriteRect() const;
    
    // Position and transform
    Vector2 GetPosition() const { return m_position; }
    void SetPosition(const Vector2& position);
    
    // Physics
    PhysicsBody& GetPhysicsBody() { return m_physicsBody; }
    const PhysicsBody& GetPhysicsBody() const { return m_physicsBody; }
    
    // Collision
    CollisionBox& GetCollisionBox() { return m_collisionBox; }
    const CollisionBox& GetCollisionBox() const { return m_collisionBox; }
    
    // State queries
    bool IsOnGround() const { return m_isOnGround; }
    bool IsOnWall() const { return m_isOnWall; }
    bool IsDashing() const { return m_isDashing; }
    bool IsFacingRight() const { return m_facingRight; }
    
    // Ground and wall detection
    void SetOnGround(bool onGround) { m_isOnGround = onGround; }
    void SetOnWall(bool onWall, bool isRightWall = true);
    
    // Movement constants (from requirements)
    static constexpr float MOVE_SPEED = 120.0f;           // pixels/second (Req 1.1)
    static constexpr float JUMP_VELOCITY = -300.0f;       // pixels/second upward (Req 1.2)
    static constexpr float WALL_SLIDE_SPEED = 60.0f;      // pixels/second downward (Req 1.3)
    static constexpr float WALL_JUMP_HORIZONTAL = 250.0f; // pixels/second (Req 1.4)
    static constexpr float WALL_JUMP_VERTICAL = -280.0f;  // pixels/second upward
    static constexpr float DASH_DISTANCE = 80.0f;         // pixels (Req 1.5)
    static constexpr float DASH_DURATION = 0.3f;          // seconds (Req 1.5)
    static constexpr float INVINCIBILITY_DURATION = 1.0f; // seconds
    
    // Jump control
    static constexpr float JUMP_HOLD_TIME = 0.3f;         // Max time to hold jump for variable height
    static constexpr float JUMP_MIN_VELOCITY = -150.0f;   // Minimum jump velocity if released early
    
private:
    // Core components
    Vector2 m_position;
    PhysicsBody m_physicsBody;
    CollisionBox m_collisionBox;
    Animator* m_animator;
    InputManager* m_inputManager;
    
    // Movement state
    bool m_isOnGround;
    bool m_isOnWall;
    bool m_isRightWall;
    bool m_facingRight;
    
    // Jump state
    bool m_isJumping;
    float m_jumpHoldTime;
    bool m_jumpButtonReleased;
    
    // Dash state
    bool m_isDashing;
    float m_dashTimer;
    float m_dashDirection;
    
    // Invincibility
    bool m_isInvincible;
    float m_invincibilityTimer;
    
    // Update methods
    void UpdateMovement(float deltaTime);
    void UpdateJump(float deltaTime);
    void UpdateDash(float deltaTime);
    void UpdateWallSlide(float deltaTime);
    void UpdatePhysics(float deltaTime);
    void UpdateAnimation();
    
    // Input handling
    void HandleHorizontalInput();
    void HandleJumpInput();
    void HandleDashInput();
    
    // Helper methods
    void StartJump(float velocityY);
    void StartWallJump();
    void StartDash();
    void UpdateFacingDirection(float horizontalInput);
    AnimationState DetermineAnimationState() const;
};
