# Solusek C++ HTTP Server

## Dependencies

For `-DUSE_OPENSSL` you need `libssl`.

On Ubuntu:
```
sudo apt-get install libssl-dev
```

On RedHat:
```
sudo yum install openssl-devel
```

On Alpine:
```
sudo apt add openssl-dev
```

For `-DUSE_MYSQL` you need `libmariadb`.

On Ubuntu:
```
sudo apt-get install libmariadb-dev
```

On RedHat:
```
sudo yum install mariadb-shared
```

For `-DUSE_PQXX` you need `libpqxx`

On Ubuntu:
```
sudo apt-get install libpqxx-dev
```

If neither `-DUSE_MYSQL` or `-DUSE_PQXX` is defined the database layer will be defined empty (for applications that don't use MySQL nor PostgreSQL this is recommended.)

## To build library

```
mkdir build
cd build
cmake ../src/
make
sudo make install
sudo ldconfig
```

I still haven't added a make install so you might want to copy the library so to your /usr/local/lib/ directory.

## Examples

I included an example app called sample-server:
```
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
http://www.solusek.com/
