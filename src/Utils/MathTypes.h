#pragma once

// Lightweight math types used throughout the project.
// Provides a simple Vector2 (float) and Rectangle (int) types
// with common helpers/operators.
//
// These types intentionally live in the global namespace because
// existing code expects `Vector2` and `Rectangle` to be available
// without qualification.

#include <cmath>
#include <algorithm>
#include <cstdint>
#include <d3d9.h>

//
// Vector2 - simple 2D vector with float components
//
struct Vector2 {
    float x;
    float y;

    // Constructors
    Vector2() noexcept : x(0.0f), y(0.0f) {}
    Vector2(float xy) noexcept : x(xy), y(xy) {}
    Vector2(float x_, float y_) noexcept : x(x_), y(y_) {}

    // Element access
    float operator[](std::size_t i) const noexcept { return (i == 0) ? x : y; }
    float& operator[](std::size_t i) noexcept { return (i == 0) ? x : y; }

    // Arithmetic operators
    Vector2 operator+(const Vector2& o) const noexcept { return Vector2(x + o.x, y + o.y); }
    Vector2 operator-(const Vector2& o) const noexcept { return Vector2(x - o.x, y - o.y); }
    Vector2 operator*(float s) const noexcept { return Vector2(x * s, y * s); }
    Vector2 operator/(float s) const noexcept { return Vector2(x / s, y / s); }

    Vector2& operator+=(const Vector2& o) noexcept { x += o.x; y += o.y; return *this; }
    Vector2& operator-=(const Vector2& o) noexcept { x -= o.x; y -= o.y; return *this; }
    Vector2& operator*=(float s) noexcept { x *= s; y *= s; return *this; }
    Vector2& operator/=(float s) noexcept { x /= s; y /= s; return *this; }

    bool operator==(const Vector2& o) const noexcept { return x == o.x && y == o.y; }
    bool operator!=(const Vector2& o) const noexcept { return !(*this == o); }

    // Common operations
    float Length() const noexcept { return std::sqrt(x * x + y * y); }
    float LengthSq() const noexcept { return x * x + y * y; }

    void Normalize() noexcept {
        float len = Length();
        if (len > 0.0f) { x /= len; y /= len; }
    }

    Vector2 Normalized() const noexcept {
        float len = Length();
        if (len > 0.0f) return Vector2(x / len, y / len);
        return Vector2(0.0f, 0.0f);
    }

    static float Dot(const Vector2& a, const Vector2& b) noexcept { return a.x * b.x + a.y * b.y; }
    static float Distance(const Vector2& a, const Vector2& b) noexcept { return (a - b).Length(); }
    static float DistanceSq(const Vector2& a, const Vector2& b) noexcept { return (a - b).LengthSq(); }

    // Utility
    static Vector2 Zero() noexcept { return Vector2(0.0f, 0.0f); }
    static Vector2 One() noexcept { return Vector2(1.0f, 1.0f); }
};

// Scalar * Vector2
inline Vector2 operator*(float s, const Vector2& v) noexcept { return Vector2(v.x * s, v.y * s); }

//
// Rectangle - integer axis-aligned rectangle
// Fields: x, y, width, height. (x,y) is the top-left corner.
// The semantics used in the project assume integer coordinates.
//
struct Rect {
    int x;
    int y;
    int w;
    int h;

    // Constructors
    Rect() noexcept : x(0), y(0), w(0), h(0) {}
    Rect(int x_, int y_, int w_, int h_) noexcept : x(x_), y(y_), w(w_), h(h_) {}

    // Derived properties
    int Left() const noexcept { return x; }
    int Top() const noexcept { return y; }
    int Right() const noexcept { return x + w; }    // exclusive if w is size
    int Bottom() const noexcept { return y + h; }  // exclusive if h is size

    bool IsEmpty() const noexcept { return w <= 0 || h <= 0; }

    // Check if a point (float coords) is inside the rectangle (inclusive on left/top, exclusive on right/bottom)
    bool Contains(float px, float py) const noexcept {
        return (px >= static_cast<float>(Left()) && px < static_cast<float>(Right()) &&
                py >= static_cast<float>(Top())  && py < static_cast<float>(Bottom()));
    }

    bool Contains(const Vector2& p) const noexcept { return Contains(p.x, p.y); }

    // Check if two rectangles intersect (overlap)
    bool Intersects(const Rect& other) const noexcept {
        if (IsEmpty() || other.IsEmpty()) return false;
        return !(Right() <= other.Left() || Left() >= other.Right() ||
                 Bottom() <= other.Top() || Top() >= other.Bottom());
    }

    // Compute intersection rectangle. If no intersection, returns an empty rectangle (width/height = 0).
    Rect Intersection(const Rect& other) const noexcept {
        if (!Intersects(other)) return Rect();
        int nx = (std::max)(Left(), other.Left());
        int ny = (std::max)(Top(), other.Top());
        int nr = (std::min)(Right(), other.Right());
        int nb = (std::min)(Bottom(), other.Bottom());
        return Rect(nx, ny, nr - nx, nb - ny);
    }
};

//
// Color - ARGB color wrapper for Direct3D
//
struct Color {
    DWORD value;
    
    Color() : value(0xFFFFFFFF) {}
    Color(DWORD argb) : value(argb) {}
    Color(BYTE r, BYTE g, BYTE b, BYTE a = 255) : value(D3DCOLOR_ARGB(a, r, g, b)) {}
};
