- Emscripten
  - Make a no-args benchmark front end, for easy use from Emscripten / mobile
  - Use double rather than int64 in Emscripten (JS doesn't have real int64)
- Testing
  - Expand unit test
  - Add reproducibility test -- check for exact results
  - More benchmarks -- test division, trig functions
- Features
  - Maybe only use 31 bits, so we can add with overflow checks in 32 bits
    - Might be a slight win on arm7. Benchmark should tell us
  - Maybe add a saturating mode. Not sure how to easily integrate that though
    - Make it a separate option, separate from overflow handler func?
    - That way we could e.g. saturate and count occurrences
  - Real fixed-point implementation of `math.h`
    - This is a lot of work! Can be done incrementally, though.
    - May not be needed for bitwise reproducible results. Write tests first.
