# Example: pinger-filtered

End-to-end demonstration of server-side filtered subscriptions. The program
deliberately crafts a publish sequence that crosses the filter boundary on a
single key, exercising all four cases of the broker's dispatch state machine.

# Description

Opens a filtered `View<Pinger>` with:

* row predicate: `sequence < 100`
* column projection: only `{id, sequence}` (the `message` field is dropped on
  the wire)

Publishes four Pingers all with `id=1`, varying `sequence`:

| publish | sequence | expected broker decision | observed event |
|---|---|---|---|
| 1 | 50  | not-in-view → match: **enter view** | create |
| 2 | 75  | in-view → match: **in-view update**   | update |
| 3 | 150 | in-view → no-match: **leave view**    | remove (key-only) |
| 4 | 42  | not-in-view → match: **re-enter**     | create |

Verifies the observed events match expectations. Exits with status 0 on
success, non-zero on mismatch.

# Focus

* Constructing a `View<T>` via `dots::view<T>(filter)`.
* Using the typed predicate builder (`dots::filter::attr<P> < value`).
* Combining a row predicate with a column projection mask.
* Observing the broker's enter / update / leave / re-enter transitions
  on a single keyed instance.

# Usage

Requires a running DOTS host (e.g. [dotsd](../../dotsd/README.md)) that
advertises `filteredSubscriptions` in `DotsMsgHello.capabilities` (any
dots-cpp host built from this tree).

```sh
./bin/examples/pinger-filtered/pinger-filtered
```

To target a non-default broker:

```sh
./bin/examples/pinger-filtered/pinger-filtered --dots-endpoint tcp://127.0.0.1:11235
```
