#ifndef UTILS_H
#define UTILS_H

extern "C"{
#include "base/abc/abc.h"
#include "aig/saig/saig.h"
#include "sat/cnf/cnf.h"
#include "sat/bsat/satSolver.h"
#include "base/main/main.h"
Aig_Man_t * Abc_NtkToDar(Abc_Ntk_t * pNtk, int fExors, int fRegisters);
}

#include "../comm.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>

using namespace std;

/*! Macro function
 *  Executes command in the ABC_Frame_t. May print logs to console.
 *  @param pAbc      [in]       Frame to read file to
 *  @param cmd       [in]       std::string or char* containing the command
 */
#define RUN_CMD(pAbc, cmd) ({                                                  \
    if (Cmd_CommandExecute(pAbc, string(cmd).c_str())){                        \
        cout<<"Cannot execute command\t"<<cmd<<endl;                           \
        return 1;                                                              \
    }                                                                          \
})

extern bool VERBOSE;
extern bool DEBUG;

void logAndStop(string msg);
int readAig(Abc_Frame_t *pAbc, string fileName);
Cnf_Dat_t *aig2cnf(Aig_Man_t *pAig);
void addCnfToSolver(sat_solver *pSat, Cnf_Dat_t *pCnf);
void
addClausesToSolver(sat_solver *pSat, vector<vector<lit>> vClauses, int nVars);

/*! Data structure for a conjunction of literals i.e. a cube. The internal
    vector representation maintains the following invariant:
 *      - is either a cube of size 0 (tautology).
 *      - or has distinct elements in sorted order and is not a contradiction.
 *  The last point means that p and !p should not be both present in the cube
 */
class cube_ : public data_struct{
public:
    /// Number of literals in the cube
    int nLits;
    /// Vector holding the literals
    vector<lit> vLits;
    bool sanityCheck(const vector<lit> &);

    cube_();
    cube_(const vector<lit> &);
    cube_(cube_ &);
    void operator=(data_struct &);
    void operator=(cube_ &);
    void operator=(const vector<lit> &);

    void clear();
    void conjunct(data_struct &);
    void complement();
    void toPrime(unordered_map<lit, lit> &);

    operator bool() const;
    bool operator==(data_struct &);

    void print();
};

/*! Data structure holding a CNF formula. Internally is stored as a vector of
 *  vectors.
 *  TODO Optimizations
 */
class region : public data_struct{
public:
    /// Number of variables in CNF.
    int nVars;
    /// Number of ckayses in CNF.
    int nClauses;
    /// Total number of literals CNF.
    int nLits;
    /// Vector holding all clauses. Each clause is a vector of literals.
    vector<vector<lit>> vClauses;

    bool sanityCheck(vector<lit> &U);

    region();
    region(Cnf_Dat_t *pCnf);
    void initialize(vector<vector<lit>> init, int nVars_);
    region(vector<vector<lit>> init, int nVars_);
    region(const region &init);
    void operator=(data_struct &init);
    void operator=(Cnf_Dat_t *pCnf);
    void addClause(vector<lit> tmp, bool negate);
    void conjunct(data_struct &U);
    // void conjunct(const region &U);

    operator bool() const;
    vector<lit> operator[](int i) const;
    int size() const;

    int implies(const region &B) const;
    void addToSolver(sat_solver *pSolver) const;
    void toPrime(unordered_map<lit, lit> &toPrimeMap);
    bool operator==(data_struct &init);

    void print();
};

/*! Data structure for the index.
 */
class index_ : public data_struct{
    /// Value of the index
    int val;
public:
    int getval();
    index_();
    void set(int val_);
    void operator++();
    void operator--();
    void operator=(data_struct &v);
    bool operator==(data_struct &v);
    bool operator>(data_struct &v);
    bool operator>=(data_struct &v);
    bool operator<(data_struct &v);
    bool operator<=(data_struct &v);

    operator bool() const;
};

class coll_ : public data_struct{
    int sz;
    vector<data_struct *> vMem;
public:
    coll_();
    void operator=(const region &);
    data_struct *operator[](int i);
    int size();
};

//! Wrapper to SAT solver provided by ABC. ABC does not allow us to add single
//! literals to solver. Add clauses in the form of a CNF(region) via member
//! function addRegion, and single literals in the form of cube_ via addAssumps.
class solver{
    sat_solver *pSolver;
    vector<int> vars;
public:
    vector<lit> assumps;
    solver(Aig_Man_t *pNtk, Cnf_Dat_t *pCnf);
    ~solver();
    void reset();

    void add(data_struct &U);
    void clearAssumps();
    // void addAssumps(const cube_ &U);
    // void addRegion(const region &U);
    void solve(data_struct &U);
};

#endif
