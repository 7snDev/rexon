# REXON

![REXON Logo](./icon/logo.png)

**REXON** is a lightweight, cross-platform C++ backend library designed to handle basic HTTP routing for both **Windows** and **Linux**. It provides a simple, modern API for creating backend applications with support for GET and POST requests, custom 404 handlers, and optional logging.

---

## Features

- 🖥️ Cross-platform: Works on both Windows and Linux  
- 🔌 Built-in HTTP server  
- 🧭 Route handling for `GET` and `POST`  
- 📜 Simple response customization (status, content-type, body)  
- 🛠️ Custom 404 (Not Found) handler  
- 🧩 Utility to extract URL query attributes  
- 🪵 Optional request logging  
- 🔓 MIT Licensed  

---

## Getting Started

### 1. Include the Header

Include the `REXON.h` file in your C++ project:

```cpp
#include "REXON.h"
```

### 2. Create a Simple API

```cpp
#include "REXON.h"

int main() {
    REXON::api server(8080);

    server.GET("/", [](std::string) {
        return REXON::Response("200 OK", "text/plain", "Welcome to REXON!");
    });

    server.POST("/data", [](std::string body) {
        return REXON::Response("200 OK", "application/json", "{\"received\":\"" + body + "\"}");
    });

    server.NOTFOUND([](std::string) {
        return REXON::Response("404 Not Found", "text/plain", "This route does not exist.");
    });

    server.show_log(true);
    server.start();
    return 0;
}
```

---

## API Overview

### Class: `REXON::api`

| Method                  | Description                                       |
|------------------------|---------------------------------------------------|
| `api(int port)`        | Create server listening on specified port         |
| `start()`              | Start the server                                  |
| `stop()`               | Stop the server                                   |
| `GET(path, callback)`  | Register a GET route with a callback              |
| `POST(path, callback)` | Register a POST route with a callback             |
| `NOTFOUND(callback)`   | Handle undefined routes                           |
| `show_log(bool)`       | Enable/disable logging of client connections      |

### Class: `REXON::Response`

| Method                  | Description                             |
|------------------------|-----------------------------------------|
| `set_status()`         | Set HTTP status code (e.g. `"200 OK"`)  |
| `set_content_type()`   | Set MIME type (e.g. `"text/plain"`)     |
| `set_body()`           | Set the response body                   |
| `get_status()`         | Get the status                          |
| `get_content_type()`   | Get the content type                    |
| `get_body()`           | Get the response body                   |

### Utility Function

```cpp
std::string REXON::get_attribute(std::string query, std::string key);
```

Extract a value from a query string (e.g. `?id=123&name=John` → key: `"id"` → `"123"`).

---

## Platform Notes

- On **Windows**, REXON uses Winsock (requires linking with `ws2_32.lib`)  
- On **Linux**, it uses standard POSIX sockets  

> Platform-specific details are handled automatically by the library.

---

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

---

## Contributing

Contributions are welcome! Feel free to submit pull requests or report issues.

---

## Author

Made with ❤️ in C++ by **7sn**  
GitHub: [@7sndev](https://github.com/7sndev)  
Repo: [7sndev/rexon](https://github.com/7sndev/rexon)
