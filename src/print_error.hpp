#include <string_view.hpp>
#include <string_tracker.hpp>

#include "parse.hpp"

namespace conftaal {

void print_message(string_view message, string_tracker const &, string_view where, bool color = true, bool error = false);

void print_error(parse_error const &, string_tracker const &, bool color = true);

}
