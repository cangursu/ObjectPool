# Object Pool

A thread-safe template library that implements an object pooling pattern to efficiently reuse initialized objects such as database connections, network sockets, or other resource-intensive objects.

## Features

- Template-based implementation supporting any object type
- Thread-safe object acquisition and release
- Automatic resource management with RAII pattern through `ObjectPoolLock`
- Configurable pool size
- Support for custom initialization parameters
- Comprehensive unit test coverage

## Requirements

- C++17 or later
- CMake 3.12 or later
- GTest (for running tests)
- pthread library

## Building the Project

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Usage Example

Here's a basic example of how to use the Object Pool:

```cpp
// Define your pooled object class
class DatabaseConnection {
public:
    bool Init(const ParamList& params) {
        // Initialize your database connection
        return true;
    }

    void Release() {
        // Cleanup resources
    }
};

// Create and initialize the pool
ObjectPool<DatabaseConnection> pool(8); // Pool size of 8
ParamList params = {
    {"Host", "localhost"},
    {"Port", "5432"}
};
pool.Init(params);

// Use the pool with RAII pattern
{
    ObjectPoolLock<DatabaseConnection> conn(pool);
    // Use conn as a pointer or reference to DatabaseConnection
    // Connection will be automatically returned to pool when leaving scope
}

// Or manually manage object lifetime
DatabaseConnection* conn = pool.ObjectAcquire();
if (conn) {
    // Use the connection
    pool.ObjectRelease(conn);
}
```

## API Reference

### `ObjectPool<T>`

Main template class for managing the object pool.

#### Methods:
- `ObjectPool(size_t size = ObjectPoolCountDefault)`: Constructor with configurable pool size
- `Init(const Args& ... args)`: Initialize pool objects with provided arguments
- `Release()`: Release all resources
- `ObjectAcquire()`: Get an object from the pool (blocks if none available)
- `ObjectRelease(T* p)`: Return an object to the pool
- `Size()`: Get pool size
- `Close(bool v)`: Control pool availability

### `ObjectPoolLock<T>`

RAII wrapper for automatic object management.

#### Usage:
```cpp
{
    ObjectPoolLock<MyObject> obj(pool);
    // Use obj-> or *obj to access the pooled object
    // Object automatically returned to pool when obj goes out of scope
}
```

## Running Tests

```bash
# Build and run tests
cd build
ctest --output-on-failure
```

## Author

Can Gürsu
