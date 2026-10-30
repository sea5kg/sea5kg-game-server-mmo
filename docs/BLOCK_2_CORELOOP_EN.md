## Block 2. Core Loop

### Micro Loop (seconds)
- Player clicks a destination point → character moves toward it (free coordinates, no grid).
- Click a building or empty spot → open building menu / placement preview.
- Place a building (gathering or production) at a valid free position.
- Assign a production task (what to gather / what to craft).
- Collect output from storage, deposit input, trade.

### Meso Loop (session 30–90 min)
1. Explore the map → find resource areas (forest, stone, ore, fertile soil).
2. Place a gathering building on/near a resource area (lumberyard, quarry, mine, farm).
3. Assign a gathering task → raw materials accumulate in the player's shared storage over time.
4. Place a production building (smithy, tavern) → assign a recipe → it consumes raw materials and produces goods (including while offline).
5. Collect goods → sell or trade them to other players for money.
6. Reinvest money into new buildings or upgrades.

### Macro Loop (days / weeks)
- Expand the production chain: more resource areas → more production buildings → more goods.
- Specialize in one industry → unlock speed bonus + extra recipes → become the best supplier in that niche.
- Trade and resell buildings: buy an underused building, specialize, and resell it at a higher value.
- Shared settlement goals (post-MVP).
- **No account level progression at MVP.** Cosmetics (skins) only.

### Player Role
- The player is a **manager / entrepreneur**, not a hero.
- No manual gathering: all extraction and crafting happens in buildings.
- No worker NPCs (MVP): buildings tick automatically; client plays cosmetic animations only.
- Movement is click-to-move on **free coordinates** (no grid; already implemented).

### Key Design Principles
- **Peaceful world:** no PvP, no mobs at MVP — tension comes from logistics and economy.
- **Buildings are the core unit:** they gather, produce, upgrade, and can be sold.
- **Offline progress:** buildings keep working while the owner is away, so returning is rewarding.
- **Specialization creates the market:** specialist owners are more valuable, driving demand for buildings and trade.
- **Finite resources:** resource areas deplete, pushing players to expand across the map.
- **Free coordinates:** movement and building placement are continuous, not grid-locked.

### Technical Notes (carried into Block 5)
- Server-authoritative production tick; offline calculation on server.
- Shared player storage (MVP) simplifies production and trading.
- Client sends movement and placement intents; server validates and broadcasts state.
- **WebSocket transport; custom C++ server; no frameworks.**
- Building DB schema: `owner_id`, `building_type`, `level`, `storage`, `production_state`, `specialization_bonus`, `position` (x, y — free coordinates).
- No account level system at MVP; cosmetics (skins) only.