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
    RUN_CMD(pAbc, "balance"); // balance and cleanup

    // The next few commands try to reduce the size of the network, remove
    // dangling nodes, refactor and fraig the network
    RUN_CMD(pAbc, "scleanup"); // Remove dangling nodes
    if (fUseResyn2) // Copied from demo.c file
        RUN_CMD(pAbc, "balance; rewrite -l; refactor -l; balance; rewrite -l; rewrite -lz; balance; refactor -lz; rewrite -lz; balance");
    else
        RUN_CMD(pAbc, "balance; rewrite -l; rewrite -lz; balance; rewrite -lz; balance");
    RUN_CMD(pAbc, "fraig");
    RUN_CMD(pAbc, "print_stats");

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
    assert(sanityCheck(U));

    nLits = U.size();
    vLits.insert(vLits.begin(), U.begin(), U.end());
}

/*! Copy constructor. Initialize cube_ given as argument.
 *  @param[in]  init   cube_ to be initalized from.
 */
cube_::cube_(const cube_ &init){
    *this = init;
}

/*! Assignment operator.
 *  @param[in]  init   cube_ to be initalized from.
 */
void cube_::operator=(const cube_ &init){
    nLits = init.nLits;
    vLits.clear();
    vLits.insert(vLits.begin(), init.vLits.begin(), init.vLits.end());
}

/*! Clears the cube_. The resulting cube_ is empty and a tautology.
 */
void cube_::clear(){
    nLits = 0;
    vLits.clear();
}

/*! Dummy function. DO NOT USE FOR NOW. Will be implemented later if needed.
 */
void cube_::conjunct(const cube_ &U){
    assert(false);

    // nLits += U.nLits;
    // vLits.insert(vLits.end(), U.vLits.begin(), U.vLits.end());
}

/*! Replace all literals by their complements.
 *  WARN: Use only for single literals.
 *  TODO: Separate data structure for single literals?
 */
void cube_::complement(){
    assert(nLits == 1);

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
bool operator==(const cube_ &A, const cube_ &B){
    if (A.nLits != B.nLits)
        return false;

    for(int i=0; i<A.nLits; ++i){
        if (A.vLits[i] != B.vLits[i])
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
        for(int j=0; j<vClauses[i].size(); ++j){
            vClauses[i][j] = init[i][j];
            nLits++;
        }
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
void region::operator=(const region&init){
    initialize(init.vClauses, init.nVars);
}

/*! Add a clause to the CNF, treating the argument as clause to be added
 *  @param[in]  tmp     Vector to be added as clause. Has to be sorted and
 *                      should not be a tautology.
 *  @param[in]  negate  Set to true if each literal should be negated while adding
 */
void region::addClause(vector<lit> tmp, bool negate){
    if (tmp.size() == 0) return;

    sort(tmp.begin(), tmp.end());
    assert(sanityCheck(tmp));

    // TODO: check if adding clause makes any difference

    if (negate)
        for(int i=0; i<tmp.size(); ++i)
            tmp[i] = lit_neg(tmp[i]);

    vClauses.push_back(tmp);
    nLits += tmp.size();
    nClauses++;
}

/*! Add a negation of the cube_ to the CNF. Equivalent to region <- region ^ !U
 *  Useful when adding negation of counterexample to a CNF.
 *  @param[in]  U     cube_ whose negation is to be conjucted.
 */
void region::conjunct(const cube_ &U){
    addClause(U.vLits, true);
}

/*! Conjunct a CNF.
 *  @param[in]  U     region to be conjucted.
 */
void region::conjunct(const region &U){
    // TODO: check if each clause should be added or not

    vClauses.resize(nClauses+U.nClauses, vector<lit>());
    for(int i=0; i<U.nClauses; ++i)
        vClauses[nClauses+i] = U.vClauses[i];

    nClauses += U.nClauses;
    nLits += U.nLits;
}

/*! Casting operator to bool. The region is interpreted as the result of a call
 *  to SAT solver.
 *  - A non empty region implies a successfull counterexample returned by SAT
 *    solver and is interpreted as True.
 *  - An empty region is treated as False.
 */
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

/*! Checks logical equivalence of the two regions.
 *  @param[in]      A   region 1
 *  @param[in]      B   region 2
 *  @return             true if regions are logically equivalent.
 *  Internally checks if A implies B and B implies A.
 */
bool operator==(const region &A, const region &B){
    if (!A.implies(B))
        return false;
    return B.implies(A);
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
    --val;
}

//! Copy constructor
//! @param[in]      v       index_ to be copied from
void index_::operator=(const index_ &v){
    val = v.val;
}

//! Comparator operator for index_
//! @param[in]      v       index_ to be compared with
//! @return                 true iff self value is less.
bool index_::operator<(const index_ &v){
    return (val<v.val);
}

//! Check equality for index_
//! @param[in]      v       index_ to be compared with
//! @return                 true iff self value is equal.
bool index_::operator==(const index_ &v){
    return (val == v.val);
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

//! Add to list of assumptions to be made by solver while solving.
//! @param[in]      U       cube_ containing list of assumptions to be added.
void solver::addAssumps(const cube_ &U){
    assumps.insert(assumps.end(), U.vLits.begin(), U.vLits.end());
}

//! Adds a region(CNF) to the solver.
//! @param[in]      U       region to be added.
void solver::addRegion(const region &U){
    U.addToSolver(pSolver);
}

//! if SAT, returns an assignment of non prime variables
//! Solve with the current list of assumptions. Throws error if undefined result
//! is returned by solver.
//! @param[out]     cex         Assignment of all registers(latch outputs) in
//!                             counterexample.
void solver::solve(cube_ &cex){
    int status = sat_solver_solve(pSolver,
                    &assumps[0], &assumps[0] + assumps.size(),
                    (ABC_INT64_T)0, (ABC_INT64_T)0,
                    (ABC_INT64_T)0, (ABC_INT64_T)0);

    if (status == l_Undef){
        cout<<"SAT solver result undef"<<endl;
        assert(false);
    }
    else if (status == l_False){
        cex.clear();
    }
    else{
        int *asgn = Sat_SolverGetModel(pSolver, &vars[0], vars.size());
        for(int i=0; i<vars.size(); ++i)
            vars[i] = toLitCond(vars[i], !asgn[i]);
        cex = cube_(vars);
    }
}
