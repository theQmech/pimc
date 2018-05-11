#include "api.h"

void handle_exit(){
    switch (global_ret_value){
        case _result::SAT:
            cout<<"sat"<<endl;
            break;

        case _result::UNSAT:
            cout<<"uns"<<endl;
            break;

        case _result::TIMEOUT:
            cout<<"time"<<endl;
            break;

        case _result::EMEM:
            cout<<"mem"<<endl;
            break;

        case _result::UNDEF:
            cout<<"unk"<<endl;
            break;

        default:
            cout<<"unhandled"<<endl;
            break;
    }

    Abc_Stop();

    exit(0);
}

void assignWithInit(region &U){
    region tmp(man_t.cInit, man_t.Network_Cnf()->nVars);
    U = tmp;
}

//! For each primed(nonprimed) literal, flip to its nonprimed(primed) version.
//! @param[in]      U       cube_ whose variable are to be flipped. Each literal
///                         must have a valid counterpart.
void toPrime(cube_ &U){
    U.toPrime(man_t.toPrime);
}

//! For each primed(nonprimed) literal, flip to its nonprimed(primed) version.
//! @param[in]      U       cube_ whose variable are to be flipped. Each literal
///                         must have a valid counterpart.
void toPrime(region &U){
    U.toPrime(man_t.toPrime);
}

//! @return A single literal cube_. To set the safety property(P) to true,
//! set this literal to true. To set P' to false, set negation of this literal
//! to true.
cube_ getProperty(){
    vector<lit> tmp(1, toLitCond(man_t.getP(), 0));
    cube_ ret(tmp);
    return ret;
}

void InitProperty(cube_ &U){
    U.nLits = 1;
    U.vLits.clear();
    U.vLits.push_back(toLitCond(man_t.getP(), 1));
}

//! @return Region which represents the transition function in CNF form.
region getT(){
    return region(man_t.Network_Cnf());
    assert(false);
}

void initTransition(region &U){
    U = man_t.Network_Cnf();
}

//! @return Initial states in the form of a region(CNF).
region getInit(){
    return region(man_t.cInit, man_t.Network_Cnf()->nVars);
}

void initInitStates(region &U){
    U.initialize(man_t.cInit, man_t.Network_Cnf()->nVars);
}

/*! Empty constructor.
 */
InfoMan::InfoMan(){
}

/*! Empty constructor.
 */
InfoMan::~InfoMan(){
    if(pCnf)
        Cnf_DataFree(pCnf);
}

/*!
 *  @return Pointer to AIG network.
 */
Aig_Man_t *InfoMan::AigNtk(){
    return pAig;
}

/*!
 *  @return Pointer to CNF holding the network.
 */
Cnf_Dat_t *InfoMan::Network_Cnf(){
    return pCnf;
}

/*! Extracts information from the network and stores in member variables. Call
 *  this function to register the problem instance.
 *  @param[in]  pAbc        Pointer to ABC Frame holding the network
 */
void InfoMan::load_network(Abc_Frame_t *pAbc){
    // cout<<"Loading network"<<endl;

    pNtk = Abc_FrameReadNtk(pAbc);
    pAig = Abc_NtkToDar(pNtk, 0, 1);
    pCnf = aig2cnf(pAig);

    assert(Abc_NtkLatchNum(pNtk) == Aig_ManRegNum(pAig));

    cInit.resize(Aig_ManRegNum(pAig), vector<int>(1, -1));
    int i=0, j=0;
    Abc_Obj_t *pLatch;
    // cout<<"Num regs:\t"<<pAig->nRegs<<endl;
    Abc_NtkForEachLatch(pNtk, pLatch, i){
        cInit[j][0] = toLitCond(
                        pCnf->pVarNums[Aig_ObjId(Saig_ManLo(pAig, j))],
                        !Abc_LatchIsInit1(pLatch));
        ++j;
    }

    // Init the map toPrime
    Aig_Obj_t *pIn, *pOut;
    int k;
    Aig_ManForEachLiLoSeq(pAig, pOut, pIn, k){
        lit in = pCnf->pVarNums[Aig_ObjId(pIn)],
            out = pCnf->pVarNums[Aig_ObjId(pOut)];
        toPrime[in] = out;
        toPrime[out] = in;
    }
}

/*! Get variable number of the safety property of the network.
 */
lit InfoMan::getP(){
    return pCnf->pVarNums[Aig_ObjId(Aig_ManCo(pAig, 0))];
}

vector<lit> implicate(cube_ &cex, region &phi){
    sat_solver *pSolver = sat_solver_new();
    assert(pSolver);

    // add phi
    phi.addToSolver(pSolver);

    int status = sat_solver_solve(pSolver,
                &cex.vLits[0], &cex.vLits[0] + cex.vLits.size(),
                (ABC_INT64_T)0, (ABC_INT64_T)0,
                (ABC_INT64_T)0, (ABC_INT64_T)0);
    if(status != l_False){
        logAndStop("Implicate precondition does not hold");
    }

    vector<lit> retval = rec_implicate(vector<lit>(), cex.vLits, pSolver);

    if (pSolver)
        sat_solver_delete(pSolver);

    return retval;
}

vector<lit>
rec_implicate(vector<lit> supp, vector<lit> comp, sat_solver *pSolver){
    if(comp.size() == 1)
        return comp;

    vector<int> left(comp.begin(), comp.begin() + comp.size()/2);
    vector<int> right(comp.begin() + comp.size()/2, comp.end());

    int status;

    status = sat_solver_solve(pSolver,
                &left[0], &left[0] + left.size(),
                (ABC_INT64_T)0, (ABC_INT64_T)0,
                (ABC_INT64_T)0, (ABC_INT64_T)0);
    if(status = l_False)
        return rec_implicate(supp, left, pSolver);

    status = sat_solver_solve(pSolver,
                &right[0], &right[0] + right.size(),
                (ABC_INT64_T)0, (ABC_INT64_T)0,
                (ABC_INT64_T)0, (ABC_INT64_T)0);
    if(status = l_False)
        return rec_implicate(supp, right, pSolver);


    vector<lit> supp1(supp.begin(), supp.end());
    supp1.insert(supp1.begin(), right.begin(), right.end());
    vector<lit> left0 = rec_implicate(supp1, right, pSolver);

    vector<lit> supp2(supp.begin(), supp.end());
    supp1.insert(supp1.begin(), left0.begin(), left0.end());
    vector<lit> right0 = rec_implicate(supp2, right, pSolver);

    // return
    left0.insert(left0.begin(), right0.begin(), right0.end());
    return left0;
}

vector<lit> min_indc(cube_ &cex, region &init, region &phi){
    region T;
    initTransition(T);
    vector<lit> in_clause = cex.vLits;
    for (auto &&it : in_clause)
        it = lit_neg(it);

    vector<lit> fixpoint = lic(T, phi, init, in_clause);
    vector<lit> ret_val = mic(T, phi, init, fixpoint);

    return ret_val;
}

vector<lit> mic(region &T, region &phi, region &init, vector<lit> in_clause){
    assert(is_inductive(T, phi, init, in_clause));

    vector<lit> curr_clause = in_clause;
    while(curr_clause.size() > 1){
        vector<lit> iter_clause(curr_clause.begin()+1, curr_clause.end());
        bool found = false;

        // iterate over all sub clauses of size less than one
        for(int i=0; i<curr_clause.size()-1; ++i){
            if (is_inductive(T, phi, init, iter_clause)){
                found = true;
                break;
            }
            iter_clause[i] = curr_clause[i];
        }
        if (!found){
            if (is_inductive(T, phi, init, iter_clause)){
                found = true;
            }
        }

        if (!found)
            return curr_clause;
        else
            curr_clause = iter_clause;
    }

    if (curr_clause.size() == 1){
        assert(is_inductive(T, phi, init, curr_clause));
        return curr_clause;
    }

    logAndStop("Should not reach");
}

bool is_inductive(region &T, region &phi, region &init, vector<lit> in_clause){
    cube_ cex(in_clause);
    cex.complement();
    cube_ cex_prime(cex);
    toPrime(cex_prime);

    sat_solver *pSolver = sat_solver_new();
    assert(pSolver);

    init.addToSolver(pSolver);

    // Check that init ---> in_clause
    int status = sat_solver_solve(pSolver,
                &cex.vLits[0],
                &cex.vLits[0] + cex.vLits.size(),
                (ABC_INT64_T)0, (ABC_INT64_T)0,
                (ABC_INT64_T)0, (ABC_INT64_T)0);
    if (status == l_True)
        return false;
    else if (status == l_Undef){
        logAndStop("SAT result undefined");
    }

    sat_solver_delete(pSolver);
    pSolver = sat_solver_new();
    assert(pSolver);

    // Check if phi ^ in_clause ^ T -> in_clause'
    phi.addToSolver(pSolver);
    T.addToSolver(pSolver);
    if (!sat_solver_addclause(pSolver, &in_clause[0],
            &in_clause[0] + in_clause.size()))
        assert(false);
    status = sat_solver_solve(pSolver,
                &cex_prime.vLits[0],
                &cex_prime.vLits[0] + cex_prime.vLits.size(),
                (ABC_INT64_T)0, (ABC_INT64_T)0,
                (ABC_INT64_T)0, (ABC_INT64_T)0);
    if (status == l_True)
        return false;
    else if (status == l_False)
        return true;
    else
        logAndStop("SAT result undefined");
}

// if initiation is not satisifed, then return original clause
// return clause (not cube)
// TODO
vector<lit> lic(region &T, region &init, region &phi, vector<lit> in_clause){
    int status;

    vector<lit> curr_clause(in_clause.begin(), in_clause.end());
    vector<lit> ret_val = curr_clause;

    sat_solver *initSat = sat_solver_new();
    assert(initSat);
    init.addToSolver(initSat);

    while(true){
        assert(curr_clause.size() > 0);

        cube_ cex(curr_clause);
        cex.complement();
        cube_ cex_prime(cex);
        toPrime(cex_prime);

        // check if init -> curr_clause
        status = sat_solver_solve(initSat,
                    &cex.vLits[0],
                    &cex.vLits[0] + cex.vLits.size(),
                    (ABC_INT64_T)0, (ABC_INT64_T)0,
                    (ABC_INT64_T)0, (ABC_INT64_T)0);
        if (status == l_True)
            break;
        else if (status == l_Undef)
            logAndStop("SAT result undefined");

        sat_solver *pSolver = sat_solver_new();
        assert(pSolver);

        T.addToSolver(pSolver);
        phi.addToSolver(pSolver);

        if (!sat_solver_addclause(pSolver, &curr_clause[0],
                &curr_clause[0] + curr_clause.size()))
            assert(false);

        // check if phi ^ T ^ curr_clause -> curr_clause'
        status = sat_solver_solve(pSolver,
                    &cex_prime.vLits[0],
                    &cex_prime.vLits[0] + cex_prime.vLits.size(),
                    (ABC_INT64_T)0, (ABC_INT64_T)0,
                    (ABC_INT64_T)0, (ABC_INT64_T)0);
        if(status == l_False){
            // implication holds, LIC found
            break;
        }
        else if (status == l_True){
            vector<lit> vars;
            for(auto it : curr_clause)
                vars.push_back(lit_var(it));
            int *asgn = Sat_SolverGetModel(pSolver, &vars[0], vars.size());

            vector<lit> new_clause;
            for(int i=0; i<curr_clause.size(); ++i)
                if (asgn[i] != lit_sign(curr_clause[i]))
                    new_clause.push_back(curr_clause[i]);
            if (new_clause.size() == curr_clause.size())
                break;
            curr_clause = new_clause;
            ret_val = curr_clause;
        }
        else{
            logAndStop("SAT result undefined");
        }
    }

    return ret_val;
}
