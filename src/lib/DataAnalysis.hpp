/**
 *  @ RSK
 *  @ Version 0.1        03.12.2009
 * 
 *  classes for storing and displaying the analysis result of data
 *  
 */

#ifndef DATAANALYSIS_HPP
#define DATAANALYSIS_HPP

#include <vector>

using namespace std;

class Argument;
class Proposition;
class TBTree;

class DataAnalysis
{
 private:
 
 // numbers of analyzed units
 int sntncsAnalyzed;
 int propsAnalyzed;
 int cnttntsAnalyzed;
 int argsAnalyzed;
 
 // arguments that do not match any constituent token in the tree
 vector<Argument *> argsNoMatch;
 // arguments assigned to leaf nodes (words)
 vector<Argument *> argsOnLeaf;
 
 // arguments that point to a constituent, but the constituent does not
 // point back to the argument (a loading bug!)
 vector<Argument *> argsNoPointBack;
 // constituents that point to an argument, but the argument does not
 // point back to the constituent (a loading bug!)
 vector<TBTree *> cnstntsNoPointBack;
 
 // propsitions with discontinuous compound predicates
 vector<Proposition *> predsDscntCompound;
 // propsitions with continuous compound predicates
 vector<Proposition *> predsCntCompound;
  
 public:
 
  DataAnalysis();
  
  void addSntncAnalyzed();
  void addPropAnalyzed();
  void addCnttntAnalyzed();
  void addArgAnalyzed();
   
  void addArgOnLeaf(Argument * const &);
  void addArgNoMatch(Argument * const &);
  void addArgNoPointBack(Argument * const &);
  void addCnstntNoPointBack(TBTree * const &);
  void addPredDscntCompound(Proposition * const &);
  void addPredCntCompound(Proposition * const &);
  
  void logDataAnalysis(string &);
  void logArgsOnLeaf(ostream &);
};

#endif /* DATAANALYSIS_HPP */
