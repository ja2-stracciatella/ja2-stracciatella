#ifndef AUTOOBJ_H
#define AUTOOBJ_H

namespace SGP
{
	template<typename T, void (*dealloc)(T*)> class AutoObj
	{
		public:
			typedef AutoObj<T, dealloc> Type;

			AutoObj(T* const p = 0) : p_(p) {}

			~AutoObj() { if (p_) dealloc(p_); }

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

			T* operator ->() { return p_; }
			operator    T*() { return p_; }

		private:
			T* p_;

			AutoObj(const AutoObj&);   /* no copy */
			void operator =(AutoObj&); /* no assignment */
	};
}

#endif
