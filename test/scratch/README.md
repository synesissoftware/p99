# p99 test scratch consumers

Small CMake projects that verify **p99** can be consumed as a dependency.

## FetchContent (local checkout)

```bash
cmake -B _build -S test/scratch/consumer_fetchcontent
cmake --build _build
./_build/consumer
```

This uses `FetchContent` with `SOURCE_DIR` pointing at the repository root.
Downstream projects should pin `GIT_REPOSITORY` and `GIT_TAG` instead; see
[README.md](../../README.md#fetchcontent).
