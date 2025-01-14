# Build Instructions for Linux

## Requirements

To build the project on Linux, you'll need the following dependencies:

- **CMake**: To configure and build the project.
- **GCC**: The default compiler for Linux.
- **MinGW**: If you're cross-compiling for Windows, you'll need MinGW tools. Otherwise, you can skip this step.
- **Make**: To build the project after configuring it with CMake.
- **Python**: For serving files via HTTP (optional).

### Installing Dependencies on Ubuntu/Debian

Use the following command to install the required dependencies on Ubuntu/Debian-based systems:

```bash
sudo apt update
sudo apt install cmake gcc g++ make python3
```

If you're cross-compiling for Windows (using MinGW), install the necessary MinGW tools:

```bash
sudo apt install mingw-w64
```

## Building the Project

### Configure the Build

Use CMake to configure the build. Depending on your architecture, use one of the following configurations:

- **For a Release build:**

```bash
make release
```

- **For a Debug build:**

```bash
make debug
```

- **For a Release with Debug Info build:**

```bash
make release_debug
```

- **For a MinSizeRelease build:**

```bash
make minsize_release
```

Each configuration step will trigger the respective CMake configuration and build commands, as defined in the
`Makefile`.

### 3. Clean the Build

To clean up the build directory, use:

```bash
make clean
```

This will remove all previously generated files and allow you to rebuild the project from scratch.

### 4. Rebuild the Project

If you need to rebuild the project after cleaning, use:

```bash
make rebuild_release
```

or

```bash
make rebuild_debug
```

### 5. Serve the Debug Output (Optional)

To serve the debug output via an HTTP server:

```bash
make serve
```

This will serve the contents of the build directory on port 4444 using Python's built-in HTTP server.
