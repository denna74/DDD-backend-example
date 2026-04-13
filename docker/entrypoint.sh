#!/bin/bash
set -e

DATA_DIR="${DATA_DIR:-/app/data}"
mkdir -p "$DATA_DIR"

if [ ! -f "$DATA_DIR/no_ddd.db" ]; then
    echo "Initializing non-DDD database..."
    sqlite3 "$DATA_DIR/no_ddd.db" < /app/no_ddd/db/schema.sql
    sqlite3 "$DATA_DIR/no_ddd.db" < /app/no_ddd/db/seed.sql
fi

if [ ! -f "$DATA_DIR/ddd.db" ]; then
    echo "Initializing DDD database..."
    sqlite3 "$DATA_DIR/ddd.db" < /app/ddd/db/schema.sql
    sqlite3 "$DATA_DIR/ddd.db" < /app/ddd/db/seed.sql
fi

echo "Starting non-DDD server on port 8081..."
cd /app/no_ddd && /app/restaurant_no_ddd --db "$DATA_DIR/no_ddd.db" --port 8081 &

echo "Starting DDD server on port 8082..."
cd /app/ddd && /app/restaurant_ddd --db "$DATA_DIR/ddd.db" --port 8082
