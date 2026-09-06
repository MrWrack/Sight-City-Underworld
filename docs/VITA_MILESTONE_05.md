# City Limits – Vita Milestone 05

Adds two streamed Sight City storefront types to the procedural city renderer:

- Gun Store – sparse Downtown/Urban storefront with a distinct red sign and compact shop shell.
- General Store – more common neighborhood shop with green signage, windows, door and awning.

Both are deterministic from the world-cell seed, so their locations stay stable without storing millions of objects across the 120 x 120 km map. They use the same lightweight geometry/material approach as Milestone 04 and therefore add no large texture package or full-world memory cost.

This milestone adds the exterior/world foundation. Shop interiors, interaction prompts, inventory/purchasing and map icons can be layered on top later without changing the sparse world streaming architecture.
