#include "value.hpp"

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

namespace conftaal {

namespace {
	std::string demangle(const char* name) {
#ifndef __GNUG__
		return name;
#else
		int status;

		std::unique_ptr<char, void(*)(void*)> result {
			abi::__cxa_demangle(name, NULL, NULL, &status),
			std::free
		};

		return status == 0 ? result.get() : name;
#endif
	}
}

std::string TypeInfo::name() const {
	if (*this == TypeInfo::of<std::string>()) return "std::string";
	return demangle(std::type_index::name());
}

}
