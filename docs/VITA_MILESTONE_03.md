# Vita Milestone 03 — 120×120 km Sight City streaming

- Full world coordinate envelope: **120,000 m × 120,000 m = 14,400 km²**.
- World is divided into **64×64 m streaming cells** (1,875 cells per axis conceptually).
- Vita keeps only a **7×7 neighborhood (max 49 cells)** active around the player by default.
- Deterministic region classification: Downtown, Urban, Suburb, Industrial, Airport, Countryside, Mountain, Coast.
- Downtown/Urban/Suburb cells procedurally render roads, lots, parks and different building heights.
- The streaming API is deliberately independent from authored map data, so later district/map files can replace procedural generation.

This milestone proves the huge coordinate space does not imply loading the huge map into Vita RAM.
