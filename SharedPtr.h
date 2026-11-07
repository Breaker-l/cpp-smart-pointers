#pragma once

#include <cstddef>

template <typename T>
class WeakPtr;

template <typename T>
struct ControlBlock {
    T* value;
    std::size_t shared_counter = 1;
    std::size_t weak_counter = 0;      
    void (*deleter)(T*);

    ControlBlock(T* v, void (*d)(T*)) noexcept : value(v), deleter(d) {}
};

template <typename T>
class SharedPtrBase {
protected:
    ControlBlock<T>* block = nullptr;

    SharedPtrBase() noexcept = default;

    SharedPtrBase(const SharedPtrBase& other) noexcept : block(other.block) {
        if (block != nullptr) ++block->shared_counter;
    }

    SharedPtrBase(SharedPtrBase&& other) noexcept : block(other.block) {
        other.block = nullptr;
    }

    SharedPtrBase& operator=(const SharedPtrBase& other) noexcept {
        if (this != &other) {
            if (other.block != nullptr) ++other.block->shared_counter;  
            release();
            block = other.block;
        }
        return *this;
    }

    SharedPtrBase& operator=(SharedPtrBase&& other) noexcept {
        if (this != &other) {
            release();
            block = other.block;
            other.block = nullptr;
        }
        return *this;
    }

    ~SharedPtrBase() { release(); }

    void release() {
        if (block == nullptr) return;              

        if (--block->shared_counter == 0) {
            if (block->value != nullptr) {
                block->deleter(block->value);
                block->value = nullptr;
            }
            if (block->weak_counter == 0) {
                delete block;
            }
        }
        block = nullptr;
    }

public:
    T* get() const noexcept { return block != nullptr ? block->value : nullptr; }
    std::size_t useCount() const noexcept { return block != nullptr ? block->shared_counter : 0; }

    void reset(T* newPtr = nullptr, void (*newDeleter)(T*) = [](T* p) { delete p; }) {
        if (block != nullptr && block->value == newPtr) return;
        release();
        if (newPtr == nullptr) return;
        try {
            block = new ControlBlock<T>(newPtr, newDeleter);
        }
        catch (...) {
            newDeleter(newPtr);                    
            throw;
        }
    }
};

template <typename T>
class SharedPtr : public SharedPtrBase<T> {
    template <typename U>
    friend class WeakPtr;

    explicit SharedPtr(ControlBlock<T>* existingBlock) noexcept {
        if (existingBlock != nullptr && existingBlock->shared_counter != 0) {
            this->block = existingBlock;
            ++existingBlock->shared_counter;
        }
    }

public:
    SharedPtr() noexcept : SharedPtrBase<T>() {}

    explicit SharedPtr(T* value, void (*deleter)(T*) = [](T* p) { delete p; }) {
        if (value != nullptr) {
            try {
                this->block = new ControlBlock<T>(value, deleter);
            }
            catch (...) {
                deleter(value);
                throw;
            }
        }
    }

    T& operator*() const noexcept { return *this->block->value; }
    T* operator->() const noexcept { return this->block->value; }
};

template <typename T>
class SharedPtr<T[]> : public SharedPtrBase<T> {
    template <typename U>
    friend class WeakPtr;

    explicit SharedPtr(ControlBlock<T>* existingBlock) noexcept {
        if (existingBlock != nullptr && existingBlock->shared_counter != 0) {
            this->block = existingBlock;
            ++existingBlock->shared_counter;
        }
    }

public:
    SharedPtr() noexcept : SharedPtrBase<T>() {}

    explicit SharedPtr(T* value, void (*deleter)(T*) = [](T* p) { delete[] p; }) {
        if (value != nullptr) {
            try {
                this->block = new ControlBlock<T>(value, deleter);
            }
            catch (...) {
                deleter(value);
                throw;
            }
        }
    }


    void reset(T* newPtr = nullptr, void (*newDeleter)(T*) = [](T* p) { delete[] p; }) {
        if (this->block != nullptr && this->block->value == newPtr) return;
        this->release();
        if (newPtr == nullptr) return;
        try {
            this->block = new ControlBlock<T>(newPtr, newDeleter);
        }
        catch (...) {
            newDeleter(newPtr);
            throw;
        }
    }

    T& operator[](std::size_t index) const noexcept { return this->block->value[index]; }
};