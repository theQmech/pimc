#include "symbols.h"

opd::opd():ty(opd_type::null),val(-1),idx(-1){}
opd::opd(int val_, opd_type ty_):val(val_),idx(-1),ty(ty_){}
opd::opd(int val_, int idx_, opd_type ty_):val(val_),idx(idx_),ty(ty_){}

ast_node::ast_node(yy::location &loc, ast_node *n1_, ast_node *n2_):loc(loc),n1(n1_),n2(n2_){}

lre_node::lre_node(yy::location &loc, node_type ty_):ast_node(loc, NULL, NULL),n3(NULL),opd1(),opd2(),opd3(),ty(ty_){n1 = NULL;}
lre_node::lre_node(yy::location &loc, ast_node *n1_, ast_node *n2_, node_type ty_):ast_node(loc, n1_, n2_),n3(NULL),opd1(),opd2(),opd3(),ty(ty_){}
lre_node::lre_node(yy::location &loc, ast_node *n1_, ast_node *n2_, ast_node *n3_, node_type ty_):ast_node(loc, n1_, n2_),n3(n3_),opd1(),opd2(),opd3(),ty(ty_){}
lre_node::lre_node(yy::location &loc, opd op1_, opd op2_, ast_node *n1, node_type ty_):ast_node(loc, n1, NULL),n3(NULL),opd1(op1_),opd2(op2_),opd3(),ty(ty_){}
lre_node::lre_node(yy::location &loc, opd op1_, opd op2_, opd op3_, ast_node *n1, node_type ty_):ast_node(loc, n1, NULL),n3(NULL),opd1(op1_),opd2(op2_),opd3(op3_),ty(ty_){}

bool_node::bool_node(yy::location &loc, ast_node *n1_, bool_op op_):ast_node(loc, n1_, NULL),opd1(),opd2(),op(op_),result(false){}
bool_node::bool_node(yy::location &loc, ast_node *n1_, ast_node *n2_, bool_op op_):ast_node(loc, n1_, n2_),opd1(),opd2(),op(op_),result(false){}
bool_node::bool_node(yy::location &loc, opd opd1_, bool_op op_):ast_node(loc, NULL, NULL),opd1(opd1_),opd2(),op(op_),result(false){}
bool_node::bool_node(yy::location &loc, opd opd1_, opd opd2_, bool_op op_):ast_node(loc, NULL, NULL),opd1(opd1_),opd2(opd2_),op(op_),result(false){}

comp_node::comp_node(yy::location &loc, int val_, comp_type op_):ast_node(loc, NULL, NULL),opd1(),opd2(),opd3(),opd4(),val(val_),op(op_){}
comp_node::comp_node(yy::location &loc, opd opd1_, comp_type op_):ast_node(loc, NULL, NULL),opd1(opd1_),opd2(),opd3(),opd4(),op(op_){}
comp_node::comp_node(yy::location &loc, opd opd1_, opd opd2_, comp_type op_):ast_node(loc, NULL, NULL),opd1(opd1_),opd2(opd2_),opd3(),opd4(),op(op_){}
comp_node::comp_node(yy::location &loc, opd opd1_, opd opd2_, opd opd3_, comp_type op_):ast_node(loc, NULL, NULL),opd1(opd1_),opd2(opd2_),opd3(opd3_),opd4(),op(op_){
	bool k = true;
}
comp_node::comp_node(yy::location &loc, opd opd1_, opd opd2_, opd opd3_, opd opd4_, comp_type op_):ast_node(loc, NULL, NULL),opd1(opd1_),opd2(opd2_),opd3(opd3_),opd4(opd4_),op(op_){}
comp_node::comp_node(yy::location &loc, opd opd1_, int val_, comp_type op_):ast_node(loc, NULL, NULL),opd1(opd1_),opd2(),opd3(),opd4(),val(val_),op(op_){}
comp_node::comp_node(yy::location &loc, opd opd1_, opd opd2_, int val_, comp_type op_):ast_node(loc, NULL, NULL),opd1(opd1_),opd2(opd2_),opd3(),opd4(),val(val_),op(op_){};
comp_node::comp_node(yy::location &loc, opd opd1_, opd opd2_, opd opd3_, int val_, comp_type op_):ast_node(loc, NULL, NULL),opd1(opd1_),opd2(opd2_),opd3(opd3_),opd4(),val(val_),op(op_){};
