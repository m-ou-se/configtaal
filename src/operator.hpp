#pragma once

#include <string_view.hpp>

namespace conftaal {

enum class operator_ {
	/* .  */ dot,
	/* [] */ index,
	/* () */ call,
	/* == */ equal,
	/* != */ inequal,
	/* >  */ greater,
	/* <  */ less,
	/* >= */ greater_or_equal,
	/* <= */ less_or_equal,
	/* +  */ unary_plus,
	/* -  */ unary_minus,
	/* ~  */ complement,
	/* !  */ not_,
	/* +  */ plus,
	/* -  */ minus,
	/* *  */ times,
	/* /  */ divide,
	/* %  */ modulo,
	/* ** */ power,
	/* << */ left_shift,
	/* >> */ right_shift,
	/* &  */ bit_and,
	/* |  */ bit_or,
	/* ^  */ bit_xor,
	/* && */ and_,
	/* || */ or_
};

enum class order {
	left,
	right,
	unordered
};

// A lower value means a higher precedence.
int get_precedence(operator_);

order get_associativity(int precedence);

order higher_precedence(operator_ left_op, operator_ right_op);

}
