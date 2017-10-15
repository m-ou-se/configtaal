#pragma once
#include <string>

namespace conftaal {

enum class Operator {
	/* .  */ dot,
	/* [] */ index,
	/* () */ call,
	/* :  */ colon,
	/* == */ equal,
	/* != */ inequal,
	/* >  */ greater,
	/* <  */ less,
	/* >= */ greater_or_equal,
	/* <= */ less_or_equal,
	/* +  */ unary_plus,
	/* -  */ unary_minus,
	/* ~  */ complement,
	/* !  */ logical_not,
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
	/* && */ logical_and,
	/* || */ logical_or
};

std::string to_string(Operator);

enum class order {
	left,
	right,
	unordered
};

// A lower value means a higher precedence.
int get_precedence(Operator);

order get_associativity(int precedence);

order higher_precedence(Operator left_op, Operator right_op);

}
