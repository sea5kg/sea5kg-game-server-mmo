## Block 3. Mechanics

### 3.1 Map & Movement
- **Map:** finite map with **free coordinates** (x, y), no grid. The visual layer uses tiles/sprites for rendering, but the logic operates on continuous coordinates.
- **Camera:** **fixed, top-down view.** No manual scroll and no zoom (constant scale). **The camera follows the player** — centered on the character, since the map is large and does not fit on screen entirely.
- **Movement:** click-to-move. The player clicks a destination point → the character walks toward it (already implemented).
- **Collisions:** impassable areas (water, buildings, resource zones) block movement.
- **Server authority:** the client sends a movement intent; the server validates the position and broadcasts state.

### 3.2 Resource Zones
- **Types (MVP):** forest (wood), stone deposit (stone), ore deposit (ore), fertile soil (crops).
- **Visibility:**
  - Forest, stone, fertile soil — **visible to everyone** from the start.
  - **Ore** — **hidden** until scouted (see 3.11, MVP stage 2).
- **Placement:** resource zones occupy an **area on the map** (free coordinates), not a single tile.
- **Depletion:** a zone has a finite supply. When depleted, the gathering building stops producing. The player can demolish the building and place a new one in another zone.
- **Respawn (optional, post-MVP):** depleted zones may regenerate over time.
- **No manual gathering:** the player does not chop or mine by hand. Resources are gathered by buildings.

### 3.3 Gathering Buildings
Buildings placed **inside a resource zone** that extract raw materials over time.
- **Lumberyard (forest):** produces wood.
- **Quarry (stone):** produces stone.
- **Mine (ore):** produces ore.
- **Farm (fertile soil):** produces crops.
- **Behavior:** each building has a production tick (server-side). While the zone has resources, the building outputs raw material into the **building's storage** — but only up to the **target amount** set by the owner.
- **"Maintain N" setting:** the owner sets a target amount (e.g., "keep 100 wood"). The building produces while storage < N; once it reaches N — it stops. When goods are taken → production resumes.
- **Storage fee:** **no free limit** — any storage in the building's storage is paid: **1 coin per 10 units** (rounded up), charged **once per day** to the owner. The fee applies to **everything in storage** (output and input resources).
  - No money to pay for storage → the building **stops** until the owner takes the goods or tops up the balance (status `paused_storage`).
- **Shop counter:** the owner **sets the price in advance** for goods (lot: item + quantity + price).
- **Purchase:** another player approaches the building and buys the listed goods immediately, without the owner's confirmation. Money goes to the owner, goods — to the buyer. Works even if the owner is offline.
- **Building storage:** goods sit in the building's storage; the owner can move them to the shared storage.
- **No workers:** the building "ticks" on its own. The client plays a cosmetic animation (smoke, tools, movement) — purely visual, no gameplay effect.
- **Upgrades (post-MVP):** higher level → faster production / larger storage.

### 3.4 Production Buildings
Buildings that convert raw materials into goods.
- **Smithy:** ore → metal goods (e.g., tools, ingots).
- **Tavern:** crops → food / buff items.
- **Behavior:** requires input resources in storage. On each tick, consumes input and produces output.
- **"Maintain N" setting:** the owner sets a target amount of finished goods; the building stops when reached.
- **Storage fee:** same rules — any storage is paid: **1 coin per 10 units per day** (for everything in storage).
- **Input/output:** input and output buffers in the building's storage; the owner can move them to the shared storage.
- **Recipes:** fixed set at MVP; 2-stage chain (raw → goods).
- **No workers:** like gathering buildings — ticks automatically, cosmetic animation on the client.

### 3.5 Crafting
- **Crafting station:** production buildings act as crafting stations (smithy, tavern).
- **Recipes:** defined per building type. Each recipe: input(s) → output, with a time cost.
- **2-stage chain (MVP):** raw → goods. Longer chains — post-MVP.
- **Unlocking:** recipes unlocked by building level / specialization (post-MVP: tech tree).
- **Player crafting:** the player does not craft by hand; crafting happens in buildings.

### 3.6 Building Placement
- **Free coordinates:** buildings are placed at an arbitrary point on the map, no grid snapping.
- **Requirements:** gathering buildings must be placed **inside** the matching resource zone. Production buildings, **storage**, and **shop** — on free passable areas.
- **Validation:** the server checks area availability (bounding box / radius), being inside the zone (for gathering), and ownership before confirming placement.
- **Demolition:** the owner can demolish their building. **Resources in storage are lost on demolition.** The player receives a **warning** before demolition and makes the decision themselves.
- **Protection:** other players cannot destroy or modify your buildings.

### 3.7 Trading, Orders & Ratings
- **Currency:** money exists (earned by selling goods or a starting grant).
- **Auto-sale via building (MVP):** the owner sets the price in advance for goods (lot: item + quantity + price). The buyer purchases immediately if the goods are in the building's storage. Works with an offline owner.
- **Pre-order (MVP):** if goods are not in storage, the buyer can **pay and wait**.
  - **Validation:** an order cannot be placed if the resource zone has **insufficient resources** to fulfill it.
  - **Delivery:** finished goods are **automatically delivered to the buyer's storage** as they are produced.
  - **Buyer cancellation:** with a refund, while the order is not fulfilled. **Cancellation lowers the buyer_rating** (proportional to the order amount).
  - **Building demolished/sold before fulfillment:** the order is **cancelled**, money is **automatically refunded** to the buyer.
- **Shop (trading post, MVP):**
  - **Buy order:** the owner sets "buy X at price Y". **Money is frozen** for the deal. A player brings goods → the shop buys, money is unfrozen and goes to the seller, goods — to the shop's storage. The remainder of frozen money stays for next time.
  - **Delivery contract:** a permanent buy order tied to a specific building. To conclude a contract, the shop owner **physically walks to the building** and concludes it on the spot (both parties agree). Then automatically: the building produces → goods go to the shop's storage → frozen money goes to the building's owner.
  - **Sale (sell order):** the shop sells goods from its storage at the listed price (like a shop counter).
  - **Spread:** the owner earns on the difference between buying and selling.
  - The shop **does not produce** — it only resells/handles logistics.
  - Cancelling a buy order / contract → frozen money is refunded.
  - When selling a building with contracts → contracts **transfer** to the new owner along with frozen money (see building resale).
- **Seller Rating:** public, **0–100**, starts at 100.
  - **Failed order** (building demolished/sold with active orders): lowers the rating **proportional to the order amount** (e.g., −1 per 20 coins, but not less than −1 and not more than −20 per order).
  - **Fulfilled order:** slightly raises the rating (e.g., +1, up to 100), with an anti-abuse restriction.
  - **Decay for dead production:** if for 24 hours there are no active orders and no building produces → **−1 rating per day**. Resuming production stops the decay.
  - **Rating 0 → loss:** all buildings are seized and listed for sale, money goes to the system, the player starts over (rating back to 100).
  - **Rating below 20** → trading and asset transfer blocked (anti-abuse).
  - Visible in the profile and on the building's shop counter.
- **Buyer Rating:** public, **0–100**, starts at 100.
  - **Order cancellation by the buyer** (with refund): lowers the rating **proportional to the order amount**.
  - **Fulfilled (non-cancelled) order:** +1 (up to 100), with anti-abuse.
  - **Rating 0** → pre-orders blocked (can only buy immediately from the counter).
  - **Rating below 20** → limit on the number of active pre-orders.
  - Visible in the buyer's profile.
- **Direct player-to-player trade:** **only for selling/reselling buildings.** Direct goods exchange between players is not provided — goods are sold only via the shop counter / shop.
- **Building resale (MVP):** the owner sells the building to another player via direct trade. Ownership, level, goods in storage, listed lots, **active orders and contracts** transfer to the new owner. **The building is sold "as is", at its coordinates** (relocation — post-MVP).
  - **Orders and contracts are NOT cancelled** — the new owner inherits the obligations and must fulfill them.
  - **Frozen money** for buy orders/contracts transfers with the building.
  - **Seller rating does not decrease** (orders are transferred, not failed).
  - **Responsibility:** the **new owner** is responsible for fulfilling inherited orders; if they fail, their seller_rating suffers.
  - When selling, the buyer is **mandatorily shown** the list of active orders/contracts, frozen money, and goods in storage — for an informed decision.
- **Manual orders (post-MVP):** the buyer leaves a request "buy N at price X", the owner fulfills it.
- **Marketplace / auction (post-MVP):** open sale.
- **No limit on the number of buildings** at MVP (monopolization risk — Block 9).

### 3.8 Specialization & Storage
**Specialization:**
- **Model:** hard — the player chooses **one branch** and can build **only buildings of that branch**.
- **Branches (MVP):** "producer" (lumberyard + smithy) and "trader" (shop).
- **Changing specialization:**
  - **Blocked while the player has at least one building of the old branch.**
  - The player decides: demolish buildings (resources in their storage are lost if not collected) or sell them to another player.
  - Once no old-branch buildings remain → the change is available.
  - **Money is preserved**, **storage** is preserved with its resources.
  - Construction costs are not refunded (the price of the change).
- **Specialist bonus:** production speed + 1–2 extra recipes for buildings of their branch.
- **Display:** specialization is visible in the profile and in the building UI.

**Storage (separate building type):**
- **Purpose:** stores the player's resources, does not produce.
- **Available to all** specializations (not tied to a branch).
- **Built on the map** (free coordinates, occupies space).
- **Storage fee:** 1 coin / 10 units / day (general rules).
- **Quantity:** no limit — the player calculates how many they can pay for.
- **On specialization change:** preserved (not demolished).
- **Sale:** can be sold to another player as a building (with resources inside).

### 3.9 Economy Flow (summary)
1. Place a gathering building in a resource zone → raw materials accumulate in the building's storage.
2. Move raw materials to a production building → goods are produced.
3. List goods on the shop counter with a price → other players buy (immediately or via pre-order).
4. The trader places a shop: buys from producers (buy orders / contracts), sells at a higher price (sell orders).
5. Sell/trade goods for money → buy/upgrade buildings or change specialization.
6. Sell buildings to other players → a building market emerges.

### 3.10 Technical Notes (carried into Block 5)
- Server-authoritative production tick; offline calculation on the server.
- The client sends movement and placement intents; the server validates and broadcasts state.
- **WebSocket transport; custom C++ server; no frameworks.**
- Building DB schema: `owner_id`, `building_type`, `level`, `storage` (input/output), `production_state`, `specialization_bonus`, `position` (x, y — free coordinates), `shop_lots` (counter), `target_amount` (maintain N), `storage_fee_status`.
- **Order:** `buyer_id`, `building_id`, `item`, `quantity`, `price_paid`, `status` (pending / fulfilled / cancelled / refunded).
- **Shop buy order:** `shop_id`, `item`, `quantity`, `price`, `frozen_money`, `status`.
- **Contract:** `shop_id`, `building_id`, `item`, `quantity`, `price`, `frozen_money`, `status`, `both_parties_agreed`.
- **Ratings:** `seller_rating` (0–100), `buyer_rating` (0–100), `failed_orders`, `cancelled_orders`.
- Client animations are cosmetic only — no gameplay authority.
- No account level progression at MVP; cosmetics (skins) only.

### 3.11 Scouting & Maps (MVP Stage 2)
- **Hidden resources:** ore (and rare resources) are not visible on the map until scouted. Forest, stone, fertile soil — visible to everyone.
- **Scouting:** the player explores an area → discovers a deposit. Knowledge is **private** (only the scout knows the coordinates).
- **Maps as goods:** the scout can record a deposit as a "map" (item) and sell it to another player via direct trade.
- **The map buyer** receives the deposit marker and can build there.
- **"Scout" role:** a playstyle — a player who searches for and sells information rather than building. No separate class/progression at stage 2; scouting is available to everyone.
- **Future fork:** a hybrid is possible (publishing a deposit for a reward vs private map sale) — post-MVP.