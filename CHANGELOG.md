# 2026-11-01

- TLS mutual-certificate verification now honors OpenSSL's `preverify_ok` result instead of accepting every certificate.
- Session IDs use an atomic counter; updating a session key now updates its existing entry rather than appending duplicate owned pointers.
- Transaction serialization uses `std::mutex` and `std::lock_guard`, so exceptions cannot leave the global lock permanently set. The transaction's handler pointer is initialized, and UPDATE filter values are escaped.
- The database pool protects its instance collection with a mutex, uses an atomic shutdown flag, stops and joins its worker before destroying pooled instances, and marks a reused instance as in-use.
- MySQL INSERT result ownership no longer double-frees `MYSQL_RES`; INSERT statements without a result set still return the generated insert ID.
- Linux socket close is idempotent, the invalid descriptor sentinel is `-1`, connection probing distinguishes EOF from a live connection, and socket writes handle partial sends and `EINTR`.
- Binary file reads use `rb`, close the file on all normal paths, check seek/read results, and handle empty files safely. Static-file response construction no longer indexes an empty vector.
- The server's thread limit is enforced, thread counts are atomic, rejected sockets are handled, thread-creation failures clean up resources, and the active-node collection is protected while modified.
- Compiler warnings are no longer globally suppressed; `-Wall -Wextra -Wpedantic` is enabled.


