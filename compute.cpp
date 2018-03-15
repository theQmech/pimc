#include "lre/symbols.h"
#include "lre/symtab.h"
#include "utils.h"
#include "api.h"

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

// region *fetch_region(opd &opd_){
// 	region *reg = dynamic_cast<region *>(symtab[opd_.val].data);
// 	if (reg)
// 		return &reg[opd_.idx];
// 	else
// 		logAndStop("Should not reach here");
// }

// cube_ *fetch_cube(opd &opd_){
// 	cube_ *cub = dynamic_cast<cube_ *>(symtab[opd_.val].data);
// 	if (cub)
// 		return &cub[opd_.idx];
// 	else
// 		logAndStop("Should not reach here");
// }

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
			for(n1->compute(); ((bool_node *)n1)->result; n2->compute(), n1->compute());
			break;

		case node_type::if_stmt:
			n1->compute();
			if (((bool_node *)n1)->result)
				n2->compute();
			else if (n3 != NULL)
				n3->compute();
			break;

		case node_type::_break_:
			cout<<"Lalalaalalal"<<endl;
			// logAndStop("Breakpoint reached, run gdb to halt here");
			break;

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
			if (fetch_type(opd1) == type::index && opd2.ty == opd_type::num)
				result = (((index_ *)fetch_data(opd1))->getval() == opd2.val);
			else
				result = ((*fetch_data(opd1)) == (*fetch_data(opd2)));
			break;

		case bool_op::_gt:
			if (opd2.ty != opd_type::num)
				result = ((*fetch_data(opd1)) > (*fetch_data(opd2)));
			else
				result = dynamic_cast<index_ *>(fetch_data(opd1))->getval() > opd2.val;
			break;

		case bool_op::_lt:
			if (opd2.ty != opd_type::num)
				result = ((*fetch_data(opd1)) < (*fetch_data(opd2)));
			else
				result = dynamic_cast<index_ *>(fetch_data(opd1))->getval() < opd2.val;
			break;

		case bool_op::_ge:
			if (opd2.ty != opd_type::num)
				result = ((*fetch_data(opd1)) >= (*fetch_data(opd2)));
			else
				result = dynamic_cast<index_ *>(fetch_data(opd1))->getval() >= opd2.val;
			break;

		case bool_op::_le:
			if (opd2.ty != opd_type::num)
				result = ((*fetch_data(opd1)) <= (*fetch_data(opd2)));
			else
				result = dynamic_cast<index_ *>(fetch_data(opd1))->getval() <= opd2.val;
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

		case comp_type::_prime:
			if (fetch_type(opd1) == type::region)
				((region *)fetch_data(opd1))->toPrime(man_t.toPrime);
			else if (fetch_type(opd1) == type::cube)
				((cube_ *)fetch_data(opd1))->toPrime(man_t.toPrime);
			else
				logAndStop("Should not reach here");
			break;

		case comp_type::_sat:{
			solver instSolver(man_t.AigNtk(), man_t.Network_Cnf());

			instSolver.add(*fetch_data(opd1));
			instSolver.add(*fetch_data(opd2));
			instSolver.add(*fetch_data(opd3));
			// if (fetch_type(opd1) == type::region)
			// 	instSolver.addRegion(*(region *)fetch_data(opd1));
			// else
			// 	instSolver.addAssumps(*(cube_ *)fetch_data(opd1));

			// if (fetch_type(opd2) == type::region)
			// 	instSolver.addRegion(*(region *)fetch_data(opd2));
			// else
			// 	instSolver.addAssumps(*(cube_ *)fetch_data(opd2));

			// if (fetch_type(opd3) == type::region)
			// 	instSolver.addRegion(*(region *)fetch_data(opd3));
			// else
			// 	instSolver.addAssumps(*(cube_ *)fetch_data(opd3));

			instSolver.solve(*fetch_data(opd4));
			break;
		}

		case comp_type::_smp:
			logAndStop("Not implemented, use == operator");
			// if ((*(region *)fetch_data(opd1)) == (*(region *)fetch_data(opd2)))
			// 	((cube_ *)fetch_data(opd3))->clear();
			// else
			// 	*((cube_ *)fetch_data(opd3)) = vector<lit>(1,-1);
			break;

		case comp_type::_return:
			if (val == 0)
				global_ret_value = _result::UNSAT;
			else
				global_ret_value = _result::SAT;
			handle_exit();
			break;

		default:
			logAndStop("Should not reach here");
			break;
	}
}

