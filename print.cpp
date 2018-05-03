#include "utils.h"
#include "lre/symbols.h"

std::ostream& operator<<(std::ostream &os, node_type &ty){
	cout<<"node_type::";
	switch(ty){
		case node_type::empty:
			cout<<"empty";
			break;

		case node_type::compose:
			cout<<"compose";
			break;

		case node_type::while_stmt:
			cout<<"while_stmt";
			break;

		case node_type::for_stmt:
			cout<<"for_stmt";
			break;

		case node_type::iter_stmt:
			cout<<"iter_stmt";
			break;

		case node_type::dowhile_stmt:
			cout<<"dowhile_stmt";
			break;

		case node_type::if_stmt:
			cout<<"if_stmt";
			break;

		case node_type::_break_:
			cout<<"_break_";
			break;

		default:
			logAndStop("Should not reach here");
			break;
	}
}

std::ostream& operator<<(std::ostream &os, opd_type &ty){
	cout<<"opd_type::";
	switch(ty){
		case opd_type::null:
			cout<<"null";
			break;

		case opd_type::num:
			cout<<"num";
			break;

		case opd_type::var:
			cout<<"var";
			break;

		case opd_type::arr:
			cout<<"arr";
			break;

		default:
			logAndStop("Should not reach here");
			break;
	}
}

std::ostream& operator<<(std::ostream &os, bool_op &ty){
	cout<<"bool_op::";
	switch(ty){
		case bool_op::_or:
			cout<<"_or";
			break;

		case bool_op::_and:
			cout<<"_and";
			break;

		case bool_op::_not:
			cout<<"_not";
			break;

		case bool_op::_cast:
			cout<<"_cast";
			break;

		case bool_op::_eq:
			cout<<"_eq";
			break;

		case bool_op::_gt:
			cout<<"_gt";
			break;

		case bool_op::_lt:
			cout<<"_lt";
			break;

		case bool_op::_ge:
			cout<<"_ge";
			break;

		case bool_op::_le:
			cout<<"_le";
			break;

		default:
			logAndStop("Should not reach here");
			break;
	}
}

std::ostream& operator<<(std::ostream &os, comp_type &ty){
	cout<<"comp_type::";
	switch(ty){
		case comp_type::_chksfty:
			cout<<"_chksfty";
			break;

		case comp_type::_prestate:
			cout<<"_prestate";
			break;

		case comp_type::_cdecomp:
			cout<<"_cdecomp";
			break;

		case comp_type::_subsume:
			cout<<"_subsume";
			break;

		case comp_type::_gen:
			cout<<"_gen";
			break;

		case comp_type::_gen2:
			cout<<"_gen2";
			break;

		case comp_type::_copy:
			cout<<"_copy";
			break;

		case comp_type::_conjunct:
			cout<<"_conjunct";
			break;

		case comp_type::_inc:
			cout<<"_inc";
			break;

		case comp_type::_dec:
			cout<<"_dec";
			break;

		case comp_type::_smp:
			cout<<"_smp";
			break;

		case comp_type::_return:
			cout<<"_return";
			break;

		default:
			logAndStop("Should not reach here");
			break;
	}
}

void opd::print(int num_spaces){
	cout<<string(num_spaces, ' ')<<"[opd]\t"<<ty<<"\t"<<val<<"\t"<<idx<<endl;
}

void ast_node::print(int num_spaces){
	cout<<"[ast_node] should not reach here"<<endl;
	assert(false);
}

void lre_node::print(int num_spaces){
	cout<<string(num_spaces++, ' ')
		<<"[lre_node]\t"<<ty<<"\t"<<loc<<"\t"<<this<<endl;
	if (n1)
		n1->print(num_spaces);
	if (n2)
		n2->print(num_spaces);
	if (n3)
		n3->print(num_spaces);
	opd1.print(num_spaces);
	opd2.print(num_spaces);
	opd3.print(num_spaces);
}

void bool_node::print(int num_spaces){
	cout<<string(num_spaces++, ' ')
		<<"[bool_node]\t"<<op<<"\t"<<loc<<"\t"<<this<<endl;
	if (n1)
		n1->print(num_spaces);
	if (n2)
		n2->print(num_spaces);
	opd1.print(num_spaces);
	opd2.print(num_spaces);
}

void comp_node::print(int num_spaces){
	cout<<string(num_spaces++, ' ')
		<<"[comp_node]\t"<<op<<"\t"<<loc<<"\t"<<this<<endl;
	if (n1)
		n1->print(num_spaces);
	if (n2)
		n2->print(num_spaces);
	opd1.print(num_spaces);
	opd2.print(num_spaces);
	opd3.print(num_spaces);
	opd4.print(num_spaces);
}
