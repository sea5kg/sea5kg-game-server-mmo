# Game Design Document — MMO Sandbox (Working Title: TBD)

## Block 1. Concept

### Summary
A top-down 2D MMO sandbox in a retro strategy style, where players build a settlement together and develop their own businesses — farms, smithies, taverns, quarries. Choose a production chain, gather or buy raw materials, produce goods (buildings keep working even while you're offline), and trade with other players. Buildings are assets: a specialist unlocks their potential, and resale creates a market. The world is peaceful, resources are limited, and the economy is the main challenge. Develop your business, specialize, and build a thriving settlement together.

### Genre & Platform
- **Genre:** MMO sandbox, PvE, persistent world, 2D top-down, retro strategy style.
- **Platform:** Web (browser).
- **Graphics:** Canvas 2D, tile-based map, pixel-art / retro sprites, top-down view.
- **Players per world:** 50–200 (single server at launch; `world_id` included in all data from day one).
- **PvP:** None (may be added later).
- **World:** Open, finite tile-based map, empty at launch (no mobs; may be added later).

### Core Gameplay
- Players explore the world, gather resources, and build production buildings (smithy, tavern, farm, quarry).
- Each player develops their own **business** — a production building they own.
- A building produces goods for any owner, but a **specialist** in that industry unlocks its potential (speed + extra recipes).
- Buildings are **assets**: they can be sold and resold between players.

### Tension & Challenge
- Logistics (raw material → building → goods).
- Resource scarcity.
- Economy and trade.
- Cooperation between different businesses.

### Specialization
- **Model:** hybrid of "business = building" (A) and "owner specialist bonus" (C).
- One specialization per player (MVP); can be changed at the cost of lost progress.
- Bonus: production speed + 1–2 additional recipes.

### Goals (MVP)
- Build and develop a settlement.
- Unlock recipes.
- Grow your business.

### Rules & Constraints (MVP)
- **Ownership:** no limit on the number of buildings per player (monopolization risk tracked in Block 9).
- **Protection:** other players' buildings cannot be destroyed.
- **Currency:** money exists + direct player-to-player trade (marketplace/auction — post-MVP).
- **Production chain:** 2 stages (raw material → goods) at MVP; longer chains — post-MVP.
- **Offline production:** buildings keep producing while the owner is offline.

### References
- **Valheim** — building + crafting loop, peaceful world (no mobs at MVP).
- **Retro top-down strategy games** — visual style and camera.

### Technical Assumptions

- `world_id` on all entities in the database.
- Server-side production tick with offline calculation.
- **WebSocket** transport (raw, over TCP).
- **Server language: C++.** No game frameworks, no external multiplayer libraries — custom server implementation.
- Client-server model: server-authoritative state; client sends input intents (click-to-move).
- Building DB schema: `owner_id`, `building_type`, `level`, `storage` (input/output), `production_state`, `specialization_bonus`, `tile_position`.
