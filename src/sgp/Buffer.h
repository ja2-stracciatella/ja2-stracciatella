#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <memory>
#include <utility>

/*
 * Please do not use this class in new code, there is nothing it can do
 * that unique_ptr or vector cannot do better. The only reason the
 * deprecated attribute is commented out is for the sake of the existing
 * Buffer users.
 */
namespace SGP
{
	/* [[deprecated]] */ template<typename T> class Buffer
	{
		using UniqueT = std::unique_ptr<T []>;

		public:
			explicit Buffer(T* const buf = nullptr) : ptr_{ buf } {}

			explicit Buffer(std::size_t const n) { Allocate(n); }

			Buffer<T> & Allocate(std::size_t const n) { ptr_ = std::make_unique<T []>(n); return *this; }

			T* Release() noexcept { return ptr_.release(); }

			Buffer<T> & operator=(T * const buf) noexcept { ptr_.reset(buf); return *this; }

			// Small utility function to help ease the transition away from Buffer.
			UniqueT moveToUnique() noexcept { return std::move(ptr_); }

			operator T*()             noexcept { return ptr_.get(); }
			operator T const*() const noexcept { return ptr_.get(); }

		private:
			UniqueT ptr_;

			Buffer(const Buffer&) = delete;         /* no copy */
			void operator=(const Buffer&) = delete; /* no assignment */

			// Moving a buffer would actually work, but to discourage use
			// of this class, these two are deleted as well.
			Buffer(Buffer &&) = delete;         /* no move */
			void operator=(Buffer &&) = delete; /* no move assignment */
	};
}

#endif
