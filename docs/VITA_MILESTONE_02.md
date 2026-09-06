# Vita Milestone 02 — Controls + Settings

This milestone replaces hard-coded Vita gameplay buttons with logical gameplay actions and introduces persistent control/gameplay settings.

## PS Vita default mapping
- Left stick: move / steer
- Right stick: camera / aim selection
- Triangle: enter / exit vehicle
- Square: jump (reserved for context pickup/reload extension)
- Cross: sprint on foot / brake in vehicle
- Circle: stealth on foot
- L: aim on foot
- R: fire on foot / accelerate in vehicle
- D-pad Left (hold): weapon wheel
- D-pad Up: phone
- Start: quit prototype

## Settings
`GameSettings` now stores:
- Look sensitivity
- Stick deadzone
- Invert camera X/Y
- Vibration
- Aim Assist: Off / Low / Normal / High (Normal default)

The Vita build loads/saves `ux0:data/CityLimits/settings.cfg`.

## Architecture
`InputAction` and `ActionState` separate physical buttons from gameplay actions. This is the base needed for future per-platform remapping on Vita, PS3, PS4 and Xbox 360.

`AimAssist` contains shared strength/blending helpers. Target acquisition will be connected when enemies/NPC combat are added.
