#ifndef _SYMBOLS_
#define _SYMBOLS_

#include "location.hh"
#include <string>

typedef yy::location location;

enum class node_type {empty, compose, while_stmt, for_stmt, iter_stmt, dowhile_stmt, if_stmt, _break_};
enum class opd_type {null, num, var, arr};
enum class bool_op {_or, _and, _not, _cast, _eq, _gt, _lt, _ge, _le};
enum class comp_type {_chksfty, _prestate, _cdecomp, _subsume, _access, _copy, _conjunct, _inc, _dec, _smp, _return};

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

	void print(int);
};

class ast_node{
public:
	ast_node *n1;
	ast_node *n2;
	yy::location loc;
	ast_node(yy::location &, ast_node *, ast_node *);
	virtual void compute();

	virtual void print(int);
};

class lre_node : public ast_node{
	ast_node *n3;
	opd opd1;
	opd opd2;
	opd opd3;
	node_type ty;
public:
	lre_node(yy::location &, node_type);
	lre_node(yy::location &, ast_node *, ast_node *, node_type);
	lre_node(yy::location &, ast_node *, ast_node *, ast_node *, node_type);
	lre_node(yy::location &, opd, opd, ast_node *,node_type);
	lre_node(yy::location &, opd, opd, opd, ast_node *,node_type);
	void compute();

	void print(int);
};

class bool_node : public ast_node{
public:
	opd opd1;
	opd opd2;
	bool result;
	bool_op op;
	bool_node(yy::location &, ast_node *, bool_op);
	bool_node(yy::location &, opd, bool_op);
	bool_node(yy::location &, ast_node *, ast_node *, bool_op);
	bool_node(yy::location &, opd , opd , bool_op);
	bool_node(yy::location &, opd, int, bool_op);
	void compute();

	void print(int);
};

class comp_node : public ast_node{
	opd opd1;
	opd opd2;
	opd opd3;
	opd opd4;
	int val;
	comp_type op;
public:
	comp_node(yy::location &, int, comp_type);
	comp_node(yy::location &, opd, comp_type);
	comp_node(yy::location &, opd, opd, comp_type);
	comp_node(yy::location &, opd, opd, opd, comp_type);
	comp_node(yy::location &, opd, opd, opd, opd, comp_type);
	comp_node(yy::location &, opd, int, comp_type);
	comp_node(yy::location &, opd, opd, int, comp_type);
	comp_node(yy::location &, opd, opd, opd, int, comp_type);
	void compute();

	void print(int);
};
#endif
