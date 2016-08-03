/**
 *  @ RSK
 *  @ Version 0.1        03.03.2010
 * 
 *  Classes to store dependency trees of sentences and process them
 * 
 *  A DTree (Dependency Tree) is a node of type Word pointer (see sentence.hpp)
 *  which points to it dependency head as its parent. 
 *  
 *  Note that, to make save memory, we don't implement the tree as usual
 *  (e.g. like TBTree) which contains array of childern for each node.
 *  Instead, we use a search among all nodes of the tree to find children.
 *  (We currently have a severe memoryu problem mainly caused by TBTree
 *   due to vast memory usage of Tree strucure)
 * 
 *  Also note that the content items of each node is of type WordPtr that
 *  differs from TBTree design, where the item WordSpan.
 *
 *  A DTree belongs to a corresponding Sentence (see Sentence.hpp)
 *  A DTree has a pointer to a parent which is its dependency head
 * 
 */

#ifndef DTREE_HPP
#define DTREE_HPP

#include <vector>

#include "Types.hpp"

using namespace std;

class Sentence;

/**
 * 
 *  the main dependency tree class 
 * 
 */
class DTree
{
 private:

  Sentence *sentence;
  
  WordPtr word;
  DTree *parent;
  // dependency relation with the parent (head)
  string depRel;
  
 public:

  DTree(Sentence &);
  DTree(Sentence &, WordPtr &, const string &);
  
  void setParent(DTNode &);
  DTNode getParent();
  bool isRoot();
  DTNode getRoot();
  bool isTerminal();
  void getChildren(vector<DTNode> &);
  void getSiblings(vector<DTNode> &);
  short getChildOrdinal();
  DTNode getLeftSibling();
  DTNode getRightSibling();

  WordIdx getWordIdx();      
  WordPtr getWord();
  string getWordForm();
  string getWordLemma();      
  string getWordPOS();      
  string getDepRel();

  bool isGrandChildOf(DTNode const &);
  bool isGrandParentOf(DTNode const &);
  bool isSibling(DTNode const &);
  bool isCousin(DTNode const &);
  bool isDescendantOf(DTNode const &);
  
  // verifies that the corresponding word of this node is before, after,
  // or aligned to that of parameter node
  unsigned short getPositionRelationTo(DTNode const &);
  
  // finds and returns the common ancestor of this and parameter node
  DTNode getLCAncestorWith(DTNode const &);


  // finds and returns the string of dependency relations along the path
  // from this to passed node together with traversal directions
  string getRelPathTo(DTNode const &);

  // same as getRelPathTo() but augmenting the path with left/right direction
  // in addition to up/down
  string getARelPathTo(DTNode const &);

  // extract the dependency relations of nodes in the vector by iterating 
  // it in the order specified by bool parameter and concatenate them in
  // using the last parameter
  string getRelString(vector<DTNode> &, bool, string);

  // same as getRelString() but also adding left/right direction signs
  string getARelString(vector<DTNode> &, bool, string);

  // finds and returns the string of POS tags along the path from this to
  // passed node together with traversal directions (both up/down and left/right) 
  string getPOSPathTo(DTNode const &);

  // same as getPOSPathTo() but augmenting the path with left/right direction
  // in addition to up/down
  string getAPOSPathTo(DTNode const &);

  // extracts the POS tags of nodes in the vector by iterating it in the 
  // order specified by bool parameter and concatenates them using the
  // last parameter and left/right signs as direction signs 
  string getPOSString(vector<DTNode> &, bool, string);

  // same as getPOSString() but also adding left/right direction signs
  string getAPOSString(vector<DTNode> &, bool, string);
  
  // finds and returns the string of word lemmas along the path from this to
  // passed node together
  string getLemmaPathTo(DTNode const &);

  // extracts the lemmas of nodes in the vector by iterating it in the 
  // order specified by bool parameter and concatenates them using the
  // last parameter
  string getLemmaString(vector<DTNode> &, bool, string);
  
  // returns the length of path from this node to least common ancestor
  // of it and the passed parameter
  unsigned short getPathLengthToLCA(DTNode &);

  // returns the family relationship of this node to the node passed as parameter
  unsigned short getFamRelTo(DTNode const &);
  
  // returns the dependency relation pattern between the node and its children
  string getChldRelPattern();
  
  // returns the POS pattern of the node's children
  string getChldPOSPattern();
  
  // returns the dependency relation pattern between the node's parent 
  // and its children (node's siblings) excluding the node itself
  string getSibRelPattern();
  
  // returns the POS pattern between the node's parent and its children
  // (node's siblings) excluding the node itself
  string getSibPOSPattern();
  
};

#endif /* DTREE_HPP */

