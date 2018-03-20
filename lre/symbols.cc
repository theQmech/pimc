#include "symbols.h"

opd::opd():val(-1),idx(-1){}
opd::opd(int val_, opd_type ty_):val(val_),idx(-1),ty(ty_){}
opd::opd(int val_, int idx_, opd_type ty_):val(val_),idx(idx_),ty(ty_){}

ast_node::ast_node(ast_node *n1_, ast_node *n2_):n1(n1_),n2(n2_){}

lre_node::lre_node(node_type ty_):ast_node(NULL, NULL),n3(NULL),ty(ty_){n1 = NULL;}
lre_node::lre_node(ast_node *n1_, ast_node *n2_, node_type ty_):ast_node(n1_, n2_),n3(NULL),ty(ty_){}
lre_node::lre_node(ast_node *n1_, ast_node *n2_, ast_node *n3_, node_type ty_):ast_node(n1_, n2_),n3(n3_),ty(ty_){}
lre_node::lre_node(opd op1_, opd op2_, opd op3_, ast_node *n1, node_type ty_):ast_node(n1, NULL),n3(NULL),opd1(op1_),opd2(op2_),opd3(op3_),ty(ty_){}

bool_node::bool_node(ast_node *n1_, bool_op op_):ast_node(n1_, NULL),op(op_),result(false){}
bool_node::bool_node(ast_node *n1_, ast_node *n2_, bool_op op_):ast_node(n1_, n2_),op(op_),result(false){}
bool_node::bool_node(opd opd1_, bool_op op_):ast_node(NULL, NULL),opd1(opd1_),op(op_),result(false){}
bool_node::bool_node(opd opd1_, opd opd2_, bool_op op_):ast_node(NULL, NULL),opd1(opd1_),opd2(opd2_),op(op_),result(false){}

comp_node::comp_node(int val_, comp_type op_):ast_node(NULL, NULL),val(val_),op(op_){}
comp_node::comp_node(opd opd1_, comp_type op_):ast_node(NULL, NULL),opd1(opd1_),op(op_){}
comp_node::comp_node(opd opd1_, opd opd2_, comp_type op_):ast_node(NULL, NULL),opd1(opd1_),opd2(opd2_),op(op_){}
comp_node::comp_node(opd opd1_, opd opd2_, opd opd3_, comp_type op_):ast_node(NULL, NULL),opd1(opd1_),opd2(opd2_),opd3(opd3_),op(op_){}
comp_node::comp_node(opd opd1_, opd opd2_, opd opd3_, opd opd4_, comp_type op_):ast_node(NULL, NULL),opd1(opd1_),opd2(opd2_),opd3(opd3_),opd4(opd4_),op(op_){}
comp_node::comp_node(opd opd1_, int val_, comp_type op_):ast_node(NULL, NULL),opd1(opd1_),val(val_),op(op_){}
