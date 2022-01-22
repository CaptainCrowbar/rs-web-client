# Web Client

_[Web Client Library by Ross Smith](index.html)_

```c++
#include "web-client/client.hpp"
namespace RS::WebClient;
```

## Contents

* TOC
{:toc}

## Supporting types

```c++
class CurlError: public std::runtime_error {
    int error() const noexcept;
};
```

Exception used for passing on errors returned by the Libcurl API.

```c++
using Headers = std::unordered_multimap<std::string, std::string>;
```

A map used to record HTTP headers.

## Client class

```c++
class Client;
```

The main web client class. This class is a C++ wrapper around Libcurl's API.

(Currently not very complete.)

```c++
static constexpr [duration type] default_connect_timeout = 15s;
static constexpr [duration type] default_request_timeout = 60s;
static constexpr int default_redirect_limit = 10;
```

Default values for client settings.

```c++
Client();
```

The constructor initializes the Libcurl API, and may throw `CurlError` if
anything goes wrong.

```c++
~Client() noexcept;
Client(Client&& c) noexcept;
Client& operator=(Client&& c) noexcept;
```

Other life cycle functions. `Client` objects are not copyable.

```c++
HttpStatus http_get(const IO::Uri& uri, std::string& body);
HttpStatus http_get(const IO::Uri& uri, Headers& head, std::string& body);
HttpStatus http_head(const IO::Uri& uri, Headers& head);
```

Perform an HTTP(S) request. The `head` and `body` arguments, if present, will
be filled with the response headers and body. These may throw `CurlError` if
anything goes wrong internally, but actual HTTP errors will be returned as
status codes. If anything goes wrong, either by throwing an exception or
returning an error code, `head` and `body` will be empty.

```c++
template <typename R, typename P> void set_connect_timeout(std::chrono::duration<R, P> t);
template <typename R, typename P> void set_request_timeout(std::chrono::duration<R, P> t);
```

Set timeouts for the initial connection and the complete request. These may
throw `CurlError`.

```c++
void set_redirect_limit(int n);
```

Set the maximum number of redirections before giving up. This may throw
`CurlError`.

```c++
void set_user_agent(const std::string& user_agent);
```

Set the user agent string. This may throw `CurlError`.

```c++
Curl_easy* native_handle() const noexcept;
```

Returns the underlying Libcurl API handle.

## Progress callback class

```c++
class OnProgress {
    using callback = std::function<bool(int64_t dltotal, int64_t dlnow)>;
    OnProgress(Client& c, callback on_download);
    ~OnProgress() noexcept;
};
```

Add a progress recording function by constructing an `OnProgress` object on a
`Client`. The constructor may throw `CurlError`.

This keeps a reference to the client; behaviour is undefined if the client is
destroyed while it has any progress trackers attached to it.

The callback is passed the expected total and the current download byte count.
The expected total is often only an estimate, and may be unreliable or
unavailable; the callback should be prepared to handle the possibility that
`dltotal` may be zero or less than `dlnow`.
