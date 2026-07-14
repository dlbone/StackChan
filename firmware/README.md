
## Build

### Fetch Dependencies

```bash
python3 ./fetch_repos.py
```

### Tool Chains

[ESP-IDF v5.5.4](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/index.html)

### Build

```bash
idf.py build
```

### Codex Desktop Glimble screen

While the AI Agent app is in standby, it polls
`GET /stackchan/codex-pet` once per second and always shows Glimble. An active
Codex Desktop task selects the matching animation:

- `running`: working animation
- `needs_input`: waiting animation
- `ready`: waving animation
- `blocked`: failed animation

Listening or speaking hides Glimble immediately. Tapping Glimble starts a voice
turn. The original Codex v2 pet atlas is converted into the embedded PNG frames
with:

```bash
python3 scripts/extract_glimble_frames.py \
  ~/.codex/pets/glimble/spritesheet.webp \
  main/assets/assets_bin
```

### Host-side tests

The motion coordinate helpers can be tested without ESP-IDF hardware:

```bash
cmake -S tests -B build-host-tests
cmake --build build-host-tests
ctest --test-dir build-host-tests --output-on-failure
```

### Flash

```bash
idf.py flash
```
