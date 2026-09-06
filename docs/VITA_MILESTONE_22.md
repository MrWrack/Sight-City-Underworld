# City Limits — Milestone 22

Milestone 22 begins the real Sight City 3D world/road integration while preserving the locked 120 km x 120 km map coordinate system.

- Streams 64 m 3D map chunks around the player.
- Generates road-surface segments per streamed chunk with local/avenue/highway/rural classes.
- Keeps broad Downtown, Airport, Industrial, Mountain, Coast, Urban, Suburb and Countryside placement tied to Sight City coordinates.
- Provides road snapping and nearby-road queries for traffic/gameplay integration.
- Adds lightweight building/vegetation lot budgets for future renderer population.
- Designed so authored geometry can replace procedural chunk geometry without changing world coordinates.

Next: feed these map road segments directly into the traffic path graph and render chunk road/building meshes on Vita.
