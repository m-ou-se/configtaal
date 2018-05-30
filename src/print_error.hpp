#include <string_view>

#include <string_tracker.hpp>

#include "error.hpp"

namespace conftaal {

void print_message(
	std::string_view message,
	string_pool::string_tracker const &,
	std::string_view where,
	bool color = true,
	bool error = false
);

void print_error(Error const &, string_pool::string_tracker const &, bool color = true);

}
