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

// DO NOT COPY THIS CODE OR REFER TO IT TO FOR PURPOSES OF WRITING MORE CODE.
void prop_clauses(int begin, int end){
	region *frame = dynamic_cast<region *>(symtab[symtab.get_symbol("CNF_ARR")].data);
	assert(0 <= begin && begin < 10);
	assert(0 <= end && end < 10);

	cube_ cex;

	for(int curr=begin; curr<end; ++curr){
		solver instSolver(man_t.AigNtk(), man_t.Network_Cnf());
		instSolver.add(*((data_struct *)(&frame[curr])));
		instSolver.add(*(symtab[symtab.get_symbol("T")].data));

		for(int j=0; j<frame[curr].size(); ++j){
			instSolver.assumps.clear();
			instSolver.assumps = frame[curr][j];
			for(int k=0; k<instSolver.assumps.size(); ++k)
				instSolver.assumps[k] = lit_neg(instSolver.assumps[k]);

			instSolver.solve(*((data_struct *)(&cex)));

			if (cex){
			}
			else{
				frame[begin+1].addClause(instSolver.assumps, true);
			}
		}

	}

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
			// cout<<"currframe:\t"<<((index_ *)symtab[10].data)->getval()<<endl;
			logAndStop("Breakpoint reached, run gdb to halt here");
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

		case comp_type::_access:{
			vector<lit> cl = (*(dynamic_cast<region *>(fetch_data(opd2))))[((index_ *)fetch_data(opd3))->getval()];
			*(dynamic_cast<cube_ *>(fetch_data(opd1))) = cl;
			(dynamic_cast<cube_ *>(fetch_data(opd1)))->complement();
			break;
		}

		case comp_type::_size:
			fetch_data(opd1)->set(((region *)fetch_data(opd2))->size());
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

		case comp_type::_prop:{
			if (fetch_type(opd1) != type::index)
				logAndStop("Should not reach here");
			if (fetch_type(opd2) != type::index)
				logAndStop("Should not reach here");
			int begin = ((index_ *)fetch_data(opd1))->getval();
			int end = ((index_ *)fetch_data(opd2))->getval();
			prop_clauses(begin, end);
			break;
		}

		case comp_type::_sat:{
			solver instSolver(man_t.AigNtk(), man_t.Network_Cnf());

			instSolver.add(*fetch_data(opd1));
			instSolver.add(*fetch_data(opd2));
			instSolver.add(*fetch_data(opd3));

			instSolver.solve(*fetch_data(opd4));
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
