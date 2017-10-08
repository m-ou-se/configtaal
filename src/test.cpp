#include <unistd.h>

#include <iostream>
#include <iomanip>

#include <mstd/string_view.hpp>
#include <string_tracker.hpp>

#include "expression.hpp"
#include "parse.hpp"
#include "print_error.hpp"

namespace conftaal {

char const * op_str(Operator op) {
	switch (op) {
		case Operator::dot:              return "dot";
		case Operator::index:            return "index";
		case Operator::call:             return "call";
		case Operator::colon:            return "colon";
		case Operator::equal:            return "equal";
		case Operator::inequal:          return "inequal";
		case Operator::greater:          return "greater";
		case Operator::less:             return "less";
		case Operator::greater_or_equal: return "greater_or_equal";
		case Operator::less_or_equal:    return "less_or_equal";
		case Operator::unary_plus:       return "unary_plus";
		case Operator::unary_minus:      return "unary_minus";
		case Operator::complement:       return "complement";
		case Operator::logical_not:      return "logical_not";
		case Operator::plus:             return "plus";
		case Operator::minus:            return "minus";
		case Operator::times:            return "times";
		case Operator::divide:           return "divide";
		case Operator::modulo:           return "modulo";
		case Operator::power:            return "power";
		case Operator::left_shift:       return "left_shift";
		case Operator::right_shift:      return "right_shift";
		case Operator::bit_and:          return "bit_and";
		case Operator::bit_or:           return "bit_or";
		case Operator::bit_xor:          return "bit_xor";
		case Operator::logical_and:      return "logical_and";
		case Operator::logical_or:       return "logical_or";
	}
	return "???";
}

std::ostream & operator << (std::ostream & out, Expression const & expr) {
	if (auto e = dynamic_cast<OperatorExpression const *>(&expr)) {
		out << "(op:" << op_str(e->op) << ' ';
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
