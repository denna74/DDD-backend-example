# Build stage
FROM gcc:14 AS builder

RUN apt-get update && apt-get install -y cmake && rm -rf /var/lib/apt/lists/*

WORKDIR /build

COPY RESTAURANT_NO_DDD/ /build/RESTAURANT_NO_DDD/
COPY RESTAURANT_DDD/ /build/RESTAURANT_DDD/

# Build non-DDD
WORKDIR /build/RESTAURANT_NO_DDD
RUN mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . -j$(nproc)
RUN cd build && ctest --output-on-failure

# Build DDD
WORKDIR /build/RESTAURANT_DDD
RUN mkdir -p build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . -j$(nproc)
RUN cd build && ctest --output-on-failure

# Runtime stage
FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y sqlite3 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/RESTAURANT_NO_DDD/build/restaurant_no_ddd /app/restaurant_no_ddd
COPY --from=builder /build/RESTAURANT_DDD/build/restaurant_ddd /app/restaurant_ddd

COPY RESTAURANT_NO_DDD/db/ /app/no_ddd/db/
COPY RESTAURANT_DDD/db/ /app/ddd/db/

COPY docker/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh

EXPOSE 8081 8082

ENTRYPOINT ["/app/entrypoint.sh"]
