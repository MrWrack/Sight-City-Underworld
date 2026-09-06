# City Limits — Vita Milestone 28

## Story-first world collision & terrain movement
Milestone 28 makes Dash move against Sight City's streamed 3D environment rather than a flat y=0 prototype plane.

Implemented:
- Terrain-following movement using `EnvironmentSystem::sample()`.
- Walkable slope checking (default max 42 degrees).
- Step-height limit (0.45 m) so Dash cannot climb vertical terrain seams.
- Solid collision for streamed trees, rocks and cliffs.
- Deterministic lightweight building collision volumes for streamed 64 m map chunks.
- Axis sliding around obstacles instead of always stopping dead.
- World-boundary protection for the locked 120 x 120 km map.
- Deep-water movement blocking until swimming is implemented later.
- Jumping/falling lands against local terrain height.
- Fall-distance damage hook for later health/combat integration.
- Vita main loop now streams environment/map collision data around the active focus.

### Vita performance direction
Collision is rebuilt only from nearby streamed chunks. Environment obstacles use circles and buildings use simple AABBs; these are deliberately cheap primitives suitable for the Vita target while full visual meshes can remain more detailed.

### Scope
Story Mode remains the priority. Online/multiplayer work is intentionally postponed until the complete Story game is playable and testable.
