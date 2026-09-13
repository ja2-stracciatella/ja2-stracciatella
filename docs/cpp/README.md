# C++ source docs

Code-specific C++ documentation is generated using [Doxygen](https://www.doxygen.nl).

### Installing Doxygen

- **macOS (Homebrew):** `brew install doxygen`
- **Debian/Ubuntu:** `sudo apt-get install doxygen`
- **Windows:** Install via [Chocolatey](https://chocolatey.org/) with `choco install doxygen.install`, or download the installer from the [official Doxygen downloads page](https://www.doxygen.nl/download.html).
- Alternatively, download a release archive or build from source directly from the [official Doxygen downloads page](https://www.doxygen.nl/download.html).

Verify the install with `doxygen --version`.

### Generating the full documentation

Run this command from this `docs/cpp/` directory:

```sh
doxygen Doxyfile.full
```

It will generate the full HTML developer documentation, covering the entire engine source tree.

#### Alternative, modder-focused code docs

Instead of the above full-sized documentation, a smaller subset of the documentation can be generated, focusing only on the C++ API reachable from the Lua scripting extension:

```sh
doxygen Doxyfile.mod-api-only
```
