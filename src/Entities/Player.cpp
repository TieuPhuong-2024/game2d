#include "Player.h"
#include "../Utils/Logger.h"
#include <algorithm>

Player::Player()
    : m_position(0.0f, 0.0f)
    , m_animator(nullptr)
    , m_inputManager(nullptr)
    , m_isOnGround(false)
    , m_isOnWall(false)
    , m_isRightWall(true)
    , m_facingRight(true)
    , m_isJumping(false)
    , m_jumpHoldTime(0.0f)
    , m_jumpButtonReleased(false)
    , m_isDashing(false)
    , m_dashTimer(0.0f)
    , m_dashDirection(1.0f)
    , m_isInvincible(false)
    , m_invincibilityTimer(0.0f) {
    
    // Set up collision box (typical Mega Man X size)
    m_collisionBox.width = 16.0f;
    m_collisionBox.height = 24.0f;
    m_collisionBox.offset = Vector2(0.0f, 0.0f);
    
    // Configure physics body
    m_physicsBody.useGravity = true;
    m_physicsBody.mass = 1.0f;
}

Player::~Player() {
}

void Player::Initialize(const Vector2& startPosition) {
    m_position = startPosition;
    m_physicsBody.Reset();
    m_isOnGround = false;
    m_isOnWall = false;
    m_isJumping = false;
    m_isDashing = false;
    m_isInvincible = false;
    
    LOG_INFO("Player initialized at position (" + 
             std::to_string(startPosition.x) + ", " + 
             std::to_string(startPosition.y) + ")");
}

void Player::SetAnimator(Animator* animator) {
    m_animator = animator;
}

void Player::SetInputManager(InputManager* inputManager) {
    m_inputManager = inputManager;
}

void Player::SetPosition(const Vector2& position) {
    m_position = position;
}

void Player::SetOnWall(bool onWall, bool isRightWall) {
    m_isOnWall = onWall;
    m_isRightWall = isRightWall;
}

void Player::Update(float deltaTime) {
    if (!m_inputManager) {
        LOG_WARNING("Player update called without InputManager");
        return;
    }
    
    // Update invincibility timer
    if (m_isInvincible) {
        m_invincibilityTimer -= deltaTime;
        if (m_invincibilityTimer <= 0.0f) {
            m_isInvincible = false;
            m_invincibilityTimer = 0.0f;
        }
    }
    
    // Update dash first (overrides other movement)
    UpdateDash(deltaTime);
    
    if (!m_isDashing) {
        // Handle input and movement
        UpdateMovement(deltaTime);
        UpdateJump(deltaTime);
        UpdateWallSlide(deltaTime);
    }
    
    // Update physics
    UpdatePhysics(deltaTime);
    
    // Update animation state
    UpdateAnimation();
}

void Player::UpdateMovement(float deltaTime) {
    HandleHorizontalInput();
    HandleJumpInput();
    HandleDashInput();
}

void Player::HandleHorizontalInput() {
    if (!m_inputManager) return;
    
    float horizontalInput = 0.0f;
    
    // Get horizontal input
    if (m_inputManager->IsActionDown(InputAction::MOVE_LEFT)) {
        horizontalInput -= 1.0f;
    }
    if (m_inputManager->IsActionDown(InputAction::MOVE_RIGHT)) {
        horizontalInput += 1.0f;
    }
    
    // Update facing direction
    UpdateFacingDirection(horizontalInput);
    
    // Apply horizontal movement (Requirement 1.1: 120 pixels/second)
    if (horizontalInput != 0.0f) {
        m_physicsBody.velocity.x = horizontalInput * MOVE_SPEED;
    } else {
        // Deceleration when no input
        if (m_isOnGround) {
            // Quick stop on ground
            m_physicsBody.velocity.x *= 0.8f;
            if (std::abs(m_physicsBody.velocity.x) < 1.0f) {
                m_physicsBody.velocity.x = 0.0f;
            }
        } else {
            // Slower air control
            m_physicsBody.velocity.x *= 0.95f;
        }
    }
}

void Player::HandleJumpInput() {
    if (!m_inputManager) return;
    
    // Check for jump press (Requirement 1.2)
    if (m_inputManager->IsActionPressed(InputAction::JUMP)) {
        if (m_isOnGround) {
            // Ground jump
            StartJump(JUMP_VELOCITY);
        } else if (m_isOnWall) {
            // Wall jump (Requirement 1.4)
            StartWallJump();
        }
    }
    
    // Track if jump button was released
    if (!m_inputManager->IsActionDown(InputAction::JUMP)) {
        m_jumpButtonReleased = true;
    }
}

void Player::HandleDashInput() {
    if (!m_inputManager) return;
    
    // Check for dash press (Requirement 1.5)
    if (m_inputManager->IsActionPressed(InputAction::DASH) && !m_isDashing) {
        StartDash();
    }
}

void Player::UpdateJump(float deltaTime) {
    if (!m_isJumping) return;
    
    m_jumpHoldTime += deltaTime;
    
    // Variable jump height based on hold duration
    if (m_jumpButtonReleased && m_physicsBody.velocity.y < 0.0f) {
        // Released jump button while ascending - reduce upward velocity
        if (m_physicsBody.velocity.y < JUMP_MIN_VELOCITY) {
            m_physicsBody.velocity.y = JUMP_MIN_VELOCITY;
        }
        m_isJumping = false;
    }
    
    // Stop jump control after max hold time
    if (m_jumpHoldTime >= JUMP_HOLD_TIME) {
        m_isJumping = false;
    }
    
    // Stop jump when falling
    if (m_physicsBody.velocity.y >= 0.0f) {
        m_isJumping = false;
    }
}

void Player::UpdateDash(float deltaTime) {
    if (!m_isDashing) return;
    
    m_dashTimer += deltaTime;
    
    // Dash movement (Requirement 1.5: 80 pixels in 0.3 seconds)
    float dashSpeed = DASH_DISTANCE / DASH_DURATION;
    m_physicsBody.velocity.x = m_dashDirection * dashSpeed;
    m_physicsBody.velocity.y = 0.0f; // No vertical movement during dash
    
    // Disable gravity during dash
    m_physicsBody.useGravity = false;
    
    // End dash after duration
    if (m_dashTimer >= DASH_DURATION) {
        m_isDashing = false;
        m_dashTimer = 0.0f;
        m_physicsBody.useGravity = true;
        m_isInvincible = false; // End invincibility with dash
    }
}

void Player::UpdateWallSlide(float deltaTime) {
    // Wall sliding (Requirement 1.3: 60 pixels/second downward)
    if (m_isOnWall && !m_isOnGround && m_physicsBody.velocity.y > 0.0f) {
        // Limit falling speed to wall slide speed
        if (m_physicsBody.velocity.y > WALL_SLIDE_SPEED) {
            m_physicsBody.velocity.y = WALL_SLIDE_SPEED;
        }
    }
}

void Player::UpdatePhysics(float deltaTime) {
    // Update physics body
    m_physicsBody.Update(deltaTime);
    
    // Apply velocity to position
    m_position += m_physicsBody.velocity * deltaTime;
}

void Player::UpdateAnimation() {
    if (!m_animator) return;
    
    // Determine and set animation state
    AnimationState newState = DetermineAnimationState();
    m_animator->SetCurrentState(newState);
}

AnimationState Player::DetermineAnimationState() const {
    // Determine animation state based on player state (Requirement 5.4)
    if (m_isDashing) {
        return AnimationState::Dashing;
    } else if (m_isOnWall && !m_isOnGround) {
        return AnimationState::WallSliding;
    } else if (!m_isOnGround) {
        // In air
        if (m_physicsBody.velocity.y < 0.0f) {
            return AnimationState::Jumping;
        } else {
            return AnimationState::Falling;
        }
    } else if (m_isOnGround) {
        // On ground
        if (std::abs(m_physicsBody.velocity.x) > 10.0f) {
            return AnimationState::Running;
        } else {
            return AnimationState::Idle;
        }
    }
    
    return AnimationState::Idle;
}

Rect Player::GetCurrentSpriteRect() const {
    if (!m_animator) {
        return Rect();
    }
    
    // Get the current frame rectangle from animator
    Rect spriteRect = m_animator->GetCurrentFrameRect();
    
    // Apply horizontal flipping based on facing direction
    // Note: Actual sprite flipping will be handled by the renderer
    // This method returns the source rectangle for rendering
    return spriteRect;
}

void Player::StartJump(float velocityY) {
    m_physicsBody.velocity.y = velocityY;
    m_isJumping = true;
    m_jumpHoldTime = 0.0f;
    m_jumpButtonReleased = false;
    m_isOnGround = false;
    
    LOG_INFO("Player jumped with velocity: " + std::to_string(velocityY));
}

void Player::StartWallJump() {
    // Wall jump velocity (Requirement 1.4: 250 pixels/second horizontal)
    float horizontalVelocity = m_isRightWall ? -WALL_JUMP_HORIZONTAL : WALL_JUMP_HORIZONTAL;
    
    m_physicsBody.velocity.x = horizontalVelocity;
    m_physicsBody.velocity.y = WALL_JUMP_VERTICAL;
    
    m_isJumping = true;
    m_jumpHoldTime = 0.0f;
    m_jumpButtonReleased = false;
    m_isOnWall = false;
    m_isOnGround = false;
    
    // Update facing direction
    m_facingRight = !m_isRightWall;
    
    LOG_INFO("Player wall jumped");
}

void Player::StartDash() {
    m_isDashing = true;
    m_dashTimer = 0.0f;
    m_dashDirection = m_facingRight ? 1.0f : -1.0f;
    
    // Invincibility during dash (Requirement 1.5)
    m_isInvincible = true;
    m_invincibilityTimer = DASH_DURATION;
    
    LOG_INFO("Player dashed");
}

void Player::UpdateFacingDirection(float horizontalInput) {
    if (horizontalInput > 0.0f) {
        m_facingRight = true;
    } else if (horizontalInput < 0.0f) {
        m_facingRight = false;
    }
}
