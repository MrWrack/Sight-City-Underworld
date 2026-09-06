# Milestone 60 — VitaSDK / VPK Packaging Preparation

- Vita app name corrected to **Sight City: Underworld**.
- Vita package version set to **00.60**.
- Dedicated output VPK name: `sight_city_underworld.vpk`.
- Packaging now requires `sce_sys/icon0.png` and validates that it is exactly 128×128 PNG.
- Added `scripts/build_vita.sh` for VitaSDK configure/build.
- Added `scripts/verify_vita_package.py` to reject missing/wrong-sized bubble artwork.
- Online remains disabled until Story Mode passes a physical PS Vita hardware test.

## Important
The approved final LiveArea/bubble artwork was not contained in the M59 source archive, so M60 deliberately does **not** substitute a fake icon. Put the approved artwork at `sce_sys/icon0.png` after resizing it to exactly 128×128.

## Hardware test gate
1. Build with VitaSDK.
2. Install generated VPK on a homebrew-enabled PS Vita.
3. Verify LiveArea title/icon.
4. Launch Story Mode and verify save/load + checkpoint retry.
5. Test streaming through Downtown, highway, countryside, coast and Mount Ridge.
6. Record FPS/frame pacing, memory pressure and crashes.
7. Keep Online disabled until the Story build is confirmed acceptable on hardware.
