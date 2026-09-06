# Sight City: Underworld - Milestone 34

## Mission 2: Like Old Times

Story Mode remains the only development priority before a real PS Vita hardware test.

Implemented Mission 2 as a playable mission-state flow:

1. Meet Rhys at the old garage.
2. Enter the delivery car.
3. Drive the car to the delivery point.
4. A rival gang chase starts after delivery.
5. Lose the rival gang.
6. Reach the old workshop.
7. Finish the job with Rhys.

Mission reward: **$8,000 + 650 XP**.
Mission 3 **Easy Money** unlocks after completion.

The gang pursuit has its own chase-pressure state so it is separate from police Wanted level. This can later be driven by real world distance, visibility and pursuing gang vehicles.

Desktop smoke test verifies Mission 1 -> Mission 2 -> Mission 3 unlock progression, rewards, gang-chase state and HUD notifications.
