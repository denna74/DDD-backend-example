# Restaurant Management System (Domain-Driven Design)

A restaurant management system built with **Domain-Driven Design**, demonstrating bounded contexts, ubiquitous language, and hexagonal architecture. This is a working prototype based on the [DDD TED Talk transcript](../ddd-ted-talk-transcript.md).

## Purpose

Three people work in this restaurant. Each thinks about the business differently:

- **Marco** (head chef) thinks in **dishes**, **stations**, and **fire orders**. He says "fire the risotto" and "the grill station is backed up."
- **Sara** (floor manager) thinks in **tables**, **covers**, and **walk-ins**. She says "seat the walk-in at table 4" and "we did 85 covers tonight."
- **Diane** (owner) thinks in **cost items**, **waste**, and **food cost ratios**. She says "the salmon margin is 62%" and "we wasted 3kg of produce this week."

This codebase gives each of them their own model -- their own bounded context -- that speaks their language and shows only what they need.

## The Three Bounded Contexts

### Kitchen (Marco)

Marco doesn't see prices. He doesn't see descriptions written for guests. He sees **Dishes** with prep times, cooking methods, and stations. He creates **FireOrders** to coordinate timing across stations -- the risotto takes 18 minutes, so fire it before the steak.

- **Domain objects:** `Dish`, `FireOrder`, `FireOrderLine`, `Station`
- **Key operations:** `fireDish()`, `plateDish()`, `markDishOutOfStock()`, `restoreDish()`
- **Events published:** `DishMarkedOutOfStock` (consumed by Floor to mark menu items sold out)

### Floor (Sara)

Sara doesn't see prep times. She doesn't see ingredient costs. She sees **MenuItems** with names, descriptions, and prices -- what the guest sees. She manages **Tables** and seats parties as walk-ins or reservations. She tracks **Covers** to know how the night went.

- **Domain objects:** `MenuItem`, `Table`, `Seating`, `WalkIn`, `Cover`
- **Key operations:** `seatWalkIn()`, `seatReservation()`, `turnTable()`, `markSoldOut()`, `countCoversTonight()`
- **Events consumed:** `DishMarkedOutOfStock` from Kitchen

### Finance (Diane)

Diane doesn't see stations. She doesn't see cooking methods. She sees **CostItems** with ingredient costs, supplier prices, selling prices, and margins. She tracks **WasteRecords** by category and calculates the **FoodCostRatio** across the whole operation.

- **Domain objects:** `CostItem`, `WasteRecord`, `FoodCostRatio`
- **Key operations:** `recordWaste()`, `calculateMargin()`, `foodCostReport()`

## Key DDD Concepts Demonstrated

### Ubiquitous Language

The code speaks the business language. Not `updateOrderStatus("fired")` but `fireDish()`. Not `createReservation(type="UNPLANNED")` but `seatWalkIn()`. Not `toggleAvailability(false)` but `markSoldOut()`.

### Bounded Contexts

The same `items` table in SQLite is read by three different repositories and mapped to three different domain objects:

| Context | Domain Object | Fields It Sees |
|---------|--------------|----------------|
| Kitchen | `Dish` | name, prep_time_minutes, cooking_method, station, is_available |
| Floor | `MenuItem` | name, description, price_cents, is_available |
| Finance | `CostItem` | name, ingredient_cost_cents, supplier_price_cents, price_cents (selling), margin |

Each context ignores the fields it doesn't need. Marco never sees prices. Sara never sees prep times. Diane never sees stations.

### Anti-Corruption Layer

Each bounded context has its own repository that maps shared database tables to context-specific domain objects. `SqliteKitchenRepository` reads `items` and produces `Dish` objects. `SqliteFloorRepository` reads the same `items` table and produces `MenuItem` objects. The domain is protected from the storage schema.

### Domain Events

When Marco marks a dish out of stock, a `DishMarkedOutOfStock` event flows through the event bus to the Floor context, which automatically marks the corresponding menu item as sold out. Contexts communicate without sharing domain objects.

### Hexagonal Architecture

Each context follows ports-and-adapters:

```
context/
  domain/       # Pure domain objects, zero dependencies
  ports/        # Interfaces (IKitchenRepository, IKitchenEventPublisher)
  adapters/     # Implementations (SqliteKitchenRepository, HttpKitchenController)
  application/  # Service layer orchestrating domain + ports
```

The domain has no knowledge of SQLite, HTTP, or JSON. It could be tested with in-memory fakes. Adapters are swappable.

## Architecture

```
┌─────────────┐    Events    ┌─────────────┐              ┌─────────────┐
│   Kitchen    │────────────→│    Floor     │              │   Finance   │
│  (Marco)     │             │   (Sara)     │              │  (Diane)    │
│              │             │              │              │             │
│ Dish         │             │ MenuItem     │              │ CostItem    │
│ FireOrder    │             │ Table        │              │ WasteRecord │
│ Station      │             │ Seating      │              │ FoodCostRatio│
└──────┬───────┘             └──────┬───────┘              └──────┬──────┘
       │                            │                             │
       └────────────────────────────┼─────────────────────────────┘
                                    │
                            ┌───────┴───────┐
                            │  Shared DB    │
                            │  (SQLite)     │
                            │  items, orders│
                            │  tables, etc. │
                            └───────────────┘
```

## API Endpoints

### Kitchen (`/api/kitchen/`)

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/api/kitchen/dishes` | List all dishes (prep time, station, cooking method) |
| `POST` | `/api/kitchen/dishes` | Add a dish |
| `POST` | `/api/kitchen/dishes/:id/out-of-stock` | Mark dish out of stock (publishes event) |
| `POST` | `/api/kitchen/dishes/:id/restore` | Restore dish availability |
| `GET` | `/api/kitchen/fire-orders` | List all fire orders |
| `GET` | `/api/kitchen/fire-orders/:id` | Get a fire order with lines |
| `POST` | `/api/kitchen/fire-orders` | Create a fire order with timing offsets |
| `POST` | `/api/kitchen/fire-orders/:id/lines/:lineId/fire` | Fire a dish (start cooking) |
| `POST` | `/api/kitchen/fire-orders/:id/lines/:lineId/plate` | Plate a dish (done cooking) |

### Floor (`/api/floor/`)

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/api/floor/tables` | List all tables with status |
| `POST` | `/api/floor/tables` | Add a table |
| `POST` | `/api/floor/tables/:id/seat-walk-in` | Seat a walk-in party |
| `POST` | `/api/floor/tables/:id/seat-reservation` | Seat a reservation |
| `POST` | `/api/floor/tables/:id/turn` | Turn a table (mark as available after clearing) |
| `GET` | `/api/floor/menu-items` | List menu items (name, description, price) |
| `POST` | `/api/floor/menu-items` | Add a menu item |
| `PUT` | `/api/floor/menu-items/:id` | Update a menu item |
| `POST` | `/api/floor/menu-items/:id/sold-out` | Mark menu item as sold out |
| `GET` | `/api/floor/covers/tonight` | Count tonight's covers |

### Finance (`/api/finance/`)

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/api/finance/cost-items` | List cost items with margins |
| `POST` | `/api/finance/cost-items` | Add a cost item |
| `PUT` | `/api/finance/cost-items/:id` | Update cost item prices |
| `GET` | `/api/finance/cost-items/:id/margin` | Calculate margin for a specific item |
| `POST` | `/api/finance/waste` | Record a waste event |
| `GET` | `/api/finance/waste` | List all waste records |
| `GET` | `/api/finance/reports/food-cost-ratio` | Get overall food cost ratio |

## Project Structure

```
src/
  kitchen/
    domain/      dish.h, fire_order.h, fire_order_line.h, station.h, kitchen_events.h
    ports/       i_kitchen_repository.h, i_kitchen_event_publisher.h
    adapters/    sqlite_kitchen_repository, http_kitchen_controller
    application/ kitchen_service
  floor/
    domain/      menu_item.h, table.h, seating.h, walk_in.h, cover.h, floor_events.h
    ports/       i_floor_repository.h, i_floor_event_publisher.h
    adapters/    sqlite_floor_repository, http_floor_controller
    application/ floor_service
  finance/
    domain/      cost_item.h, waste_record.h, food_cost_ratio.h, finance_events.h
    ports/       i_finance_repository.h, i_finance_event_publisher.h
    adapters/    sqlite_finance_repository, http_finance_controller
    application/ finance_service
  shared_kernel/
    db.h         Database connection wrapper
    event_bus.h  In-process event bus for cross-context communication
    types.h      Shared value types (identifiers, timestamps)
  main.cpp
```

## Build & Run

```bash
mkdir -p build && cd build
cmake ..
cmake --build .
./restaurant_ddd --port 8082
```

## Run Tests

```bash
cd build
ctest --output-on-failure
```

## Docker

From the `../docker/` directory:

```bash
docker compose -f docker/docker-compose.yml up
```

The DDD server runs on port **8082**. The non-DDD server runs alongside on port 8081.

## Compare With

See [`../RESTAURANT_NO_DDD/`](../RESTAURANT_NO_DDD/) -- same database schema, same seed data, completely different architecture. The non-DDD version shows what the code looks like WITHOUT DDD: one `Item` model with all fields, generic CRUD verbs, and missing domain concepts (no waste tracking, no fire orders, no covers).
