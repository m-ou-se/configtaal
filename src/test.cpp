#include <unistd.h>

#include <iostream>
#include <iomanip>

#include <string_view.hpp>
#include <string_tracker.hpp>

#include "expression.hpp"
#include "parse.hpp"

namespace conftaal {

std::ostream & operator << (std::ostream & out, expression const & expr) {
	if (auto e = dynamic_cast<operator_expression const *>(&expr)) {
		out << '(' << e->op << ' ';
		if (!e->is_unary()) {
			out << *e->lhs << ' ';
		}
		out << *e->rhs << ')';
	} else if (auto e = dynamic_cast<identifier_expression const *>(&expr)) {
		out << "(ref " << e->identifier << ')';
	} else if (auto e = dynamic_cast<object_literal_expression const *>(&expr)) {
		out << "(object";
		for (auto const & v : e->elements) {
			out << ' ' << *v.first << '=' << *v.second;
		}
		out << ')';
	} else if (auto e = dynamic_cast<list_literal_expression const *>(&expr)) {
		out << "(list";
		for (auto const & v : e->elements) {
			out << ' ' << *v;
		}
		out << ')';
	} else if (auto e = dynamic_cast<string_literal_expression const *>(&expr)) {
		out << std::quoted(std::string(e->value));
	} else {
		out << "???";
	}
	return out;
}

}

void print_message(string_view message, string_tracker const & tracker, string_view where = {}, bool error = false, bool color = true) {
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

int main(int argc, char * * argv) {
	if (argc != 2) {
		std::clog << "Usage: " << argv[0] << " file" << std::endl;
		return 1;
	}

	string_tracker tracker;

	auto src = tracker.add_file(argv[1]);

	if (!src) {
		std::cerr << "Unable to open file." << std::endl;
		return 1;
	}

	try {
		conftaal::parser parser(tracker, *src);
		auto expr = parser.parse_expression();
		if (!expr) throw conftaal::parse_error("missing expression", *src);
		std::cout << *expr << std::endl;
	} catch (conftaal::parse_error & e) {
		bool color = isatty(2);
		print_message(e.what(), tracker, e.where(), true, color);
		for (auto const & note : e.notes()) {
			print_message(note.first, tracker, note.second, false, color);
		}
	}
}

