#pragma once
#include "SharedPtr.h"

template <typename T>
class WeakPtr {

	ControlBlock<T>* block = nullptr;

	void release() {
		if (block == nullptr) { return; }
		
		if (--this->block->weak_counter == 0 && this->block->shared_counter == 0) {
			delete block;
		}
		this->block == nullptr;
	}



public:
	explicit WeakPtr() = default;

	WeakPtr(const SharedPtr<T>& other) noexcept {
		this->block = other.block;
		if (this->block) {
			this->block->weak_counter++;
		}
	}

	WeakPtr(const WeakPtr& other) noexcept : block(other.block) {
		if (block != nullptr) ++block->weak_counter;
	}

	WeakPtr(WeakPtr&& other) noexcept : block(other.block) {
		other.block = nullptr;
	}

	WeakPtr& operator=(const WeakPtr& other) noexcept {
		if (this != &other) {
			if (other.block != nullptr) ++other.block->weak_counter;
			release();
			block = other.block;
		}
		return *this;
	}

	WeakPtr& operator=(WeakPtr&& other) noexcept {
		if (this != &other) {
			release();
			block = other.block;
			other.block = nullptr;
		}
		return *this;
	}

	~WeakPtr() { release(); }

	

	bool expired() const noexcept {
		if (block == nullptr) {
			return true;
		}
		return block->shared_counter == 0;
	}

	size_t useCount() const noexcept {
		if (this->block) {
			return this->block->shared_counter;
		}
		return 0;
	}

	void reset() noexcept {
		release();
	}

	SharedPtr<T> lock() const noexcept{
		if (block == nullptr || block->shared_counter == 0) {
			return SharedPtr<T>();
		}
		return SharedPtr<T>(this->block);
	}


};