# ifmetrica

Collect network interface usage metrics (such as rx/tx bytes and packets) every second and insert it to clickhouse with wider interval (customizeable). Can buffer metrics if clickhouse unavailable or returning errors (buffer size also customizeable).

Requirements:
- c++23 for compile
- linux only for run

Build steps:
1. Checkout git submodules (clockhouse-cpp and nlohman-json required for build)
2. `mkdir cmake-build && cd cmake-build && cmake .. && make -j8`
3. Grab `ifmetrica` binary and palce whatever you want.

Run steps:
1. Copy `ifmetrica.json.example` file to `ifmetrica.json`
2. Setup clickhouse connection and other settings
3. Create table in clickhouse, ddl in `metrics.sql`
4. Run `ifmetrica path/to/ifmetrica.json`.
