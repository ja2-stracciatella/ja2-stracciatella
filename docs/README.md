# Docs

Technical docs directory for a developer audience.

Player-centric documentation can be found on the [project website](https://ja2-stracciatella.github.io/) and in the original *Jagged Alliance 2* game manual.

## C++ source documentation

C++ source code documentation is generated using [Doxygen](https://www.doxygen.nl).

### Installing Doxygen

Download the installer from the [official Doxygen downloads page](https://www.doxygen.nl/download.html).

Verify the install with `doxygen --version`.

### Generating the full documentation

Run this command from inside JA2 Stracciatella's `docs/` directory:

```sh
doxygen Doxyfile
```

It will generate HTML developer documentation, covering the entire `src/` C++ code.

## Rust source documentation

With the Rust toolchain installed, generate and open local documentation for all Rust workspace crates from inside JA2 Stracciatella's `docs/` directory:

```sh
cargo doc --manifest-path ../rust/Cargo.toml --target-dir docs-generated/rust --workspace --no-deps
```

Cargo writes the HTML documentation to `docs-generated/rust/doc/`, alongside the Doxygen output.
