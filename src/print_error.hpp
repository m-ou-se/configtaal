#include <mstd/string_view.hpp>
#include <string_tracker.hpp>

#include "error.hpp"

namespace conftaal {

void print_message(
	mstd::string_view message,
	string_pool::string_tracker const &,
	mstd::string_view where,
	bool color = true,
	bool error = false
);

void print_error(Error const &, string_pool::string_tracker const &, bool color = true);

}
