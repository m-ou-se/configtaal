#include <unistd.h>

#include <iostream>
#include <iomanip>

#include <string_view.hpp>
#include <string_tracker.hpp>

#include "expression.hpp"
#include "parse.hpp"
#include "print_error.hpp"

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
	} else if (auto e = dynamic_cast<integer_literal_expression const *>(&expr)) {
		out << e->value;
	} else {
		out << "???";
	}
	return out;
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
		conftaal::print_error(e, tracker, isatty(2));
	}
}

