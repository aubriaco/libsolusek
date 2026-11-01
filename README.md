# libsolusek

**libsolusek** is a C++ library designed to embed an HTTP/HTTPS server directly into your applications. It provides a lightweight, flexible foundation for building web services, REST APIs, and network-enabled applications without requiring a separate web server.

The library includes HTTP request and response handling, SSL/TLS support, session management, database connectivity, and caching capabilities.

## Key Features

- **Embedded HTTP/HTTPS Server** – Run a web server directly within your C++ application.
- **SSL/TLS Support** – Secure connections using OpenSSL.
- **Database Integration** – Support for PostgreSQL and MySQL/MariaDB.
- **Redis Integration** – Caching and session-related functionality.
- **Session Management** – Manage application sessions and state.
- **Multithreaded Processing** – Handle concurrent client connections.
- **CMake Build System** – Build and integrate the library into Linux-based applications.

## Use Cases

Ideal for embedded web interfaces, REST APIs, backend services, administration portals, and applications requiring integrated HTTP/HTTPS functionality.

## Dependencies

For `-DUSE_OPENSSL` you need `libssl`.

On Ubuntu:

```bash
sudo apt-get install libssl-dev
```

On RedHat:

```bash
sudo yum install openssl-devel
```

On Alpine:

```bash
sudo apt add openssl-dev
```

For `-DUSE_MYSQL` you need `libmariadb`.

On Ubuntu:

```bash
sudo apt-get install libmariadb-dev
```

On RedHat:

```bash
sudo yum install mariadb-shared
```

For `-DUSE_PQXX` you need `libpqxx`

On Ubuntu:

```bash
sudo apt-get install libpqxx-dev
```

If neither `-DUSE_MYSQL` or `-DUSE_PQXX` is defined the database layer will be defined empty (for applications that don't use MySQL nor PostgreSQL this is recommended.)

## To build library

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

Post-build: Only on Ubuntu:

```bash
sudo ldconfig
```

I still haven't added a make install so you might want to copy the library so to your /usr/local/lib/ directory.

## Examples

I included an example app called sample-server:

```c++
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

     // Static file mount
     g_Server->addStaticDirectory("../static/");

     g_Server->registerStaticIndex("index.html");

     // API endpoint example
     g_Server->registerEndpoint(new solusek::MEndpoint("/api/cookie", cookieEndpoint, "GET"));

     g_Server->run();

     g_Server->dispose();

     return 0;
}
```

## Credits

Created by Alessandro Ubriaco

<https://alessandroubriaco.com/>

<https://solusek.com/>
