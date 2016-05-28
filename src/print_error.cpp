#include <string_view.hpp>
#include <string_tracker.hpp>

#include "print_error.hpp"

namespace conftaal {

void print_message(string_view message, string_tracker const & tracker, string_view where, bool color, bool error) {
	auto bad_src = tracker.get(where);
	auto bad_src_end = tracker.get(where.data() + where.size());
	if (color) std::clog << "\033[1m";
	std::clog << bad_src.location << ": ";
	if (error) {
		if (color) std::clog << "\033[31m";
		std::clog << "error: ";
		if (color) std::clog << "\033[;1m";
	}
	std::clog << message;
	if (color) std::clog << "\033[m";
	std::clog << std::endl;
	if (bad_src.location.column == 0 || bad_src.original_char - &bad_src.original_source[0] >= bad_src.location.column - 1) {
		char const * bad_line_start = bad_src.original_char - (bad_src.location.column - 1);
		std::string pointer_line;
		char const * original_source_end = bad_src.original_source.data() + bad_src.original_source.size();
		for (char const * c = bad_line_start; ; ++c) {
			if (c == bad_src.original_char || (c > bad_src.original_char && c < bad_src_end.original_char)) {
				pointer_line += '^';
			} else {
				pointer_line += ' ';
			}
			if (c == original_source_end || *c == '\n') break;
			if (*c == '\t') {
				std::clog << ' ';
				while (pointer_line.size() % 8 != 0) {
					pointer_line.push_back(pointer_line.back());
					std::clog << ' ';
				}
			} else {
				std::clog << *c;
			}
		}
		while (!pointer_line.empty() && pointer_line.back() == ' ') pointer_line.pop_back();
		if (bad_line_start == original_source_end) {
			if (color) std::clog << "\033[34m";
			std::clog << "<end of file>";
			if (color) std::clog << "\033[m";
		}
		std::clog << std::endl;
		if (color) std::clog << "\033[1;32m";
		std::clog << pointer_line;
		if (color) std::clog << "\033[m";
		std::clog << std::endl;
	}
}

void print_error(parse_error const & error, string_tracker const & tracker, bool color) {
	print_message(error.what(), tracker, error.where(), color, true);
	for (auto const & note : error.notes()) {
		print_message(note.first, tracker, note.second, color, false);
	}
}

}
