# C++ source docs

C++ source code documentation is generated using [Doxygen](https://www.doxygen.nl).

### Installing Doxygen

- **macOS (Homebrew):** `brew install doxygen`
- **Debian/Ubuntu:** `sudo apt-get install doxygen`
- **Windows:** Install via [Chocolatey](https://chocolatey.org/) with `choco install doxygen.install`, or download the installer from the [official Doxygen downloads page](https://www.doxygen.nl/download.html).
- Alternatively, download a release archive or build from source directly from the [official Doxygen downloads page](https://www.doxygen.nl/download.html).

Verify the install with `doxygen --version`.

### Generating the full documentation

Run this command from the `ja2-stracciatella/docs/cpp/` directory:

```sh
doxygen Doxyfile
```

It will generate the full HTML developer documentation, covering the entire engine source tree.
