#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

template <typename T>
struct DefaultDelete {
    void operator()(T* ptr) const { delete ptr; }
};

template <typename T>
struct DefaultDelete<T[]> {
    void operator()(T* ptr) const { delete[] ptr; }
};

template <typename T, typename Deleter = DefaultDelete<T>>
class UniquePtr {
    T* ptr = nullptr;
    Deleter deleter;

public:
    UniquePtr() noexcept = default;

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    explicit UniquePtr(T* rawPtr) noexcept : ptr(rawPtr) {}

    UniquePtr(T* rawPtr, Deleter d) noexcept : ptr(rawPtr), deleter(std::move(d)) {}

    UniquePtr(UniquePtr&& other) noexcept
        : ptr(other.ptr), deleter(std::move(other.deleter)) {
        other.ptr = nullptr;
    }

    ~UniquePtr() {
        if (ptr != nullptr) {
            deleter(ptr);
        }
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
            deleter = std::move(other.deleter);
        }
        return *this;
    }

    T& operator*() const noexcept { return *ptr; }
    T* operator->() const noexcept { return ptr; }

    explicit operator bool() const noexcept { return ptr != nullptr; }

    T* get() const noexcept { return ptr; }

    T* release() noexcept {
        T* old = ptr;
        ptr = nullptr;
        return old;
    }

    void reset(T* newPtr = nullptr) noexcept {
        if (ptr != newPtr) {
            if (ptr != nullptr) {
                deleter(ptr);
            }
            ptr = newPtr;
        }
    }
};

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
    T* ptr = nullptr;
    Deleter deleter;

public:
    UniquePtr() noexcept = default;

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    explicit UniquePtr(T* rawPtr) noexcept : ptr(rawPtr) {}

    UniquePtr(T* rawPtr, Deleter d) noexcept : ptr(rawPtr), deleter(std::move(d)) {}

    UniquePtr(UniquePtr&& other) noexcept
        : ptr(other.ptr), deleter(std::move(other.deleter)) {
        other.ptr = nullptr;
    }

    ~UniquePtr() {
        if (ptr != nullptr) {
            deleter(ptr);
        }
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
            deleter = std::move(other.deleter);
        }
        return *this;
    }

    T& operator[](std::size_t index) const { return ptr[index]; }

    explicit operator bool() const noexcept { return ptr != nullptr; }

    T* get() const noexcept { return ptr; }

    T* release() noexcept {
        T* old = ptr;
        ptr = nullptr;
        return old;
    }

    void reset(T* newPtr = nullptr) noexcept {
        if (ptr != newPtr) {
            if (ptr != nullptr) {
                deleter(ptr);
            }
            ptr = newPtr;
        }
    }
};

template <typename T, typename... Args>
std::enable_if_t<!std::is_array_v<T>, UniquePtr<T>>
makeUnique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
std::enable_if_t<std::is_array_v<T>&& std::extent_v<T> == 0, UniquePtr<T>>
makeUnique(std::size_t size) {
    return UniquePtr<T>(new std::remove_extent_t<T>[size]);
}