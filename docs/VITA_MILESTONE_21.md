# City Limits — Milestone 21

## Vehicle Physics + Traffic Collision Layer
- Added lightweight real-time vehicle physics suitable for the Vita performance target.
- AI throttle, brake and steering now feed a vehicle physics body instead of directly translating every car.
- Wheelbase/yaw steering, acceleration, braking and aerodynamic/rolling drag are simulated.
- Added physical car-to-car collision separation, impact response and damage accumulation.
- Existing road-network routing, traffic lights, overtaking and police pursuit remain integrated.
- Designed as the bridge from simulation-only traffic to rendered 3D road vehicles.

Next: bind road geometry/lanes to canonical Sight City map chunks, static road obstacles, renderable vehicle transforms and player-vs-NPC collisions.
