# PS Vita backend – Milestone 01

This backend turns the shared City Limits gameplay core into a Vita application.
It uses VitaSDK + vita2d and lightweight perspective projection for a small city
made from original colored geometry. No GTA assets, map data, missions or code are used.

## Controls
- Left stick: walk / steer
- Right stick: orbit camera
- R: accelerate while driving
- L: brake / reverse
- X: enter or exit the nearby car
- Triangle: add wanted heat (temporary prototype test)
- Start: quit

## Vita target
- 960x544 output
- 30 fps gameplay target
- Nearby procedural blocks instead of one huge world mesh
- Flat-shaded low-cost geometry
- Shared gameplay code remains platform-independent

## Build with VitaSDK
You need VitaSDK and vita2d available in that toolchain.

```sh
export VITASDK=/path/to/vitasdk
cmake -S . -B build-vita \
  -DBUILD_VITA=ON \
  -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake"
cmake --build build-vita
```

A successful Vita build creates `city_limits_vita.vpk` in the build directory.
