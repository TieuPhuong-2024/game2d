# Source Code Organization

This document describes the organization of the source code for the Mega Man X3 game project.

## Folder Structure

```
src/
├── Core/           - Game engine core and entry point
├── Graphics/       - Rendering and visual systems
├── Animation/      - Animation and sprite management
├── Input/          - Input handling and configuration
├── Physics/        - Physics simulation and collision detection
├── Entities/       - Game entities (player, enemies, etc.)
└── Utils/          - Utility classes and helpers
```

## Module Descriptions

### Core/
Contains the main game engine and application entry point.
- `GameEngine.h/cpp` - Main game engine class, manages all subsystems
- `main.cpp` - Application entry point (WinMain)

### Graphics/
Handles all rendering operations using Direct3D 9.
- `D3DRenderer.h/cpp` - Main renderer, manages Direct3D device
- `Texture2D.h/cpp` - Texture loading and management
- `SpriteBatch.h/cpp` - Efficient 2D sprite rendering
- `Camera2D.h/cpp` - 2D camera with transform support
- `DebugDraw.h/cpp` - Debug visualization (colored rectangles)

### Animation/
Manages sprite animations and sprite sheets.
- `Animator.h/cpp` - Animation state machine and playback
- `AnimationClip.h/cpp` - Individual animation clip data
- `AnimationState.h` - Enumeration of animation states
- `SpriteSheet.h/cpp` - Sprite sheet texture and frame management

### Input/
Handles keyboard and gamepad input.
- `InputManager.h/cpp` - Input polling and action mapping
- `InputConfig.h/cpp` - Input configuration and key bindings

### Physics/
Physics simulation and collision detection systems.
- `PhysicsBody.h/cpp` - Rigid body physics component
- `CollisionBox.h/cpp` - AABB collision box
- `CollisionSystem.h/cpp` - Collision detection and resolution
- `PlatformCollision.h/cpp` - Platform-specific collision handling

### Entities/
Game entities and characters.
- `Player.h/cpp` - Player character (X) with movement and abilities

### Utils/
Shared utilities and helper classes.
- `MathTypes.h` - Math types (Vector2, Rect, etc.)
- `Logger.h/cpp` - Logging system

## Include Path Convention

All includes use relative paths from the file's location:
- Same folder: `#include "FileName.h"`
- Different folder: `#include "../FolderName/FileName.h"`

Example from `Player.h` in Entities/:
```cpp
#include "../Utils/MathTypes.h"
#include "../Physics/PhysicsBody.h"
#include "../Animation/Animator.h"
```

## Adding New Files

When adding new files:
1. Place them in the appropriate folder based on their purpose
2. Update `MegaManX3.vcxproj` to include the new files
3. Use relative includes following the convention above
4. Update this README if adding a new module category
