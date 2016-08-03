/**
 *  @ RSK
 *  @ Version 0.1        24.11.2009
 * 
 *  Classes to store Treebank trees of sentences and process them
 * 
 *  A TBTree (Treebank Tree) is tree node which in turn represent a tree
 *  or subtree by pointing to its childern (which are TBTrees themselves).
 *  So, it also represents tree constitunts when it has a parent (isn't
 *  topmost node)
 *
 *  A TBTree belongs to a corresponding Sentence (see Sentence.hpp)
 *  A TBTree consists of some other TBTrees as its children
 *  A TBTree as a contituent aligns to some Arguments (see Arguments.hpp)
 * 
 */

#ifndef TBTREE_HPP
#define TBTREE_HPP

#include <vector>

#include "Types.hpp"

using namespace std;

class Sentence;
class DataAnalysis;
class Argument;

/**
 * 
 *  the main Treebank tree class 
 * 
 */
class TBTree
{
 private:

  Sentence *sentence;

  TBTree *parent;
  vector<TBTree *> children;
  
  // this is set when needed for the 1st time
  TBTree *headChild;
  // this is set when needed for the 1st time
  TBTree *contentChild;

  string syntTag;
  // word span facilitates accessing constituent words without tracing to 
  // the leaves
  WordSpan wordSpan;

  // a Constituent may map to more than one argument, one for each predicate
  vector<Argument *>  args;
  
  // the word inside the constituent to which the dependency-based argument
  // is assigned when mapping from constitueny to dependency using CoNLL
  // 2008 conversion rules
  // It is placed here, since the Constituent is accessible by Argument
  // and Sample objects
  // The value is loaded in sample generation time to retain the efficiency
  WordPtr depArgWord;

 public:

  TBTree(Sentence &, const string &, const WordIdx &);
  TBTree(Sentence &, TBTree &, const string &, const WordIdx &);
  
  TBTree *getParent();
  bool isRoot();
  bool isTerminal();
  // if this is a coordinated phrase, puts the ordinal of the conjunction
  // child into the passed parameter and returns true; otherwise puts 0
  // there and returns false
  bool isCoordinated(short &);
  TBTree *getChild(short &);
  // fetchs the ordinal of the node among its sibling (e.g. 5th child)
  short getChildOrdinal();
  short getChildrenCount();
  string getSyntTag();
  
  WordPtr setDepArgWord();
  WordPtr getDepArgWord();

  // finds and returns the string contating the path from this to passed
  // node together with traversal directions (the common SRL path feature)
  string getPathTo(TBTree * const &);
  
  WordPtr getHeadWord();
  string getHeadWordForm();
  string getHeadWordLemma();
  Constituent getHeadWordCnstnt();
  Constituent getHeadChild();
  // finds and sets the head child of this constituent using rules in
  // Surdeanu et al. (2008) and Johansson and Negues (2007) and returns
  // the constituent of the head word as a by-product.
  Constituent setHeadChild();
  // finds the head child of NAC and WHNP phrase using Surdeanu et al. rules
  Constituent findCoNLLHeadOfNACWHNP(short &);
  // finds the head child of NAC and WHNP phrase using J&N rules
  Constituent findJNHeadOfNACWHNP(short &);
  
  string getContentWordForm();
  string getContentWordLemma();
  Constituent getContentWord();
  Constituent getContentChild();
  // finds and sets the content child of this constituent using rules in
  // Surdeanu et al. (2003) and returns the constituent of the content
  // word as a by-product.
  Constituent setContentChild();
  
  // returns the subcategorization (CFG) rule from which the parent of 
  // this is expanded (i.e. this is produced
  string getSubcat();
  // returns the Governing Category of the noun phrase which is its first
  // dominating "S" or "VP" node
  string getGovCat();
  
  void setLastWord(const WordIdx &);
  WordSpan getWordSpan();
  string getWordsStr();
  string getWordsLemmaStr();
  unsigned short getWordsCount();
  
  void setProbableArgs();
  void deleteArg(Argument * const &);
  // assigns the [leaf] node (constituent) to the corresponding predicate
  void assignToPropPred(const short &);
  // checks whether the passed argument is assigend to this constituent
  bool hasArgument(Argument * const &);

  void analyzeData(DataAnalysis *&);
  
  void collectChildren(vector<Constituent> &);
  void collectXPCnstnts(vector<Constituent> &);

  void displayTree();
};

#endif /* TBTREE_HPP */

