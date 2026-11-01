# CHANGE LOG

## 2026-11-01

- TLS mutual-certificate verification now honors OpenSSL's `preverify_ok` result instead of accepting every certificate.
- Session IDs use an atomic counter; updating a session key now updates its existing entry rather than appending duplicate owned pointers.
- Transaction serialization uses `std::mutex` and `std::lock_guard`, so exceptions cannot leave the global lock permanently set. The transaction's handler pointer is initialized, and UPDATE filter values are escaped.
- The database pool protects its instance collection with a mutex, uses an atomic shutdown flag, stops and joins its worker before destroying pooled instances, and marks a reused instance as in-use.
- MySQL INSERT result ownership no longer double-frees `MYSQL_RES`; INSERT statements without a result set still return the generated insert ID.
- Linux socket close is idempotent, the invalid descriptor sentinel is `-1`, connection probing distinguishes EOF from a live connection, and socket writes handle partial sends and `EINTR`.
- Binary file reads use `rb`, close the file on all normal paths, check seek/read results, and handle empty files safely. Static-file response construction no longer indexes an empty vector.
- The server's thread limit is enforced, thread counts are atomic, rejected sockets are handled, thread-creation failures clean up resources, and the active-node collection is protected while modified.
- Compiler warnings are no longer globally suppressed; `-Wall -Wextra -Wpedantic` is enabled.
- Replaced NUL-terminated body accumulation with length-aware reads. Binary bodies and embedded NUL bytes are preserved.
- Added strict decimal `Content-Length` parsing, overflow checks, duplicate framing-header rejection, and rejection of `Content-Length` combined with `Transfer-Encoding`.
- Added bounded request headers (32 KiB total, 8 KiB per line, 100 fields) and a bounded request body (16 MiB by default).
- Added HTTP/1.0 and HTTP/1.1 request-line validation; HTTP/1.1 requests require `Host`.
- Added chunked request-body decoding, including chunk terminators and bounded trailer consumption, a chunk-count limit, and a framing-byte limit. Duplicate Host headers and NUL bytes in headers are rejected. Unsupported transfer encodings are rejected.
- Malformed requests receive HTTP 400; oversized requests receive HTTP 413.
- Uses thread-safe UTC date formatting for response headers.
- Client workers are detached but tracked through an atomic active-worker count and a condition variable. Shutdown stops accepting connections and waits for all active workers to finish, without retaining an unbounded vector of completed thread IDs.
- Fixed the shutdown-wakeup connection accounting so it does not leave the active-worker count permanently elevated.
- Removed the worker-package race where the parent thread wrote to a package after the worker could already have freed it.
- Worker accounting is decremented on normal and exceptional exits.
- `accept()` now retries interrupted system calls but returns on other errors instead of looping indefinitely.
- MySQL transactions now issue `START TRANSACTION`, commit with `mysql_commit`, and roll back if destroyed without a successful commit.
- Reworked CMake to use imported OpenSSL, Threads, and pkg-config targets. Backend compile definitions and link dependencies propagate from the library to consumers.
- `USE_MYSQL` and `USE_PQXX` are mutually exclusive. Redis remains independently selectable.
- Fixed the sample server's MySQL compile-definition typo by removing its duplicate manual dependency setup; it inherits the library's public compile definitions and link requirements.
- The sample server accepts an optional listen-port argument, which makes isolated integration testing possible.
- Fixed `MResponse` content-type constructor handling and added common HTTP status descriptions, including 413.
