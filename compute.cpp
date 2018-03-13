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
	else if (opd_.ty == opd_type::arr)
		return &((symtab[opd_.val].data)[opd_.idx]);
	else
		assert(false);
}

void ast_node::compute(){
	std::cerr<<"Not implemented"<<std::endl;
	assert(false);
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

		default:
			std::cerr<<"Should not reach here"<<std::endl;
			assert(false);
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
			if (fetch_type(opd1) == type::index){
				if (v1 != -1)
					result = ((*fetch_data(opd1)) == v1);
			}
			else
				result = ((*fetch_data(opd1)) == (*fetch_data(opd2)));
			break;

		case bool_op::_gt:
			result = ((*fetch_data(opd1)) == (*fetch_data(opd2)));
			break;

		case bool_op::_lt:
			result = ((*fetch_data(opd1)) == (*fetch_data(opd2)));
			break;

		case bool_op::_ge:
			result = ((*fetch_data(opd1)) == (*fetch_data(opd2)));
			break;

		case bool_op::_le:
			result = ((*fetch_data(opd1)) == (*fetch_data(opd2)));
			break;

		default:
			std::cerr<<"Should not reach here"<<std::endl;
			assert(false);
			break;
	}
}

void comp_node::compute(){
	switch(op){
		case comp_type::_copy:
			(*fetch_data(opd1)) = (*fetch_data(opd2));
			break;

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
				std::cerr<<"Should not reach here"<<std::endl;
				assert(false);
			break;

		case comp_type::_set:
			fetch_data(opd1)->set(val);
			break;

		case comp_type::_sat:{
			solver instSolver(man_t.AigNtk(), man_t.Network_Cnf());

			if (fetch_type(opd1) == type::region)
				instSolver.addRegion(*(region *)fetch_data(opd1));
			else
				instSolver.addAssumps(*(cube_ *)fetch_data(opd1));
			if (fetch_type(opd1) == type::region)
				instSolver.addRegion(*(region *)fetch_data(opd2));
			else
				instSolver.addAssumps(*(cube_ *)fetch_data(opd2));
			if (fetch_type(opd1) == type::region)
				instSolver.addRegion(*(region *)fetch_data(opd3));
			else
				instSolver.addAssumps(*(cube_ *)fetch_data(opd3));

			instSolver.solve(*(cube_ *)(fetch_data(opd4)));
			break;
		}

		case comp_type::_smp:
			if ((*(region *)fetch_data(opd1)) == (*(region *)fetch_data(opd2)))
				((cube_ *)fetch_data(opd3))->clear();
			else
				*((cube_ *)fetch_data(opd3)) = cube_(vector<lit>(1,-1));
			break;

		case comp_type::_return:
			if (val)
				global_ret_value = _result::UNSAT;
			else
				global_ret_value = _result::SAT;
			handle_exit();
			break;

		default:
			std::cerr<<"Should not reach here"<<std::endl;
			assert(false);
			break;
	}
}

