#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>

namespace SGP
{
	template<typename T> class Buffer
	{
		public:
			explicit Buffer(T* const buf = 0) : buf_(buf) {}

			explicit Buffer(size_t const n) : buf_(new T[n]{}) {}

			~Buffer() { if (buf_) delete[] buf_; }

			Buffer& Allocate(size_t const n) { return *this = new T[n]{}; }

			T* Release()
			{
				T* const buf = buf_;
				buf_ = 0;
				return buf;
			}

			Buffer& operator =(T* const buf)
			{
				if (buf_) delete[] buf_;
				buf_ = buf;
				return *this;
			}

			operator T*()             { return buf_; }
			operator T const*() const { return buf_; }

		private:
			T* buf_;

			Buffer(const Buffer&);          /* no copy */
			void operator =(const Buffer&); /* no assignment */
	};
}

#endif
