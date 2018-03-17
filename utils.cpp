#include "api.h"
#include "utils.h"

void logAndStop(string msg){
    cerr<<msg<<endl;
    assert(false);
}

/*! Function
 * Reads .aig file to Abc_Frame_t
 * @param pAbc      [out]       Frame to read file to
 * @param fileName [in]        .aig file name
 * Author(s): Shetal Shah
 */
int readAig(Abc_Frame_t *pAbc, string fileName){
    int fUseResyn2  = 1; // copied from demo.c.

    RUN_CMD(pAbc, "read " + string(fileName)); // read the file
    RUN_CMD(pAbc, "print_stats "); // print the numer of i/o latch and and gates

    // Following commands may modify network, sometimes even removing latches
    // (pdtvisgray0.aig). Keep them commented for now.

    // RUN_CMD(pAbc, "balance"); // balance and cleanup

    // // The next few commands try to reduce the size of the network, remove
    // // dangling nodes, refactor and fraig the network
    // RUN_CMD(pAbc, "scleanup"); // Remove dangling nodes
    // if (fUseResyn2) // Copied from demo.c file
    //     RUN_CMD(pAbc, "balance; rewrite -l; refactor -l; balance; rewrite -l; rewrite -lz; balance; refactor -lz; rewrite -lz; balance");
    // else
    //     RUN_CMD(pAbc, "balance; rewrite -l; rewrite -lz; balance; rewrite -lz; balance");
    // RUN_CMD(pAbc, "fraig");
    // RUN_CMD(pAbc, "print_stats");

    return 0;
}

/*! Function. Creates a Cnf_Dat_t and returns pointer to it.
 *  @param[in]      pAig        Pointer to Aig_Man_t
 *  @return                     Pointer to Cnf_Dat_t of the AIG netowrk
 */
Cnf_Dat_t *aig2cnf(Aig_Man_t *pAig){
    // weird hack used in funcion Pdr_ManNewSolver() in pdrCnf.c
    // TODO figure out why in spare time.
    assert(pAig);

    int nregs = pAig->nRegs;
    pAig->nRegs = Aig_ManCoNum(pAig);
    Cnf_Dat_t *pCnf = Cnf_Derive(pAig, Aig_ManCoNum(pAig));
    pAig->nRegs = nregs;

    assert(pCnf);
    return pCnf;
}

/** Function
 * Adds CNF Formula to Solver
 * @param pSat      [in]        Sat Solver
 * @param pCnf      [in]        Cnf Formula
 *
 * Author(s): Shubham Goel, Sumith Kulal
 */
void addCnfToSolver(sat_solver *pSat, Cnf_Dat_t *pCnf){
    sat_solver_setnvars(pSat, sat_solver_nvars(pSat) + pCnf->nVars);
    for (int i = 0; i < pCnf->nClauses; i++){
        if (!sat_solver_addclause(pSat, pCnf->pClauses[i], pCnf->pClauses[i+1]))
            assert(false);
    }
}

/** Function
 * Adds clauses as CNF Formula to Solver
 * @param pSat      [in]        Sat Solver
 * @param vClauses  [in]        Clauses
 * @param nVars     [in]        Number of variables in CNF
 */
void addClausesToSolver(sat_solver *pSat, vector<vector<lit>> vClauses, int nVars){
    sat_solver_setnvars(pSat, nVars);

    for(int i=0; i<vClauses.size(); ++i){
        if (!sat_solver_addclause(pSat, &vClauses[i][0], &vClauses[i][0]+vClauses[i].size()))
            assert(false);
    }
}

/*! Checks if argument holds a vector which will maintain the data structure's
    invariant.
    @param[in]  U   vector of literals to be sanity-checked.
 */
bool cube_::sanityCheck(const vector<lit> &U){
    if (U.size() == 0) return false;

    for(int i=1; i<U.size(); ++i)
        if (!(lit_var(U[i-1]) < lit_var(U[i])))
            return false;

    return true;
}

/*! Empty Constructor
 */
cube_::cube_(){
    nLits = 0;
}

/*! Constructor. Initialize with vector of literals as cube_
 *  @param[in]  U   A valid vector to be initialized with.
 *                  Must satisfy the data structure invariant.
 */
cube_::cube_(const vector<lit> &U){
    *this = U;
}

/*! Copy constructor. Initialize cube_ given as argument.
 *  @param[in]  init   cube_ to be initalized from.
 */
cube_::cube_(cube_ &init){
    *this = init;
}

/*! Assignment operator.
 *  @param[in]  init   cube_ to be initalized from.
 */
void cube_::operator=(data_struct &init){
    cube_ *rhs = dynamic_cast<cube_ *>(&init);
    assert(rhs);

    *this = rhs->vLits;
}

void cube_::operator=(const vector<lit> &init){
    assert(sanityCheck(init));

    nLits = init.size();
    vLits.clear();
    vLits.insert(vLits.begin(), init.begin(), init.end());
}

/*! Clears the cube_. The resulting cube_ is empty and a tautology.
 */
void cube_::clear(){
    nLits = 0;
    vLits.clear();
}

/*! Dummy function. DO NOT USE FOR NOW. Will be implemented later if needed.
 */
void cube_::conjunct(data_struct &U){
    logAndStop("Not implemented. Will be implemented later");

    // nLits += U.nLits;
    // vLits.insert(vLits.end(), U.vLits.begin(), U.vLits.end());
}

/*! Replace all literals by their complements.
 *  WARN: Use only for single literals.
 *  TODO: Separate data structure for single literals?
 */
void cube_::complement(){
    // assert(nLits == 1);

    for(int i=0; i<nLits; ++i){
        vLits[i] = lit_neg(vLits[i]);
    }
}

/*! Replace each non-primed variable by primed variable and vice-versa.
 *  Throws error if corresponding primed/nonprimed version doesn't exist.
 *  @param[in] toPrimeMap   Mapping from each nonprimed variable to its primed
                            version and vice versa. Can be obtained from InfoMan.
 */
void cube_::toPrime(unordered_map<int, int> &toPrimeMap){
    for(int i=0; i<nLits; ++i){
        unordered_map<lit, lit>::iterator it = toPrimeMap.find(lit_var(vLits[i]));
        assert(it != toPrimeMap.end());

        vLits[i] = toLitCond(it->second, lit_sign(vLits[i]));
    }
}

/*! Casting operator to bool. The cube_ is interpreted as the result of a call
 *  to SAT solver.
 *  - A non empty cube_ implies a successfull counterexample returned by SAT
 *    solver and is interpreted as True.
 *  - An empty cube_ is treated as False
 */
cube_::operator bool() const{
    return !(nLits == 0);
}

/*! Checks if two cube_ are equal.
 *  @param[in]  A   The first cube_
 *  @param[in]  B   The second cube_
 */
bool cube_::operator==(data_struct &U){
    cube_ *rhs = dynamic_cast<cube_ *>(&U);
    assert(rhs);

    if (nLits != rhs->nLits)
        return false;

    for(int i=0; i<nLits; ++i){
        if (vLits[i] != rhs->vLits[i])
            return false;
    }

    return true;
}

/*! TODO implement this.
 */
bool region::sanityCheck(vector<lit> &U){
    if (U.size() == 0)
        return false;

    for(int i=0; i<U.size()-1; ++i)
        if (lit_var(U[i]) == lit_var(U[i+1]))
            return false;

    for(int i=0; i<nClauses; ++i)
        if (std::equal(U.begin(), U.end(), vClauses[i].begin()))
            return false;

    return true;
}

/*! Empty constructor
 */
region::region(){
    nVars = man_t.Network_Cnf()->nVars;
    nClauses = 0;
    nLits = 0;
    vClauses.clear();
}

/*! Constructor. Initialize from Cnf_Dat_t (struct from ABC)/
 *  @param[in]  pCnf    Pointer to valid Cnf_Dat_t.
 */
region::region(Cnf_Dat_t *pCnf){
    *this = pCnf;
}

/*! Copy/Initialization function.
 *  @param[in]  init    CNF as vector of vectors of lits. Each clause should be
                        sorted and should not be a tautology.
    @param[in] nVars_  Number of variables in CNF.
 */
void region::initialize(vector<vector<lit>> init, int nVars_){
    for(int i=0; i<init.size(); ++i){
        sort(init.begin(), init.end());
        assert(sanityCheck(init[i]));
    }

    nClauses = nLits = 0;
    nVars = nVars_;
    vClauses.resize(init.size(), vector<lit>());
    for(int i=0; i<vClauses.size(); ++i){
        vClauses[i].resize(init[i].size(), -1);
        std::copy(init[i].begin(), init[i].end(), vClauses[i].begin());
        nLits += init[i].size();
        nClauses++;
    }
}

/*! Constructor. Parameters are self explanatory.
 *  @param[in]  init    CNF as vector of vectors of lits. Each clause should be
                        sorted and should not be a tautology.
    @param[in]  nVars_  Number of variables in CNF.
 */
region::region(vector<vector<lit>> init, int nVars_){
    initialize(init, nVars_);
}

/*! Copy constructor
 *  @param[in] init     region to be copied from
 */
region::region(const region &init){
    initialize(init.vClauses, init.nVars);
}

/*! Assignment operator
 *  @param[in] init     region to be copied from
 */
void region::operator=(data_struct &init){
    region *rhs = dynamic_cast<region *>(&init);
    assert(rhs);

    // TODO assign if data_struct is cube

    initialize(rhs->vClauses, rhs->nVars);
}

void region::operator=(Cnf_Dat_t *pCnf){
    assert(pCnf);
    //TODO Sanity Check

    nClauses = nLits = 0;
    nVars = pCnf->nVars;
    vClauses.resize(pCnf->nClauses, vector<lit>());
    for(int i=0; i<vClauses.size(); ++i){
        vClauses[i].resize(pCnf->pClauses[i+1]-pCnf->pClauses[i], -1);
        for(int j=0; j<vClauses[i].size(); ++j){
            vClauses[i][j] = pCnf->pClauses[i][j];
            nLits++;
        }
        nClauses++;
    }
    assert(nLits == pCnf->nLiterals);
    assert(nClauses == pCnf->nClauses);
}


/*! Add a clause to the CNF, treating the argument as clause to be added
 *  @param[in]  tmp     Vector to be added as clause. Has to be sorted and
 *                      should not be a tautology.
 *  @param[in]  negate  Set to true if each literal should be negated while adding
 */
void region::addClause(vector<lit> tmp, bool negate){
    if (tmp.size() == 0) return;

    sort(tmp.begin(), tmp.end());

    // TODO: check if adding clause makes any difference

    if (negate)
        for(int i=0; i<tmp.size(); ++i)
            tmp[i] = lit_neg(tmp[i]);

    if(!sanityCheck(tmp)) return;

    vClauses.push_back(tmp);
    nLits += tmp.size();
    nClauses++;
}

void region::conjunct(data_struct &U){
    region *reg = dynamic_cast<region *>(&U);
    cube_ *cub = dynamic_cast<cube_ *>(&U);

    if (reg){
        for(int i=0; i<reg->nClauses; ++i)
            addClause(reg->vClauses[i], false);
        // vClauses.resize(nClauses + reg->nClauses, vector<lit>());
        // for(int i=0; i<reg->nClauses; ++i)
        //     vClauses[nClauses+i] = reg->vClauses[i];

        // nClauses += reg->nClauses;
        // nLits += reg->nLits;
    }
    else if (cub)
        addClause(cub->vLits, true);
    else
        logAndStop("Should not reach here");
}

region::operator bool() const{
    if (vClauses.size() == 0)
        return false;
    else return !(vClauses[0].size() == 0);
}

/*! Access operator. Returns a copy of ith clause as a vector of lits.
 *  @param[in]  i   index of the clause to be accessed
 *  @return         A copy of ith literal returned as a vector<lit>.
 */
vector<lit> region::operator[](int i) const{
    assert(0<=i && i<vClauses.size());

    return vClauses[i];
}

/*! Returns number of clauses in the CNF.
 *  @return         Number of clauses in the CNF.
 */
int region::size() const{
    return vClauses.size();
}

/*! Checks if this region logically implies another region.
 *  @param[in]      B   Number of clauses in the CNF.
 *  @return             true if B is implied, false otherwise
 */
bool region::implies(const region &B) const{
    bool ret_val = true;

    sat_solver *pSolver = sat_solver_new();
    assert(pSolver);

    addToSolver(pSolver);

    for(int i=0; i<B.size(); ++i){
        vector<lit> assumps = B[i];
        for(int j=0; j<assumps.size(); ++j)
            assumps[j] = lit_neg(assumps[j]);

        int status = sat_solver_solve(pSolver,
                &assumps[0], &assumps[0] + assumps.size(),
                (ABC_INT64_T)0, (ABC_INT64_T)0,
                (ABC_INT64_T)0, (ABC_INT64_T)0);

        if (status == l_Undef){
            cout<<"SAT solver result undef"<<endl;
            assert(false);
        }
        else if (status == l_False){
            continue;
        }
        else{
            ret_val = false;
            break;
        }
    }

    sat_solver_delete(pSolver);
    return ret_val;
}

/*! Add this region as a CNF to the solver.
 *  @param[in]  pSolver     Add to this solver, should be a valid pointer
 */
void region::addToSolver(sat_solver *pSolver) const{
    addClausesToSolver(pSolver, vClauses, nVars);
}

/*! Replace each non-primed variable by primed variable and vice-versa.
 *  Throws error if corresponding primed/nonprimed version doesn't exist.
 *  @param[in] toPrimeMap   Mapping from each nonprimed variable to its primed
                            version and vice versa. Can be obtained from InfoMan.
 */
void region::toPrime(unordered_map<int, int> &toPrimeMap){
    for(int i=0; i<nClauses; ++i){
        for(int j=0; j<vClauses[i].size(); ++j){
            unordered_map<lit, lit>::iterator it = toPrimeMap.find(lit_var(vClauses[i][j]));
            assert(it != toPrimeMap.end());

            vClauses[i][j] = toLitCond(it->second, lit_sign(vClauses[i][j]));
        }
    }
}

bool region::operator==(data_struct &init){
    region * rhs = dynamic_cast<region *>(&init);
    assert(rhs);

    if (!(this->implies(*rhs)))
        return false;
    return rhs->implies(*this);
}

//! @return     Value of the index.
int index_::getval(){
    return val;
}

//! Constructor. Sets value to 0.
index_::index_(){
    val = 0;
}

/*! Constructor. Sets value to given integer.
 *  @param[in]  val_    Value to be set
 */
void index_::set(int val_){
    val = val_;
}

//! Increment the index.
void index_::operator++(){
    ++val;
}

//! Decrement the index.
void index_::operator--(){
    // assert(val > 0);
    --val;
}

//! Copy constructor
//! @param[in]      v       index_ to be copied from
void index_::operator=(data_struct &v){
    index_ *rhs = dynamic_cast<index_ *>(&v);
    assert(rhs);

    val = rhs->val;
}

bool index_::operator==(data_struct &v){
    index_ *rhs = dynamic_cast<index_ *>(&v);
    assert(rhs);
    return (val == rhs->val);
}

bool index_::operator>(data_struct &v){
    index_ *rhs = dynamic_cast<index_ *>(&v);
    assert(rhs);
    return (val > rhs->val);
}

bool index_::operator>=(data_struct &v){
    index_ *rhs = dynamic_cast<index_ *>(&v);
    assert(rhs);
    return (val >= rhs->val);
}

bool index_::operator<(data_struct &v){
    index_ *rhs = dynamic_cast<index_ *>(&v);
    assert(rhs);
    return (val < rhs->val);
}

bool index_::operator<=(data_struct &v){
    index_ *rhs = dynamic_cast<index_ *>(&v);
    assert(rhs);
    return (val <= rhs->val);
}

//! Cast to bool. Interpreted as true for nonzero values only.
index_::operator bool() const{
    return !(val == 0);
}

//! Constructor.
solver::solver(Aig_Man_t *pNtk, Cnf_Dat_t *pCnf){
    pSolver = sat_solver_new();
    assert(pSolver);

    int k, idx = 0;
    Aig_Obj_t *pObj;
    vars.resize(Aig_ManRegNum(pNtk), -1);
    Aig_ManForEachLoSeq(pNtk, pObj, k)
        vars[idx++] = pCnf->pVarNums[Aig_ObjId(pObj)];
}

solver::~solver(){
    if (pSolver)
        sat_solver_delete(pSolver);
}

//! Reset the solver completely. All incremental work by solver will be lost.
void solver::reset(){
    if (pSolver)
        sat_solver_delete(pSolver);

    pSolver = sat_solver_new();
    assert(pSolver);

    clearAssumps();
}

//! Clear all the assumptions.
void solver::clearAssumps(){
    assumps.clear();
}

void solver::add(data_struct &U){
    region *reg = dynamic_cast<region *>(&U);
    cube_ *cub = dynamic_cast<cube_ *>(&U);

    if (reg)
        reg->addToSolver(pSolver);
    else if (cub)
        assumps.insert(assumps.end(), cub->vLits.begin(), cub->vLits.end());
    else
        logAndStop("Should not reach here");
}

//! if SAT, returns an assignment of non prime variables
//! Solve with the current list of assumptions. Throws error if undefined result
//! is returned by solver.
//! @param[out]     cex         Assignment of all registers(latch outputs) in
//!                             counterexample.
void solver::solve(data_struct &U){
    cube_ *cex = dynamic_cast<cube_ *>(&U);

    int status = sat_solver_solve(pSolver,
                    &assumps[0], &assumps[0] + assumps.size(),
                    (ABC_INT64_T)0, (ABC_INT64_T)0,
                    (ABC_INT64_T)0, (ABC_INT64_T)0);

    if (status == l_Undef){
        cout<<"SAT solver result undef"<<endl;
        assert(false);
    }
    else if (status == l_False){
        cex->clear();
    }
    else{
        vector<lit> ret(vars.size());
        int *asgn = Sat_SolverGetModel(pSolver, &vars[0], vars.size());
        for(int i=0; i<vars.size(); ++i)
            ret[i] = toLitCond(vars[i], !asgn[i]);
        *cex = ret;
    }
}
