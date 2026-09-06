# City Limits - Milestone 20

## Road AI + police traffic pursuit
- Added a lightweight road graph with route finding for NPC vehicles.
- NPC drivers select destinations and follow road-node routes instead of random 90-degree wandering.
- Steering now turns gradually toward route nodes.
- Added forward collision prediction and emergency avoidance/braking.
- Added region-based road speed limits.
- Added police traffic units that dynamically select fleeing NPC targets and pursue them.
- Aggressive NPC drivers can become fleeing police targets.
- Existing traffic lights, indicators, overtaking and parking remain integrated.
- Vita traffic population remains capped lower than desktop for performance.

This remains a gameplay prototype. Production navmesh/road splines, vehicle physics, animation, audio and hardware profiling are still required.
