# Milestone 61 — LiveArea Complete + First VitaSDK Build Path

M61 completes the PS Vita package resource layout for **Sight City: Underworld**.

## Added

- Canonical approved Sight City: Underworld bubble artwork is now included as the source artwork.
- `sce_sys/icon0.png` generated at the required **128×128** size.
- LiveArea assets:
  - `sce_sys/livearea/contents/template.xml`
  - `sce_sys/livearea/contents/bg.png` — **840×500**
  - `sce_sys/livearea/contents/startup.png` — **280×158**
- Vita package metadata:
  - App name: `Sight City: Underworld`
  - Title ID: `SCUW00001`
  - Version: `00.61`
  - Output: `sight_city_underworld.vpk`
- `scripts/verify_vita_package.py` validates all resource sizes, XML and CMake package references.
- `scripts/build_vita.sh` now uses the main CMake project correctly with `BUILD_VITA=ON`.
- `scripts/build_vita_docker.sh` provides a VitaSDK Docker build path.
- GitHub Actions workflow can build and upload the VPK artifact in a VitaSDK container.

## Test status

The desktop build and M61 package verification can be run in this workspace. A real ARM VitaSDK compilation still requires VitaSDK (or Docker/GitHub Actions), and a physical PS Vita test has **not** been claimed.

Online remains hard-disabled until the Story build has been tested successfully on real PS Vita hardware.
