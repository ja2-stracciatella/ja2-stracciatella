#ifndef VECTOR_H
#define VECTOR_H

#include <new>


namespace SGP
{
	template<typename T> class Vector
	{
		public:
			Vector() : v_(), capacity_(), size_() {}

			~Vector() { Reserve(0); }

			size_t Capacity() const { return capacity_; }
			size_t Size()     const { return size_;     }

			T&       operator [](size_t const i)       { return v_[i]; }
			T const& operator [](size_t const i) const { return v_[i]; }

			void PushBack(const T& e)
			{
				if (size_ >= capacity_) Reserve(MAX(1, size_ * 2));
				new (&v_[size_]) T(e);
				++size_;
			}

			void Reserve(size_t const n);

		private:
			T*     v_;
			size_t capacity_;
			size_t size_;

			Vector(const Vector&);          /* no copy */
			void operator =(const Vector&); /* no assignment */
	};

	template<typename T> void Vector<T>::Reserve(size_t const n)
	{
		T* const new_v = n == 0 ? 0 : MALLOCN(T, n);

		size_t       i;
		T*     const old_v    = v_;
		size_t const old_size = size_;
		size_t const new_size = MIN(old_size, n);
		try
		{
			for (i = 0; i < new_size; ++i) new (&new_v[i]) T(old_v[i]);
		}
		catch (...)
		{
			for (size_t k = i; k != 0;) new_v[--i].~T();
			free(new_v);
			throw;
		}

		v_        = new_v;
		capacity_ = n;
		size_     = new_size;

		for (size_t k = old_size; k != 0;) old_v[--k].~T();
		free(old_v);
	}
}

#endif
