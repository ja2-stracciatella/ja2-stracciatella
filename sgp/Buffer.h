#ifndef BUFFER_H
#define BUFFER_H

#include "MemMan.h"


namespace SGP
{
	template<typename T> class Buffer
	{
		public:
			explicit Buffer()         : buf_(0)             {}
			explicit Buffer(size_t n) : buf_(MALLOCN(T, n)) {}

			~Buffer() { if (buf_) MemFree(buf_); }

			void Allocate(size_t const n)
			{
				if (buf_) MemFree(buf_);
				buf_ = MALLOCN(T, n);
			}

			T* Release()
			{
				T* const buf = buf_;
				buf_ = NULL;
				return buf;
			}

			operator T*()             { return buf_; }
			operator T const*() const { return buf_; }

		private:
			T* buf_;

			Buffer(const Buffer&); /* no copy */
	};
}

#endif
