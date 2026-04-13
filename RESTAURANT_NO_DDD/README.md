# Restaurant Management System (Non-DDD)

A traditional CRUD-style restaurant management system. This is the **baseline** for comparison with the [DDD version](../RESTAURANT_DDD/) -- it shows what happens when you build around generic technical concepts (items, orders, tables, reservations) instead of the domain.

## Purpose

This project deliberately uses the **wrong language**. Based on the [DDD TED Talk transcript](../ddd-ted-talk-transcript.md), a restaurant has three distinct worlds: the kitchen (Marco), the floor (Sara), and finance (Diane). This codebase ignores those boundaries and lumps everything into a single flat architecture.

The result:

- Walk-ins are called "reservations" with `type: UNPLANNED`. Sara says "seat the walk-in" but the code says `createReservation(type="UNPLANNED")`.
- Firing a dish is called "updating order status." Marco says "fire the risotto" but the code says `updateOrderStatus(id, "fired")`.
- There is one `Item` model with 10 fields that serves everyone and satisfies no one.

## The Problem

**Marco** (head chef) gets `price_cents`, `description`, and `supplier_price_cents` -- fields he never uses. He needs to think in dishes, stations, and fire timing. Instead he gets a generic "item."

**Sara** (floor manager) gets `prep_time_minutes`, `cooking_method`, and `station` -- fields she doesn't care about. She needs menu items with names, descriptions, and prices for guests. Instead she gets the same generic "item."

**Diane** (owner/finance) can't see food waste. The `waste_records` table exists in the schema, but there is no dedicated model, no service, and no API endpoint for it. Waste is invisible to the application.

## Architecture

```
src/
  models/           # item.h, order.h, order_line.h, table.h, reservation.h
  repositories/     # item_repository, order_repository, table_repository, reservation_repository
  services/         # item_service, order_service, table_service, reservation_service
  controllers/      # item_controller, order_controller, table_controller, reservation_controller
  main.cpp
  db.h / db.cpp
```

Flat layers: Model -> Repository -> Service -> Controller. One `Item` model carries all 10 fields (id, name, description, price_cents, prep_time_minutes, cooking_method, station, ingredient_cost_cents, supplier_price_cents, is_available) regardless of who is consuming it.

## API Endpoints

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/api/items` | List all items (all 10 fields) |
| `POST` | `/api/items` | Create an item |
| `PUT` | `/api/items/:id` | Update an item |
| `PATCH` | `/api/items/:id/availability` | Toggle item availability |
| `GET` | `/api/orders` | List all orders with lines |
| `POST` | `/api/orders` | Create an order |
| `PUT` | `/api/orders/:id/status` | Update order status (generic string) |
| `GET` | `/api/tables` | List all tables |
| `POST` | `/api/tables` | Create a table |
| `PUT` | `/api/tables/:id/status` | Update table status (generic string) |
| `GET` | `/api/reservations` | List all reservations |
| `POST` | `/api/reservations` | Create a reservation (walk-ins are `type: UNPLANNED`) |
| `PUT` | `/api/reservations/:id` | Update a reservation |

Notice: no waste endpoints. No fire-dish action. No cover count. No food cost ratio. The domain concepts are missing because the architecture doesn't model the domain.

## Build & Run

```bash
mkdir -p build && cd build
cmake ..
cmake --build .
./restaurant_no_ddd --port 8081
```

## Run Tests

```bash
cd build
ctest --output-on-failure
```

## Docker

From the `docs/` directory:

```bash
docker compose -f docker/docker-compose.yml up
```

The non-DDD server runs on port **8081**.

## Compare With

See [`../RESTAURANT_DDD/`](../RESTAURANT_DDD/) -- same database schema, same seed data, completely different architecture. The DDD version speaks the business language and gives each person (Marco, Sara, Diane) exactly the model they need.
