# Vita Milestone 09 — Police, traffic law, 7-star Wanted

Adds the gameplay-state foundation for:
- Traffic offenses witnessed by police: speeding, red light, wrong-way, reckless driving, hit-and-run.
- Police stop request before pursuit for ordinary traffic violations.
- Pull-over/compliance path with citations/fines.
- Refusing a stop escalates to pursuit.
- Wanted system now supports 0–7 stars.
- Pursuit crash resolution uses injury severity: lower severity -> arrest/jail; severe injury -> ambulance -> hospital -> custody/jail.
- State timers are lightweight and platform-neutral for Vita.

This milestone is systems/state logic. Physical police-car pathfinding, handcuff animations, ambulance routing, hospital/police-station interiors and UI payment screens remain later integration work.
