#include "RustInterface.h"

void throwRustError(bool condition) {
	if (condition) {
		RustPointer<char> err{ getRustError() };
		throw std::runtime_error(err.get());
	}
}
