# Sight City: Underworld — Milestone 48

## Mission 13: Kane
Dash tracks Elias Kane, disables his security escort and captures him **alive**. During interrogation Kane reveals that Rhys was involved with Project Sight before Dash returned to Sight City and exposes Vossler's private island data center.

Reward: **$110,000 + 9,500 XP**. Completing the mission unlocks **Mission 14 — No Safe Place**.

Online development remains paused while Story Mode and physical PS Vita testing remain the priority.

# Sight City: Underworld — Milestone 46

## Mission 11: The Facility
Story Mode now continues into the hidden rural Project Sight facility. Dash infiltrates the site, reaches the operations room, downloads the Project Sight target archive, discovers surveillance files on Dash, Mayja and Rhys, identifies **Adrian Vossler** as the man behind Project Sight, and escapes with the evidence.

Reward: **$70,000 + 6,500 XP**. Completing the mission unlocks **Mission 12 — Phase II**.

Online development remains paused while Story Mode and the PS Vita build are the priority.

# Sight City: Underworld — Milestone 45

## Mission 10 — Follow the Money

Story Mode now traces the money through the Industrial Zone, Downtown, Airport and West Coast, identifies Vossler Holdings and Project Sight, awards $55,000 + 5,000 XP, and unlocks Mission 11 — The Facility. Online remains paused.

# City Limits – Cross-gen open-world prototype

Original open-world action prototype designed around a shared gameplay core and
platform-specific rendering/input backends. The goal is a GTA-like genre structure
without copying GTA characters, map, art, missions, names or source code.

## Milestone 01
The shared C++ gameplay core works on desktop/headless, and the project now contains
a real PS Vita backend using VitaSDK + vita2d:
- third-person player and follow camera
- left/right analog Vita input
- enter/exit interaction
- driveable prototype car
- procedural nearby city blocks
- lightweight perspective-projected geometry
- world streaming cells
- 0–5 wanted system

See `src/platform/vita/README.md` and `docs/VITA_MILESTONE_01.md`.

## Desktop smoke test
```sh
cmake -S . -B build
cmake --build build
./build/city_limits
```

## PS Vita build
```sh
export VITASDK=/path/to/vitasdk
cmake -S . -B build-vita \
  -DBUILD_VITA=ON \
  -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake"
cmake --build build-vita
```

The intended Vita target is 960x544 at 30 fps with strict geometry and memory budgets.
The Vita-specific source is under `src/platform/vita`.

## Cross-gen architecture
Gameplay under `src/game` has no console SDK headers. Each console backend owns
rendering, controller input, audio, save data, networking and platform lifecycle.
Official PS3/Xbox 360/PS4 backends require the corresponding authorized console SDKs;
the Vita homebrew backend uses VitaSDK.

## Milestone 02 — Controls + Settings
- Logical `InputAction` layer instead of gameplay code depending directly on Vita buttons.
- Updated Vita defaults: Triangle enter/exit, Square jump, Cross sprint/brake, Circle stealth, L aim, R fire/gas, D-pad Left weapon wheel, D-pad Up phone.
- Persistent sensitivity, deadzone, invert axes, vibration and Aim Assist level.
- Aim Assist defaults to Normal.
- See `docs/VITA_MILESTONE_02.md`.


## Milestone 03
Sight City now uses a 120 km × 120 km world envelope with 64 m streaming cells and a first Downtown/Urban/Suburb procedural renderer. See `docs/VITA_MILESTONE_03.md`.

## Milestone 04

Downtown now has a stronger PS Vita-friendly visual base: procedural facade materials/windows, rooftops, sidewalks, road markings, parks/plazas, and more grounded building geometry. See `docs/VITA_MILESTONE_04.md`.

## Milestone 05
Sight City now streams deterministic Gun Store and General Store storefronts alongside the existing Downtown/Urban/Suburb generation. See `docs/VITA_MILESTONE_05.md`.

## Milestone 06
Adds separate Story/Online map discovery, purchasable property data and ownership persistence, map marker foundations for stores/properties, plus lightweight nearby traffic and pedestrian systems. See `docs/VITA_MILESTONE_06.md`.

## Milestone 08
Milestone 08 adds local pedestrian/traffic simulation, traffic lights, NPC turn indicators, player vehicle blinkers/hazards/headlights, and first-/third-person camera modes with persistent settings. See `docs/VITA_MILESTONE_08.md`.


## Milestone 09
Traffic-law police stops, fines, pursuit escalation, 7-star Wanted, crash injury resolution, ambulance/hospital/custody state flow. See docs/VITA_MILESTONE_09.md.

## Milestone 12
Story and Online now have separate linked campaign progression, while Sight City Online has an expanded Online-only character creator. Story keeps a fixed protagonist identity but can change outfits. Character saves are versioned and keep Story/Online progression separate.

## Milestone 13
Adds the in-game phone foundation with Sight Shop, Properties, Garage, Bank, Businesses, Missions and Map apps. See `docs/VITA_MILESTONE_13.md`.

## Milestone 14
Phone 2.0 + Heist Room foundation: Internet, Mechanic, Contacts, Heists, bank operations, owned getaway-vehicle validation, and NEW/pulsing map-marker state. See `docs/VITA_MILESTONE_14.md`.

## Milestone 15
Property/garage/vehicle ownership 2.0 is implemented in `GarageSystem` and the upgraded `PropertySystem`. See `docs/VITA_MILESTONE_15.md`.

## Milestone 17
Vehicle Shops & Dealerships 2.0 adds dealer stock, used vehicles, test drives, garage delivery, Internet purchase hooks, and per-vehicle tyre-smoke color customization. See `docs/VITA_MILESTONE_17.md`.

## Milestone 18
Playable-system expansion: English-only game text direction, Dash story identity, story-character AI, world/NPC AI crime-response hooks, and a Gun Store with weapons, ammo, armor, sights and suppressors. See `docs/VITA_MILESTONE_18.md`.

## Milestone 19 — NPC Vehicle AI
AI-driven traffic now includes driver personalities, traffic-light reactions, car-following, overtaking, indicators, parking, regional vehicle selection and police-fleeing behavior. See `docs/VITA_MILESTONE_19.md`.


## Milestone 20
Traffic AI now uses a lightweight road graph and route finding, gradual steering, collision avoidance, and NPC police pursuit of fleeing AI drivers. See `docs/VITA_MILESTONE_20.md`.

## Milestone 21
Vehicle physics and physical NPC traffic collision layer are now integrated. AI drivers feed steering/throttle/brake into lightweight physics while preserving road pathfinding and police pursuit behavior. See `docs/VITA_MILESTONE_21.md`.

## Milestone 22
Sight City 3D map chunk integration: 64 m streamed chunks, road surfaces/classes, road snapping, and regional geometry budgets across the locked 120 x 120 km world. See `docs/VITA_MILESTONE_22.md`.

## Milestone 23
Adds the Story-first streamed environment layer: terrain heights, mountains/hills, trees, rocks/cliffs, grass, beaches, ocean water and offshore island zones with Vita-oriented LOD/collision metadata. Online remains postponed until Story is playable.


## Milestone 24
- Story-first world work; Online remains postponed.
- Mount Ridge terrain profile now has a designed 1,800 m summit.
- Broad northern foothills provide a gradual climb toward Mount Ridge.
- South Hills uses a separate rolling profile around 700 m.
- Terrain height is streamed procedurally, preserving the locked 120 x 120 km Sight City map footprint.
- Mountain geometry is suitable for later roads, tunnels, cliffs, vegetation and collision meshes.

## Milestone 25 — Day / Night Cycle
Story Mode now includes `TimeOfDaySystem`: a 48-real-minute full day, Dawn/Morning/Day/Evening/Dusk/Night phases, smooth renderer lighting parameters, street/vehicle-light hooks, time-sensitive traffic/pedestrian density, nightclub activity and shop-hours hooks. Online remains postponed until Story Mode is playable and testable.

## Milestone 26 — Seasons & Dynamic Weather
Adds four seasons, deterministic changing weather, rain/fog/snow/snowstorms, altitude-aware temperature, snow accumulation/melt, wet/snow road grip, and dark blue evening/night sky + blue-grey clouds. Story Mode remains the priority; Online is postponed.

## Milestone 27 - Dash 3D Player Controller
Story-first player gameplay foundation: analog walk/run, sprint, stealth movement, jump/gravity/landing, movement animation states, third/first-person camera compatibility, and proximity-based vehicle enter/exit with vehicle position synchronization. Online remains postponed until Story is playable end-to-end.

## Milestone 28 — Story terrain & world collision
Dash now follows Sight City's real terrain height, handles slopes/steps, collides with streamed trees, rocks, cliffs and lightweight building volumes, and lands on local terrain after jumping/falling. Deep water is blocked until swimming is implemented. Story Mode remains the development priority; Online is postponed.

## Milestone 29
Story-mode flyable aircraft foundation: taxi, takeoff, flight controls, landing/crash handling and a 16-aircraft fictional catalog. See `docs/VITA_MILESTONE_29.md`.

## Milestone 31 — Fire, Explosions & Emergency AI
Adds actual fire/explosion runtime systems plus NPC flee reactions and fire-engine emergency response/suppression. Story mode remains the priority; Online is deferred.

## Milestone 32 — Emergency Response System
Story Mode now has a unified Police + Ambulance + Fire dispatcher. Serious crashes, shootings, explosions, fires and medical calls can request different combinations of services. Ambulances stabilize and transport casualties, police secure scenes, fire engines suppress fires, pedestrians flee danger, and civilian traffic yields for sirens. Online work remains paused until Story is tested on real PS Vita hardware.

## Milestone 33 — Sight City: Underworld
Story-first integration begins. Mission 1, **Back in Sight City**, now has an executable objective/checkpoint/reward flow and unlocks **Like Old Times**. Online implementation is frozen until Story is tested on real PS Vita hardware. The PS Vita app metadata name is now **Sight City: Underworld**.


## Milestone 34 - Mission 2: Like Old Times
Mission 2 is now implemented as a Story gameplay flow: old garage, delivery car, vehicle delivery, rival gang chase, escape to the old workshop, checkpoints/rewards, and Mission 3 unlock. Online development remains paused until Story is tested on real PS Vita hardware.

## Milestone 35
Mission 3 `Easy Money` is playable as a Story-system flow. Dynamic incident locations avoid recently used spots, Story character meeting markers can pulse by time of day, and the speedometer supports saved km/h/mph selection. Online remains paused until Story is tested on real PS Vita hardware.


## Milestone 36
Mission 4 - **The Long Way Home** implemented: Industrial Zone escape, highway pursuit, mountain tunnels, escalating police Wanted response, North Countryside garage, reward and Mission 5 unlock. Story remains the only development priority until physical PS Vita testing.

## Milestone 37 — Weapons Expansion + Wildlife AI
Story-first milestone. Online remains paused until Story is tested on a real PS Vita.

- Expanded Gun Store catalog to 14 weapons across handguns, SMGs, shotguns, rifles, sniper rifles and melee.
- Added WildlifeSystem with deer, hare, fox, boar, birds and seagulls.
- Region-aware wildlife populations with deterministic dynamic spawning rather than fixed encounter points.
- Wildlife AI can roam, feed, rest and flee from nearby danger.
- Animals react to approaching vehicles; close high-speed impacts are registered by the lightweight simulation.
- PS Vita wildlife budget is capped at 12 fully simulated nearby animals; distant wildlife is intended for cheaper LOD/ambient representation.

## Milestone 38 — Story Trophy System
Adds a persistent in-game Story trophy system for PS Vita with Bronze, Silver, Gold and Platinum grades. `Complete It All` is the Platinum trophy and cannot be manually awarded: it unlocks only after every other trophy has been earned. Trophy hooks cover Story missions, heists, racing/bicycle events, exploration, ownership, police escape and aircraft. Online remains paused until Story is tested on real PS Vita hardware.

## Milestone 39 — Combat & Weapon Gameplay
Adds the Story combat runtime: equip/reload/fire, hit and headshot resolution, armor, recoil hooks, NPC engage/cover/flank/flee states, and witnessed-gunfire crime reporting. Online remains paused until Story is tested on real PS Vita.

## Milestone 40 — Mission 5: Our First Score
Story Mission 5 is now implemented as the first full heist flow. Dash meets Mayja in the Heist Room, must choose an owned heist-compatible getaway vehicle, collect equipment, select an escape route, intercept an armored valuables transport, secure the score, escape an escalating police response and deliver the take to the safehouse. Completion awards $75,000 + 3,000 XP and unlocks the Heists progression. Online remains paused until Story is tested on real PS Vita hardware.


## Milestone 41 - Mission 6: Something's Off
- Story Mode only; Online remains paused.
- Rhys misses a planning session after the first heist.
- Dash finds Rhys near rival territory and can follow him.
- The mission deliberately keeps Rhys ambiguous; the betrayal is not revealed yet.
- Mayja detects the first signs of an information leak.
- Reward: $30,000 + 2,500 XP.
- Unlocks Mission 7: Wrong Place, Wrong Time.


## Milestone 42 — Wrong Place, Wrong Time
- Story Mission 7 implemented.
- Rival gang pre-planned Industrial Zone ambush.
- Rhys rescues Dash without revealing the later betrayal.
- Mayja confirms there is an insider.
- Reward: $35,000 + 3,000 XP.
- Mission 8: Behind My Back is unlocked.
- Online remains paused while Story/PS Vita is the priority.


## Milestone 43 — Behind My Back
- Story Mission 8 implemented.
- Dash and Mayja begin direct surveillance of Rhys.
- Tail Rhys through rival territory without alerting him.
- Secure evidence of Rhys meeting the rival gang leader.
- Return the evidence to Mayja.
- Reward: $38,000 + 3,500 XP.
- Unlocks Mission 9: Betrayed.
- Online development remains paused while Story Mode is the priority.

## Milestone 47
Mission 12: **Phase II** adds the 72-hour crisis, choose-two Power/Transport/Money objectives, Downtown lockdown, Elias Kane reveal, and $90,000 + 8,000 XP reward. Online remains paused.


## Milestone 50
Mission 15 - The Hostage is implemented. See `docs/VITA_MILESTONE_50.md`. Online development remains paused.


## Milestone 51
Mission 16 - The Cleanup is implemented with the permanent four-way cleanup choice, Master Ledger reveal, $175,000 + 15,000 XP, and Mission 17 unlock. Online remains paused.

## Milestone 52 — Mission 17: The Ledger
- Story Mission 17 implemented.
- Mayja uses the Master Ledger to identify four Project Sight financial nodes.
- Harbor / Port, Airport, South Hills and Downtown Money Trail can be completed in any order; all four are required.
- The complete financial map confirms Sight City Central Reserve as the next major target.
- Reward: $210,000 + 18,000 XP.
- Unlocks Operation Blackout.
- Online remains paused until Story is tested on a real PS Vita.


## Milestone 53
Mission 18 — Operation Blackout implemented with four mutually exclusive operations, branch bonuses, crew-protection persistence, and Mission 19 unlock. Online development remains paused.


## Milestone 54
Mission 19 - Central Reserve Heist implemented. See `docs/VITA_MILESTONE_54.md`.


## Milestone 55
Central Reserve aftermath and crew cuts implemented. See `docs/VITA_MILESTONE_55.md`.


## Milestone 58
Final battle preparation is implemented: five supply categories, staging board, crew/loadout readiness and TAKE SIGHT CITY unlock. Online remains paused.


## M58
TAKE SIGHT CITY final assault is implemented; Downtown is the final territory and Adrian Vossler is defeated permanently. Story Complete state is now reachable.


## M58
Credits -> A Few Weeks Later -> persistent post-story Free Roam. Vossler stays gone; dynamic gangs, territory attacks, crew calls and Mayja smaller-heist hooks become available. Online remains paused.

## M59 - Story / Free-Roam Polish + PS Vita Test Preparation
- Added explicit PS Vita gameplay budgets targeting 30 FPS.
- Added checkpoint capture/retry state for Story failure recovery.
- Added post-story save-state consistency validation.
- Added conservative active-world caps for Vita (peds, traffic, wildlife, incidents).
- Online remains hard-disabled until Story has passed a physical PS Vita hardware test.
- This milestone prepares the codebase for the first real VitaSDK/VPK hardware test; it does not claim a physical-device test has occurred.

## M60 — VitaSDK / VPK Packaging Preparation
- Vita metadata now uses **Sight City: Underworld**.
- Added strict 128×128 `sce_sys/icon0.png` validation and Vita build helper.
- VPK output is `sight_city_underworld.vpk`.
- Physical Vita testing is still pending; Online remains disabled.
- See `docs/VITA_MILESTONE_60.md`.

## M61 — LiveArea Complete + VitaSDK Build Path
- Added the approved Sight City: Underworld artwork to the Vita package pipeline.
- Added exact 128×128 `sce_sys/icon0.png`.
- Added LiveArea `template.xml`, 840×500 background and 280×158 startup image.
- Fixed the Vita build script to configure the main project with `BUILD_VITA=ON` instead of using `CMakeLists.vita.txt` as a cache preload.
- Vita metadata is `Sight City: Underworld`, Title ID `SCUW00001`, version `00.61`.
- Added Docker and GitHub Actions VitaSDK build paths.
- Online remains disabled until physical PS Vita Story testing passes.
- See `docs/VITA_MILESTONE_61.md`.
