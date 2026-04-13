CREATE TABLE IF NOT EXISTS items (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    price_cents INTEGER NOT NULL,
    prep_time_minutes INTEGER NOT NULL,
    cooking_method TEXT NOT NULL,
    station TEXT NOT NULL,
    ingredient_cost_cents INTEGER NOT NULL,
    supplier_price_cents INTEGER NOT NULL,
    is_available INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS orders (
    id TEXT PRIMARY KEY,
    table_number INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'new',
    created_at TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS order_lines (
    id TEXT PRIMARY KEY,
    order_id TEXT NOT NULL REFERENCES orders(id),
    item_id TEXT NOT NULL REFERENCES items(id),
    status TEXT NOT NULL DEFAULT 'waiting',
    fire_at_offset_minutes INTEGER,
    fired_at TEXT,
    plated_at TEXT
);

CREATE TABLE IF NOT EXISTS tables (
    id TEXT PRIMARY KEY,
    table_number INTEGER NOT NULL UNIQUE,
    capacity INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'available'
);

CREATE TABLE IF NOT EXISTS seatings (
    id TEXT PRIMARY KEY,
    table_id TEXT NOT NULL REFERENCES tables(id),
    cover_count INTEGER NOT NULL,
    is_walk_in INTEGER NOT NULL DEFAULT 0,
    reservation_name TEXT,
    seated_at TEXT NOT NULL,
    cleared_at TEXT
);

CREATE TABLE IF NOT EXISTS waste_records (
    id TEXT PRIMARY KEY,
    item_id TEXT NOT NULL REFERENCES items(id),
    quantity REAL NOT NULL,
    unit TEXT NOT NULL,
    reason TEXT NOT NULL,
    recorded_at TEXT NOT NULL
);
