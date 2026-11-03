# libsolusek

**libsolusek** is a C++ library for embedding an HTTP/HTTPS server directly into an application. It provides HTTP request and response handling, static-file serving, sessions, and optional database and caching integrations—without requiring a separate web-server process.

Use it to build local administration interfaces, REST-style endpoints, internal services, or other network-enabled C++ applications.

## Features

- **Embedded HTTP/HTTPS server** — Host endpoints inside your application.
- **TLS support** — Secure connections using OpenSSL.
- **HTTP routing** — Register endpoints with handlers and HTTP methods.
- **Static content** — Serve files from configured directories and specify an index file.
- **Sessions and cookies** — Maintain session state and set response cookies.
- **Concurrent connections** — Process multiple clients.
- **Optional database backends** — Enable MySQL/MariaDB or PostgreSQL (`libpqxx`) support at build time.
- **Optional Redis support** — Enable Redis integration with `hiredis`.
- **CMake build** — Build the shared library and bundled sample server.

## Requirements

- A C++ compiler and CMake
- OpenSSL development libraries (required by the current CMake configuration)
- Optional development libraries for any enabled database or Redis backend

### Install dependencies

**Debian / Ubuntu**

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev
```

For optional integrations:

```bash
sudo apt-get install -y libmariadb-dev libpqxx-dev libhiredis-dev
```

**Fedora / RHEL-family distributions**

```bash
sudo dnf install cmake gcc-c++ make openssl-devel
```

Install the corresponding MySQL/MariaDB, `libpqxx`, or `hiredis` development packages if enabling those integrations. Package names vary by distribution and release.

**Alpine Linux**

```bash
sudo apk add build-base cmake openssl-dev
```

## Build

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The project builds a shared library and the bundled `sample-server-test` example.

### Windows x64 (MSVC)

Install Visual Studio 2022 or its Build Tools with **Desktop development with C++**,
CMake, and vcpkg. Set `VCPKG_ROOT` to your bootstrapped vcpkg directory.
Run these commands in PowerShell from the repository root:

```powershell
& "$env:VCPKG_ROOT/vcpkg.exe" install openssl:x64-windows
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build --config Release --parallel
Push-Location build
.\Release\sample-server-test.exe 8080
Pop-Location
```

From another terminal, run `curl.exe http://127.0.0.1:8080/api/cookie/set`.
The response body should be `Cookie set!`. Launching the sample with
`build` as the working directory also enables its relative static-file paths.

The Windows build uses Winsock. Keep `solusek.dll` and the matching OpenSSL
runtime DLLs beside your application or on its DLL search path. Use a fresh build
directory when changing the CMake generator, architecture, or vcpkg toolchain.

The default Windows build requires only OpenSSL. For PostgreSQL or Redis, install
`libpqxx:x64-windows` or `hiredis:x64-windows` and enable the corresponding
CMake option below. MySQL/MariaDB uses the existing pkg-config discovery path.

### Optional backends

The following CMake variables enable additional integrations:

| Variable | Backend | Required development library |
| --- | --- | --- |
| `USE_MYSQL` | MySQL / MariaDB | MySQL-compatible client library |
| `USE_PQXX` | PostgreSQL | `libpqxx` |
| `USE_REDIS` | Redis | `hiredis` |

For example, to enable PostgreSQL and Redis:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DUSE_PQXX=ON -DUSE_REDIS=ON
cmake --build build --parallel
```

If neither `USE_MYSQL` nor `USE_PQXX` is enabled, the library builds without a database backend. OpenSSL is enabled by the current build configuration; `USE_OPENSSL` is not currently an optional CMake switch.

### Install

The repository's CMake configuration includes install rules for the library and public headers:

```bash
sudo cmake --install build
sudo ldconfig  # Linux systems using ldconfig, such as Debian/Ubuntu
```

The existing install rules target `/usr/local/lib` and `/usr/local/include`. Review them before using a custom installation prefix or packaging the library for a distribution.

## Example: embedded HTTP server

The repository includes a sample application in `sample-server/`. The following example registers an endpoint that sets a cookie and serves static content:

```cpp
#include <solusek/solusek.h>

static solusek::IServer *g_Server;

void interruptCallback(int sig)
{
    printf("Interrupt signal called.\n");
    g_Server->stop();
}

solusek::MResponse cookieEndpoint(const void *sobj, const solusek::MRequest& data)
{
    solusek::IServer *server = (solusek::IServer*)sobj;

    solusek::MResponse resp;
    solusek::MCookie cookie("test", "value");
    cookie.Path = "/";
    resp.Cookies.push_back(cookie);

    resp.Body = "Cookie set!";

    return resp;
}

int main(int argc, char **argv)
{
    g_Server = solusek::createServer();
    g_Server->setInterruptCallback(interruptCallback);
    g_Server->setListenPort(8080);

    // Static files and default index
    g_Server->addStaticDirectory("../static/");
    g_Server->registerStaticIndex("index.html");

    // HTTP GET /api/cookie
    g_Server->registerEndpoint(
        new solusek::MEndpoint("/api/cookie", cookieEndpoint, "GET"));

    g_Server->run();
    g_Server->dispose();
    return 0;
}
```

The relative static-directory path is resolved from the application's working directory. Adjust it to match your deployment layout.

After starting the example, test the registered endpoint:

```bash
curl -i http://localhost:8080/api/cookie
```

The expected response body is `Cookie set!`, with a cookie named `test`.

## Project structure

- `src/` — Library implementation and public headers.
- `sample-server/` — Example embedded server.
- `CMakeLists.txt` — Build and installation configuration.

## Credits

Created by **Alessandro Ubriaco**.

- [alessandroubriaco.com](https://alessandroubriaco.com/)
- [solusek.com](https://solusek.com/)
