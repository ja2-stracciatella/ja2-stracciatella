#ifndef AUTOPTR_H
#define AUTOPTR_H

namespace SGP
{
	template<typename T> class AutoPtr
	{
		public:
			typedef AutoPtr<T> Type;

			explicit AutoPtr(T* const p = 0) : p_(p) {}

			~AutoPtr() { if (p_) delete p_; }

			void Deallocate() { *this = 0; }

			T* Release()
			{
				T* const p = p_;
				p_ = 0;
				return p;
			}

			void operator =(T* const p)
			{
				if (p_) delete p_;
				p_ = p;
			}

			T* const operator ->() { return p_; }

			operator T*() { return p_; }

		private:
			T* p_;

			AutoPtr(const AutoPtr&);   /* no copy */
			void operator =(AutoPtr&); /* no assignment */
	};
}

#endif
