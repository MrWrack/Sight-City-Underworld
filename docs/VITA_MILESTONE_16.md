# City Limits — Vita Milestone 16

Garage UI / vehicle customization foundation:
- Owned-vehicle garage data, active vehicle selection and moving between garages.
- Mechanic-compatible ownership records.
- Damage, repair and resale-value integration.
- Heist getaway vehicle lock prevents selling an active heist vehicle.
- Upgrade state: engine, brakes, transmission, suspension, tires, rims, lights and cosmetics.
- Paint Shop: primary/secondary body color, wheel color, gloss/metallic/matte finish and restore original paint.
- Paint and upgrades persist as part of each VehicleRecord; Story and Online use separate GarageSystem instances/saves.
- Vita-friendly data-oriented implementation; rendering/UI screens can consume these states without duplicating vehicle logic.
