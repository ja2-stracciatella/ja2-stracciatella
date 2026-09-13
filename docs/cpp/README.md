# C++ source docs

## Generating code documentation with Doxygen

Code-specific C++ documentation is generated using [Doxygen](https://www.doxygen.nl).

### Installing Doxygen

- **macOS (Homebrew):** `brew install doxygen`
- **Debian/Ubuntu:** `sudo apt-get install doxygen`
- **Windows:** Install via [Chocolatey](https://chocolatey.org/) with `choco install doxygen.install`, or download the installer from the [official Doxygen downloads page](https://www.doxygen.nl/download.html).
- Alternatively, download a release archive or build from source directly from the [official Doxygen downloads page](https://www.doxygen.nl/download.html).

Verify the install with `doxygen --version`.

### Generating the documentation

Run these commands from this `docs/cpp/` directory.

Full developer documentation, covering the entire engine source tree:

```sh
doxygen Doxyfile.full
```

Modder-focused documentation, covering only the C++ API reachable from the Lua scripting extension:

```sh
doxygen Doxyfile.mod-api-only
```

By default, both commands write their output into `docs-generated/` directory, but you can change this by modifying the `OUTPUT_DIRECTORY` setting in `Doxyfile.full` or `Doxyfile.mod-api-only`, or by setting `OUTPUT_DIRECTORY` directly in the command line.
```sh
doxygen Doxyfile.full --output-directory <output_directory>
```
