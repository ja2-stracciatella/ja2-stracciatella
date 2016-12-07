#ifndef PODOBJ_H
#define PODOBJ_H

#include "MemMan.h"


namespace SGP
{
	template<typename T> class PODObj
	{
		public:
			PODObj() : p_(MALLOCZ(T)) {}

			PODObj(T* const p) : p_(p) {}

			~PODObj() { if (p_) MemFree(p_); }

			T* Allocate()
			{
				T* const p = MALLOCZ(T);
				if (p_) MemFree(p_);
				p_ = p;
				return p;
			}

			T* Release()
			{
				T* const p = p_;
				p_ = 0;
				return p;
			}

			T* operator ->() const { return p_; }

		private:
			T* p_;

			PODObj(const PODObj&);          /* no copy */
			void operator =(const PODObj&); /* no assignment */
	};
}

#endif
