#include "lre/symbols.h"
#include "lre/symtab.h"
#include "utils.h"
#include "api.h"

extern bool VERBOSE;
extern bool DEBUG;

void extra_debug();

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
			extra_debug();
			if (VERBOSE)
				cout<<"At break point "<<loc<<endl;
			if (DEBUG)
				logAndStop("Breakpoint reached, run gdb to halt here");
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
				*lhs = (*rhs2)[res];

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

		case comp_type::_gen2:{
			cube_ *rval = dynamic_cast<cube_ *>(fetch_data(opd1));
			cube_ *cex = dynamic_cast<cube_ *>(fetch_data(opd2));
			region *init = dynamic_cast<region *>(fetch_data(opd3));
			region *phi = dynamic_cast<region *>(fetch_data(opd4));

			assert(rval);
			assert(cex);
			assert(init);
			assert(phi);

			if (cex->vLits.size() == 4){
				if (cex->vLits[0] == 40 && cex->vLits[1] == 42 && cex->vLits[2] == 44
					&& cex->vLits[3] == 46)
					cout<<"  "<<endl;
			}


			*rval = min_indc(*cex, *init, *phi);
			rval->complement();

			break;
		}

		case comp_type::_return:
			if (val == 0)
				global_ret_value = _result::SAT;
			else if (val == 1)
				global_ret_value = _result::UNSAT;
			else
				global_ret_value = _result::UNDEF;
			// extra_debug();
			handle_exit();
			break;

		default:
			logAndStop("Should not reach here");
			break;
	}
}

void extra_debug(){
	// cout<<"DEBUG_START--------------------------------------------------"<<endl;
	// cout<<"cex :"<<endl;
	// ((cube_ *)symtab[symtab.get_symbol("cex")].data)->print();
	// cout<<"maxframe :\t";

	cout<<((index_ *)symtab[symtab.get_symbol("currframe")].data)->getval()<<endl;
	region *frame = (region *)symtab[symtab.get_symbol("CNF_ARR")].data;
	int maxfram = ((index_ *)symtab[symtab.get_symbol("currframe")].data)->getval();

	// cube_ cex_____ = *(cube_ *)symtab[symtab.get_symbol("cex")].data;
	// if (maxfram == 1)
	// 	if (cex_____.vLits[0] == 42 && cex_____.vLits[1] == 46){
	// 		++maxfram;
	// 		cout<<"  "<<endl;
	// 	}

	for(int i=0; i<maxfram; ++i){
		int res = frame[i].implies(frame[i+1]);
		if (res > -1){
			cout<<"Frame_"<<i<<" does not imply Frame_"<<i+1<<endl;
			frame[i].print();
			frame[i+1].print();
			assert(false);
		}
		else
			cout<<"Frame_"<<i<<"\t\t-->\tFrame_"<<i+1<<endl;
	}

	cout<<endl;

	for(int i=0; i<maxfram; ++i){
		int res = frame[i+1].implies(frame[i]);
		if (res > -1){
		}
		else
			cout<<"Frame_"<<i+1<<"\t\t-->\tFrame_"<<i<<endl;
	}

	cout<<endl;

	for(int i=0; i<maxfram; ++i){

		region l1;
		initTransition(l1);
		l1.conjunct(frame[i]);

		region l2(frame[i+1]);
		toPrime(l2);

		int res = l1.implies(l2);
		if (res > -1){
			cout<<"Frame_"<<i<<" ^ T does not imply Frame_"<<i+1<<"'"<<endl;
			frame[i].print();
			frame[i+1].print();
			l1.print();
			l2.print();
			cout<<res<<endl;
			assert(false);
		}
		else
			cout<<"Frame_"<<i<<" ^ T\t\t-->\tFrame_"<<i+1<<"'"<<endl;
	}

	cout<<endl;
	cout<<"DEBUG_END----------------------------------------------------"<<endl;
	cout<<endl;

}
