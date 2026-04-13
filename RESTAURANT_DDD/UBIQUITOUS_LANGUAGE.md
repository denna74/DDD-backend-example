# Ubiquitous Language

Shared vocabulary for the Restaurant DDD project. Each term is defined in plain language first, followed by its technical counterpart. Terms are grouped by **Bounded Context** — the subdomain that owns them.

---

## Bounded Contexts

| Context | Owner | Responsibility |
|---------|-------|----------------|
| **Kitchen** | Marco | Dishes, cooking stations, fire orders |
| **Floor** | Sara | Tables, seating, menu presentation |
| **Finance** | Diane | Costs, waste, margins, reports |

---

## Kitchen Context

### Dish
A cookable item assigned to a station with a defined preparation time and cooking method.
> `Dish` aggregate — `src/kitchen/domain/dish.h`

### Station
The physical area of the kitchen where a dish is prepared (e.g. Grill, Sauce, Cold, Pastry).
> `Station` enum — `src/kitchen/domain/station.h`

### Cooking Method
The technique used to prepare a dish (e.g. "grill", "sauté", "bake").
> String field on `Dish`.

### Prep Time
How many minutes a dish takes to cook from raw to ready.
> `prepTimeMinutes` field on `Dish`.

### Fire Order
A coordinated cooking sequence grouping multiple dishes for the same table, with timing offsets so everything is ready at once.
> `FireOrder` aggregate — `src/kitchen/domain/fire_order.h`

### Fire Order Line
One dish within a fire order, tracking its individual cooking status.
> `FireOrderLine` — `src/kitchen/domain/fire_order_line.h`

### Fire / Firing a Dish
Starting the cooking process for a dish. A cook "fires the risotto" when they begin cooking it.
> `fireDish()` command; triggers `DishFired` event.

### Plate / Plating a Dish
Completing a dish and making it ready to be served to the table.
> `plateDish()` command; triggers `DishPlated` event.

### Fire At Offset
How many minutes before service a dish should be fired, relative to other dishes in the same fire order.
> `fireAtOffsetMinutes` field on `FireOrderLine`.

### Out of Stock (Dish)
A dish that is temporarily unavailable and cannot be ordered or fired.
> `markDishOutOfStock()` command; triggers `DishMarkedOutOfStock` event, consumed by Floor to sync the menu.

### Restore (Dish)
Making an out-of-stock dish available again.
> `restoreDish()` command.

### Fire Order Status
Lifecycle of a fire order: `Coordinating` → `InProgress` → `Plated`.

### Fire Line Status
Lifecycle of a single dish within a fire order: `Waiting` → `Fired` → `Plated`.

---

## Floor Context

### Table
A physical seating location on the restaurant floor with a number and a guest capacity.
> `Table` aggregate — `src/floor/domain/table.h`

### Table Number
The human-readable label for a table (e.g. "Table 5"). Used by staff to communicate across the floor and kitchen.
> Integer field on `Table`.

### Capacity
The maximum number of guests a table can seat.
> `capacity` field on `Table`.

### Table Status
Whether a table is currently free or occupied: `Available` → `Occupied` → `Available`.

### Seat a Walk-In
Seating an unplanned party that arrives without a reservation.
> `seatWalkIn()` command; triggers `WalkInSeated` event.

### Seat a Reservation
Seating a party that booked in advance, identified by name.
> `seatReservation()` command; triggers `ReservationSeated` event.

### Turn a Table
Clearing a table after a party leaves and resetting it to available for the next guests.
> `turnTable()` command; triggers `TableTurned` event.

### Seating
A record of one party's occupancy at a table — when they sat, how many covers, walk-in or reservation.
> `Seating` aggregate — `src/floor/domain/seating.h`

### Cover
One person seated at a table. The primary unit of floor activity: "We did 85 covers tonight."
> Concept tracked via `Seating`; queried by `countCoversTonight()`.

### Tonight's Covers
The total number of guests seated during the current service period.
> `countCoversTonight()` query — `src/floor/application/floor_service.h`

### Walk-In
An unplanned arrival — a party that shows up without a reservation.
> `WalkIn` struct — `src/floor/domain/walk_in.h`

### Reservation
A planned arrival — a named party booked in advance.
> String field `reservationName` on `seatReservation()` command.

### Menu Item
A dish as presented to guests: name, description, and price. Intentionally excludes cooking details (those belong to Kitchen).
> `MenuItem` aggregate — `src/floor/domain/menu_item.h`

### Sold Out (Menu Item)
A menu item that can no longer be offered to guests.
> `markSoldOut()` command; triggers `MenuItemSoldOut` event. Also triggered automatically when Kitchen marks the corresponding dish out of stock.

---

## Finance Context

### Cost Item
An ingredient or product with a full cost structure: what it costs to make, what it costs to buy, and what it sells for.
> `CostItem` aggregate — `src/finance/domain/cost_item.h`

### Ingredient Cost
The internal per-unit cost of producing an item (labour, raw materials).
> `ingredientCostCents` field on `CostItem`.

### Supplier Price
The per-unit wholesale price paid to the supplier.
> `supplierPriceCents` field on `CostItem`.

### Selling Price
The per-unit price charged to the guest.
> `sellingPriceCents` field on `CostItem`.

### Margin
The profit percentage on a single item: `(sellingPrice − ingredientCost) / sellingPrice × 100`.
> `calculateMargin()` query.

### Waste
Spoiled, damaged, or unused inventory that must be discarded. Each waste event is recorded with a quantity, unit, and reason.
> `WasteRecord` aggregate — `src/finance/domain/waste_record.h`; `recordWaste()` command.

### Food Cost Ratio
The percentage of total revenue spent on ingredient costs across all items. A key financial health indicator.
> `FoodCostRatio` — `src/finance/domain/food_cost_ratio.h`; `foodCostReport()` query.

---

## Shared Kernel

### Id
A unique identifier for any aggregate, generated as a 32-character hex string.
> `Id` struct — `src/shared_kernel/types.h`

### Money
A monetary value stored in integer cents to avoid floating-point errors (e.g. $12.50 = 1250).
> `Money` struct — `src/shared_kernel/types.h`

### Timestamp
A point in time, stored in ISO 8601 UTC format (e.g. `2024-01-15T14:30:00Z`).
> `Timestamp` — `src/shared_kernel/types.h`

### Domain Event
A record of something that happened in the domain. Events are immutable and published after a command succeeds.
> `DomainEvent` base — `src/shared_kernel/domain_event.h`

### Event Bus
The in-process mechanism that routes domain events between bounded contexts (e.g. Kitchen → Floor).
> `EventBus` — `src/shared_kernel/event_bus.h`

---

## Alphabetical Index

| Term | Context | Section |
|------|---------|---------|
| Capacity | Floor | Tables |
| Cooking Method | Kitchen | Dishes |
| Cost Item | Finance | Cost Items |
| Cover | Floor | Seating |
| Domain Event | Shared | Shared Kernel |
| Event Bus | Shared | Shared Kernel |
| Fire / Firing | Kitchen | Fire Orders |
| Fire At Offset | Kitchen | Fire Orders |
| Fire Line Status | Kitchen | Fire Orders |
| Fire Order | Kitchen | Fire Orders |
| Fire Order Line | Kitchen | Fire Orders |
| Fire Order Status | Kitchen | Fire Orders |
| Food Cost Ratio | Finance | Reports |
| Id | Shared | Shared Kernel |
| Ingredient Cost | Finance | Cost Items |
| Margin | Finance | Cost Items |
| Menu Item | Floor | Menu |
| Money | Shared | Shared Kernel |
| Out of Stock (Dish) | Kitchen | Dishes |
| Plate / Plating | Kitchen | Fire Orders |
| Prep Time | Kitchen | Dishes |
| Reservation | Floor | Seating |
| Restore (Dish) | Kitchen | Dishes |
| Seat a Reservation | Floor | Tables |
| Seat a Walk-In | Floor | Tables |
| Seating | Floor | Seating |
| Selling Price | Finance | Cost Items |
| Sold Out (Menu Item) | Floor | Menu |
| Station | Kitchen | Dishes |
| Supplier Price | Finance | Cost Items |
| Table | Floor | Tables |
| Table Number | Floor | Tables |
| Table Status | Floor | Tables |
| Timestamp | Shared | Shared Kernel |
| Tonight's Covers | Floor | Seating |
| Turn a Table | Floor | Tables |
| Walk-In | Floor | Seating |
| Waste | Finance | Waste |
