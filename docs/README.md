# API Documentation

Generated from the Doxygen comments in `include/` via `Doxyfile` in
this folder. Published automatically to GitHub Pages by
`.github/workflows/docs.yml` on every push to `main` that touches
`include/`, `docs/`, or `README.md`.

## Generating locally

```bash
pkg install doxygen graphviz   # Termux; use your platform's package manager elsewhere
git clone --depth 1 https://github.com/jothepro/doxygen-awesome-css.git doxygen-awesome-css
doxygen Doxyfile
```

Output lands in `docs/html/index.html` — open it directly in a
browser. `doxygen-awesome-css/` isn't committed to the repo (see
`.gitignore`); both CI and the command above fetch it fresh.
