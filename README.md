# cpp-smart-pointers

Educational implementation of C++ smart pointers from scratch: `UniquePtr`, `SharedPtr` and `WeakPtr`. No standard smart pointers inside — only raw pointers, a control block and reference counters.

## Features

- `UniquePtr<T>` — move-only ownership, custom deleters, `release`, `reset`, `get`, `operator*`, `operator->`, `operator bool`.
- `UniquePtr<T[]>` — array specialization with `delete[]` and `operator[]`.
- `makeUnique<T>(args...)` and `makeUnique<T[]>(size)`.
- `SharedPtr<T>` — shared ownership with reference counting, copy and move, `useCount`, `reset`.
- `SharedPtr<T[]>` — array specialization with `delete[]` and `operator[]`.
- `WeakPtr<T>` — non-owning observer: `expired`, `useCount`, `lock`, `reset`.

## Files

| File | Description |
|---|---|
| `UniquePtr.h` | `UniquePtr`, `UniquePtr<T[]>`, `DefaultDelete`, `makeUnique` |
| `SharedPtr.h` | `ControlBlock`, `SharedPtrBase`, `SharedPtr`, `SharedPtr<T[]>` |
| `WeakPtr.h` | `WeakPtr` |
| `SmartPointers.h` | includes all headers |
| `SmartPointers.cpp` | small demo program with checks |

## Requirements

C++17 and one of: MSVC 2019/2022, GCC 8+, Clang 7+.

## Build

Visual Studio: open the solution and press Ctrl+F5.

Command line:

```bash
cl /EHsc /W4 /std:c++17 /Fe:demo.exe SmartPointers.cpp
```

```bash
g++ -std=c++17 -Wall -Wextra -o demo SmartPointers.cpp
```

## Example

```cpp
#include "SmartPointers.h"

int main() {
    UniquePtr<int> u(new int(42));

    SharedPtr<int> s(new int(7));
    SharedPtr<int> s2 = s;
    WeakPtr<int> w(s);

    SharedPtr<int> locked = w.lock();
    if (locked) {
        return *locked;
    }

    return 0;
}
```

## How it works

`SharedPtr` and `WeakPtr` share a `ControlBlock` that stores:

- a pointer to the object,
- `shared_counter` — number of `SharedPtr` owners,
- `weak_counter` — number of `WeakPtr` observers,
- a deleter.

When `shared_counter` reaches zero, the object is destroyed. The control block itself is destroyed only when both counters reach zero. `WeakPtr` stores the control block, not the object, so it can check whether the object is still alive and create a new `SharedPtr` with `lock()`.

## Notes

- Educational project, not a replacement for the standard library.
- Not thread-safe: counters are plain `size_t`. The standard versions use atomics.
