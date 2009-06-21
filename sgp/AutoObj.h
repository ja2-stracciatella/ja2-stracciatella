#ifndef AUTOOBJ_H
#define AUTOOBJ_H

namespace SGP
{
	template<typename T, void (*dealloc)(T*)> class AutoObj
	{
		public:
			explicit AutoObj(T* const p = 0) : p_(p) {}

			~AutoObj() { if (p_) dealloc(p_); }

			void Deallocate() { *this = 0; }

			T* Release()
			{
				T* const p = p_;
				p_ = 0;
				return p;
			}

			void operator =(T* const p)
			{
				if (p_) dealloc(p_);
				p_ = p;
			}

			T* operator ->() const { return p_; }

			operator T*() const { return p_; }

		private:
			T* p_;

			AutoObj(const AutoObj&);   /* no copy */
			void operator =(AutoObj&); /* no assignment */
	};
}

#endif
