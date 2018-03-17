#ifndef _SYMBOLS_
#define _SYMBOLS_

#include <string>

enum class node_type {empty, compose, while_stmt, if_stmt, _break_};
enum class opd_type {num, var, arr};
enum class bool_op {_or, _and, _not, _cast, _eq, _gt, _lt, _ge, _le};
enum class comp_type {_size, _access, _copy, _conjunct, _inc, _dec, _prime, _prop, _sat, _smp, _return};

class opd{
public:
	opd_type ty;
	// index into symbol table entry or numeric value
	int val;
	// if array, then index of indexing variable contained in symbol table
	int idx;
	opd();
	opd(int, opd_type);
	opd(int, int, opd_type);
};

class ast_node{
public:
	ast_node *n1;
	ast_node *n2;
	ast_node(ast_node *, ast_node *);
	virtual void compute();
};

class lre_node : public ast_node{
	ast_node *n3;
	node_type ty;
public:
	lre_node(node_type);
	lre_node(ast_node *, ast_node *, node_type);
	lre_node(ast_node *, ast_node *, ast_node *, node_type);
	void compute();
};

class bool_node : public ast_node{
public:
	opd opd1;
	opd opd2;
	bool result;
	bool_op op;
	bool_node(ast_node *, bool_op);
	bool_node(opd, bool_op);
	bool_node(ast_node *, ast_node *, bool_op);
	bool_node(opd , opd , bool_op);
	bool_node(opd, int, bool_op);
	void compute();
};

class comp_node : public ast_node{
	opd opd1;
	opd opd2;
	opd opd3;
	opd opd4;
	int val;
	comp_type op;
public:
	comp_node(int, comp_type);
	comp_node(opd, comp_type);
	comp_node(opd, opd, comp_type);
	comp_node(opd, opd, opd, comp_type);
	comp_node(opd, opd, opd, opd, comp_type);
	comp_node(opd, int, comp_type);
	void compute();
};
#endif
