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
				new (&v_[size_++]) T(e);
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
		T* new_v = 0;
		if (n != 0)
		{
			new_v = static_cast<T*>(malloc(sizeof(T) * n));
			if (!new_v) throw std::bad_alloc();
		}

		T*     const old_v    = v_;
		size_t const old_size = size_;
		size_t const new_size = MIN(old_size, n);
		for (size_t i = 0; i < new_size; ++i) new (&new_v[i]) T(old_v[i]);

		v_        = new_v;
		capacity_ = n;
		size_     = new_size;

		for (size_t i = old_size; i != 0;) old_v[--i].~T();
		free(old_v);
	}
}

#endif
