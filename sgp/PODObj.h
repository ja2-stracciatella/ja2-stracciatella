#ifndef PODOBJ_H
#define PODOBJ_H

#include <stdexcept>

#include "MemMan.h"


namespace SGP
{
	template<typename T> class PODObj
	{
		public:
			PODObj() : p_(MALLOC(T))
			{
				if (!p_) throw std::bad_alloc();
				memset(p_, 0, sizeof(*p_));
			}

			~PODObj() { if (p_) MemFree(p_); }

			T* Release()
			{
				T* const p = p_;
				p_ = 0;
				return p;
			}

			operator T*() const { return p_; }

			T* operator ->() const { return p_; }

		private:
			T* p_;

			PODObj(const PODObj&);          /* no copy */
			void operator =(const PODObj&); /* no assignment */
	};
}

#endif
