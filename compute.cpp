#include "lre/symbols.h"
#include "lre/symtab.h"
#include "utils.h"
#include "api.h"

extern bool VERBOSE;
extern bool DEBUG;

type fetch_type(opd &opd_){
	if (opd_.ty == opd_type::num)
		assert(false);
	else
		return symtab[opd_.val].entry_type;
}

data_struct *fetch_data(opd &opd_){
	if (opd_.ty == opd_type::var)
		return symtab[opd_.val].data;
	else if (opd_.ty == opd_type::arr){
		assert(0<=opd_.idx && opd_.idx<ARR_SZ);

		region *reg = dynamic_cast<region *>(symtab[opd_.val].data);
		cube_ *cub = dynamic_cast<cube_ *>(symtab[opd_.val].data);

		index_ *idx = dynamic_cast<index_ *>(symtab[opd_.idx].data);
		assert(idx);

		if (reg)
			return &reg[idx->getval()];
		else if (cub)
			return &cub[idx->getval()];
		else
			logAndStop("Should not reach here");
	}
	else
		logAndStop("Should not reach here");
}

int fetch_idx(opd &opd_){
	return ((opd_.ty==opd_type::num)?
				opd_.val:((index_ *)fetch_data(opd_))->getval());
}

void ast_node::compute(){
	logAndStop("Not implemented");
}

void lre_node::compute(){
	switch(ty){
		case node_type::empty:
			break;

		case node_type::compose:
			n1->compute();
			n2->compute();
			break;

		case node_type::while_stmt:
			for(
				n1->compute();
				((bool_node *)n1)->result;
				n2->compute(), n1->compute()
			);
			break;

		case node_type::dowhile_stmt:
			for(
				n2->compute(), n1->compute();
				((bool_node *)n1)->result;
				n2->compute(), n1->compute()
			);
			break;

		case node_type::for_stmt:{
			fetch_data(opd1)->set(opd2.val);
			int rval = fetch_idx(opd3);
			assert(opd2.val <= rval);

			while(((index_ *)fetch_data(opd1))->getval() < rval){
				n1->compute();
				rval = fetch_idx(opd3);
			}
			break;
		}

		case node_type::iter_stmt:{
			coll_ *setInst = dynamic_cast<coll_ *>(fetch_data(opd2));
			for(int i=0; i<setInst->size(); ++i){
				*dynamic_cast<cube_ *>(fetch_data(opd1)) = *((*setInst)[i]);
				n1->compute();
			}
			break;
		}

		case node_type::if_stmt:
			n1->compute();
			if (((bool_node *)n1)->result)
				n2->compute();
			else if (n3 != NULL)
				n3->compute();
			break;

		case node_type::_break_:{
			if (VERBOSE)
				cout<<"At break point "<<loc<<endl;
			if (DEBUG){
				// ((region *)symtab[5].data)[0].print();
				// ((region *)symtab[5].data)[1].print();
				// ((region *)symtab[5].data)[2].print();
				// logAndStop("Breakpoint reached, run gdb to halt here");
			}
			break;
		}

		default:
			logAndStop("Should not reach here");
			break;
	}
}

void bool_node::compute(){
	bool_node *n1 = (bool_node *)n1;
	bool_node *n2 = (bool_node *)n2;

	switch(op){
		case bool_op::_or:
			n1->compute();
			if (n1->result){
				result = true;
				break;
			}
			n2->compute();
			result = n2->result;
			break;

		case bool_op::_and:
			n1->compute();
			if (!n1->result){
				result = false;
				break;
			}
			n2->compute();
			result = n2->result;
			break;

		case bool_op::_not:
			n1->compute();
			result = !n1->result;
			break;

		case bool_op::_cast:
			result = (bool)(*fetch_data(opd1));
			break;

		case bool_op::_eq:
			if (opd2.ty != opd_type::num)
				result = ((*fetch_data(opd1)) == (*fetch_data(opd2)));
			else
				result = (fetch_idx(opd1) == opd2.val);
			break;

		case bool_op::_gt:
			if (opd2.ty != opd_type::num)
				result = ((*fetch_data(opd1)) > (*fetch_data(opd2)));
			else
				result = fetch_idx(opd1) > opd2.val;
			break;

		case bool_op::_lt:
			if (opd2.ty != opd_type::num)
				result = ((*fetch_data(opd1)) < (*fetch_data(opd2)));
			else
				result = fetch_idx(opd1) < opd2.val;
			break;

		case bool_op::_ge:
			if (opd2.ty != opd_type::num)
				result = ((*fetch_data(opd1)) >= (*fetch_data(opd2)));
			else
				result = fetch_idx(opd1) >= opd2.val;
			break;

		case bool_op::_le:
			if (opd2.ty != opd_type::num)
				result = ((*fetch_data(opd1)) <= (*fetch_data(opd2)));
			else
				result = fetch_idx(opd1) <= opd2.val;
			break;

		default:
			logAndStop("Should not reach here");
			break;
	}
}

void comp_node::compute(){
	switch(op){
		case comp_type::_copy:
			if (opd2.ty == opd_type::num)
				fetch_data(opd1)->set(opd2.val);
			else
				(*fetch_data(opd1)) = (*fetch_data(opd2));
			break;

		case comp_type::_access:{
			vector<lit> cl = (*(dynamic_cast<region *>(fetch_data(opd2))))[((index_ *)fetch_data(opd3))->getval()];
			*(dynamic_cast<cube_ *>(fetch_data(opd1))) = cl;
			(dynamic_cast<cube_ *>(fetch_data(opd1)))->complement();
			break;
		}

		case comp_type::_conjunct:{
			fetch_data(opd1)->conjunct(*fetch_data(opd2));
			break;
		}

		case comp_type::_inc:
			++(*fetch_data(opd1));
			break;

		case comp_type::_dec:
			--(*fetch_data(opd1));
			break;

		case comp_type::_chksfty:{
			solver instSolver(man_t.AigNtk(), man_t.Network_Cnf());

			// Add state
			instSolver.add(*fetch_data(opd2));

			// Add notP
			cube_ notP;
			InitProperty(notP);
			notP.complement();
			instSolver.add(notP);

			// Add transition 0
			region T;
			initTransition(T);
			instSolver.add(T);

			instSolver.solve(*fetch_data(opd1));
			break;
		}

		case comp_type::_prestate:{
			solver instSolver(man_t.AigNtk(), man_t.Network_Cnf());

			// Add state
			instSolver.add(*fetch_data(opd2));

			// Add cexprime
			cube_ cex = *dynamic_cast<cube_ *>(fetch_data(opd3));
			toPrime(cex);
			instSolver.add(cex);

			// Add transition 0
			region T;
			initTransition(T);
			instSolver.add(T);

			instSolver.solve(*fetch_data(opd1));
			break;
		}

		case comp_type::_cdecomp:{
			*dynamic_cast<coll_ *>(fetch_data(opd1)) =
								*dynamic_cast<region *>(fetch_data(opd2));
			break;
		}

		case comp_type::_subsume:{
			cube_ *lhs = dynamic_cast<cube_ *>(fetch_data(opd1));
			region *rhs1 = dynamic_cast<region *>(fetch_data(opd2));
			region *rhs2 = dynamic_cast<region *>(fetch_data(opd3));

			assert(lhs);
			assert(rhs1);
			assert(rhs2);

			int res = rhs1->implies(*rhs2);
			if (res == -1)
				lhs->clear();
			else
				*lhs = (*rhs1)[res];

			break;
		}

		case comp_type::_gen:{
			cube_ *rval = dynamic_cast<cube_ *>(fetch_data(opd1));
			cube_ *cex = dynamic_cast<cube_ *>(fetch_data(opd2));
			region *reg = dynamic_cast<region *>(fetch_data(opd3));

			assert(rval);
			assert(cex);
			assert(reg);

			*rval = implicate(*cex, *reg);

			break;
		}

		case comp_type::_return:
			if (val == 0)
				global_ret_value = _result::SAT;
			else
				global_ret_value = _result::UNSAT;
			handle_exit();
			break;

		default:
			logAndStop("Should not reach here");
			break;
	}
}

// print functions

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

		case comp_type::_access:
			cout<<"_access";
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
	cout<<string(num_spaces++, ' ')<<"[lre_node]\t"<<ty<<"\t"<<loc<<"\t"<<this<<endl;
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
	cout<<string(num_spaces++, ' ')<<"[bool_node]\t"<<op<<"\t"<<loc<<"\t"<<this<<endl;
	if (n1)
		n1->print(num_spaces);
	if (n2)
		n2->print(num_spaces);
	opd1.print(num_spaces);
	opd2.print(num_spaces);
}

void comp_node::print(int num_spaces){
	cout<<string(num_spaces++, ' ')<<"[comp_node]\t"<<op<<"\t"<<loc<<"\t"<<this<<endl;
	if (n1)
		n1->print(num_spaces);
	if (n2)
		n2->print(num_spaces);
	opd1.print(num_spaces);
	opd2.print(num_spaces);
	opd3.print(num_spaces);
	opd4.print(num_spaces);
}
