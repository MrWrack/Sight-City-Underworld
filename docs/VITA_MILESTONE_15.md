# City Limits — Milestone 15

Properties, Garage & Vehicle Ownership 2.0.

- Residential ownership cap: 6 per Story/Online profile.
- Standalone garages: no ownership-count cap; individual garages support up to 50 vehicle slots.
- Residential garages can be upgraded up to 50 slots.
- Business garages support up to 100 slots.
- GarageSystem owns vehicles per profile and prevents over-capacity moves.
- Business vehicles can transfer to a replacement location of the same business type.
- Vehicle sale is blocked while mission/heist locked.
- Sale value is driven mainly by condition/crash damage. An undamaged used vehicle tops out at 80% of purchase price. Repairs restore condition but permanent crash wear prevents value returning to the original new price.
- Mechanic phone integration consumes the same owned-vehicle view.
- Story and Online are represented by independent GarageSystem/PropertySystem instances in game integration.
