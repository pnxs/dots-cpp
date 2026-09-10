# Example: pinger

Counterpart to the `examples/pinger` in [dots-go](https://github.com/pnxs/dots-go),
intended for cross-language performance comparison against the same `dotsd` broker.

# Description

The application subscribes to the `Pinger` type, publishes `--loops` instances
(all with key `id=1`, varying `message`/`sequence`), and measures wall-clock
time until it has received its own echoes back from the broker.

Both the dots-cpp and dots-go pingers use the same model, the same fixed key,
and the same publish/drain semantics so the reported `msg/s` rate can be
compared apples-to-apples.

# Usage

Requires a running DOTS host (e.g. [dotsd](../../dotsd/README.md)).

```sh
./bin/examples/pinger/pinger --loops 10000
```

To target a non-default broker:

```sh
./bin/examples/pinger/pinger --dots-endpoint tcp://127.0.0.1:11235 --loops 10000
```
