#include <unistd.h>

#include <iostream>
#include <iomanip>

#include <string_view.hpp>
#include <string_tracker.hpp>

#include "expression.hpp"
#include "parse.hpp"
#include "print_error.hpp"

namespace conftaal {

std::ostream & operator << (std::ostream & out, Expression const & expr) {
	if (auto e = dynamic_cast<OperatorExpression const *>(&expr)) {
		out << "(op" << e->op_source << ' ';
		if (!e->is_unary()) {
			out << *e->lhs << ' ';
		}
		out << *e->rhs << ')';
	} else if (auto e = dynamic_cast<IdentifierExpression const *>(&expr)) {
		out << "id:" << e->identifier;
	} else if (auto e = dynamic_cast<ObjectExpression const *>(&expr)) {
		out << "(object keys=" << *e->keys << " values=" << *e->values << ')';
	} else if (auto e = dynamic_cast<ListExpression const *>(&expr)) {
		out << "(list";
		for (auto const & v : e->elements) {
			out << ' ' << *v;
		}
		out << ')';
	} else if (auto e = dynamic_cast<StringLiteralExpression const *>(&expr)) {
		out << "str:" << std::quoted(std::string(e->value));
	} else if (auto e = dynamic_cast<IntegerLiteralExpression const *>(&expr)) {
		out << "int:" << e->value;
	} else if (auto e = dynamic_cast<DoubleLiteralExpression const *>(&expr)) {
		out << "float:" << std::hexfloat << e->value;
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
		conftaal::Parser parser(tracker, *src);
		auto expr = parser.parse_expression();
		if (!expr) throw conftaal::ParseError("missing expression", *src);
		std::cout << *expr << std::endl;
	} catch (conftaal::ParseError & e) {
		conftaal::print_error(e, tracker, isatty(2));
	}
}
