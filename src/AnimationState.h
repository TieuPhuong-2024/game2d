#pragma once
#include <string>

// Enumeration of all possible animation states for game entities
// These correspond to the animation clips that will be loaded
enum class AnimationState {
    // Player character states
    Idle,
    Running,
    Jumping,
    Falling,
    WallSliding,
    WallJumping,
    Dashing,
    Shooting,
    ShootingRunning,
    ShootingJumping,
    TakingDamage,
    Death,
    
    // Enemy states
    EnemyIdle,
    EnemyPatrol,
    EnemyAttack,
    EnemyDeath,
    
    // Boss states
    BossIdle,
    BossAttack1,
    BossAttack2,
    BossAttack3,
    BossDeath,
    
    // Generic/utility
    None
};

// Helper functions for animation state management
namespace AnimationStateHelper {
    // Convert animation state to string name
    inline std::string ToString(AnimationState state) {
        switch (state) {
            case AnimationState::Idle: return "Idle";
            case AnimationState::Running: return "Running";
            case AnimationState::Jumping: return "Jumping";
            case AnimationState::Falling: return "Falling";
            case AnimationState::WallSliding: return "WallSliding";
            case AnimationState::WallJumping: return "WallJumping";
            case AnimationState::Dashing: return "Dashing";
            case AnimationState::Shooting: return "Shooting";
            case AnimationState::ShootingRunning: return "ShootingRunning";
            case AnimationState::ShootingJumping: return "ShootingJumping";
            case AnimationState::TakingDamage: return "TakingDamage";
            case AnimationState::Death: return "Death";
            case AnimationState::EnemyIdle: return "EnemyIdle";
            case AnimationState::EnemyPatrol: return "EnemyPatrol";
            case AnimationState::EnemyAttack: return "EnemyAttack";
            case AnimationState::EnemyDeath: return "EnemyDeath";
            case AnimationState::BossIdle: return "BossIdle";
            case AnimationState::BossAttack1: return "BossAttack1";
            case AnimationState::BossAttack2: return "BossAttack2";
            case AnimationState::BossAttack3: return "BossAttack3";
            case AnimationState::BossDeath: return "BossDeath";
            case AnimationState::None: return "None";
            default: return "Unknown";
        }
    }
    
    // Convert string name to animation state
    inline AnimationState FromString(const std::string& name) {
        if (name == "Idle") return AnimationState::Idle;
        if (name == "Running") return AnimationState::Running;
        if (name == "Jumping") return AnimationState::Jumping;
        if (name == "Falling") return AnimationState::Falling;
        if (name == "WallSliding") return AnimationState::WallSliding;
        if (name == "WallJumping") return AnimationState::WallJumping;
        if (name == "Dashing") return AnimationState::Dashing;
        if (name == "Shooting") return AnimationState::Shooting;
        if (name == "ShootingRunning") return AnimationState::ShootingRunning;
        if (name == "ShootingJumping") return AnimationState::ShootingJumping;
        if (name == "TakingDamage") return AnimationState::TakingDamage;
        if (name == "Death") return AnimationState::Death;
        if (name == "EnemyIdle") return AnimationState::EnemyIdle;
        if (name == "EnemyPatrol") return AnimationState::EnemyPatrol;
        if (name == "EnemyAttack") return AnimationState::EnemyAttack;
        if (name == "EnemyDeath") return AnimationState::EnemyDeath;
        if (name == "BossIdle") return AnimationState::BossIdle;
        if (name == "BossAttack1") return AnimationState::BossAttack1;
        if (name == "BossAttack2") return AnimationState::BossAttack2;
        if (name == "BossAttack3") return AnimationState::BossAttack3;
        if (name == "BossDeath") return AnimationState::BossDeath;
        return AnimationState::None;
    }
}
