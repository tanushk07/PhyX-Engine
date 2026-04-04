# phyX

A lightweight 3D physics engine built from scratch in C++17.  
Implements core rigid-body dynamics, collision detection primitives, and real-time debug visualization using [Raylib](https://www.raylib.com/).

> 🚧 **This project is a work in progress.** New features, collider types, and optimizations are planned. Stay tuned for updates.

## Features

- **Rigid-body simulation** — Gravity, velocity integration, and restitution-based bouncing
- **Sphere vs Sphere** collision detection with penetration depth
- **AABB vs AABB** overlap testing using the separating-axis method
- **Sphere vs Plane** intersection and positional correction
- **Sphere vs AABB** closest-point intersection test
- **Real-time 3D visualizer** — Wireframe debug rendering, orbit camera, and HUD overlays

## Building

Requires **CMake 3.14+** and a C++17 compiler. Raylib is fetched automatically during configuration.

```bash
cmake -B build -S .
cmake --build build
```

Run the demo:

```bash
./build/phyX          # Linux / macOS
.\build\Debug\phyX.exe  # Windows (MSVC)
```

## Controls

| Input | Action |
|---|---|
| Right-click + drag | Orbit camera |
| W / A / S / D | Move camera forward / left / back / right |
| Q / E | Move camera down / up |
| Scroll wheel | Zoom |

## Project Structure

```
src/
├── main.cpp                  # Demo scene
├── Engine.h / Engine.cpp     # Physics engine (stepping, collision dispatch)
├── core/
│   ├── Vector.h              # Vec3 math
│   ├── RigidBody.h / .cpp    # Rigid-body dynamics
│   └── IntersectionData.h    # Collision result payload
├── colliders/
│   ├── BoundingSphere.h / .cpp
│   ├── AABB.h / .cpp
│   └── PlaneCollider.h / .cpp
└── utils/
    └── Visualizer.h / .cpp   # Raylib rendering wrapper
```

## License

MIT
