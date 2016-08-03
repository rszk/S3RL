/**
 *  @ RSK
 *  @ Version 0.1        03.03.2010
 * 
 *  (refer to DTree.hpp)
 * 
 */

#include <iostream>
#include <deque>

#include "Sentence.hpp"
#include "DTree.hpp"
#include "Utils.hpp"
#include "Constants.hpp"


/**
 * 
 *  DTree class definitions
 * 
 */


/**
 *  Root node constructor
 */ 
DTree::DTree(Sentence &pSentence)
{
 sentence = &pSentence;
 word = NULL;
 parent = NULL;
}

/**
 *  non-Root node constructor
 */ 
DTree::DTree(Sentence &pSentence, WordPtr &pWord, const string &pDepRel)
{
 sentence = &pSentence;
 word = pWord;
 parent = NULL;
 depRel = pDepRel;
}

void DTree::setParent(DTNode &pParent)
{
 parent = pParent;
}

DTNode DTree::getParent()
{
 return parent;
}

bool DTree::isRoot()
{
 return (parent == NULL);
}

DTNode DTree::getRoot()
{
 return sentence->getDTree();
}

bool DTree::isTerminal()
{
 if (isRoot())
  return false;
 
 vector<DTNode> vcChildren;
 getChildren(vcChildren);
 return (vcChildren.size() == 0);
}

/**
 *  Since we do not store links to children for a DTree node due to current
 *  memory problem, we collect them on-demand. Moreover, although each 
 *  sentence has a link to its DTree (root node), but we cannot access nodes 
 *  via it due to the same problem. So, we just iterate words and look at
 *  their corrsponding DTree nodes to see whether the parent of a node
 *  is the node (this) that we are looking for its children.
 */ 
void DTree::getChildren(vector<DTNode> &pChildren)
{
 for (unsigned short cntWord = 1; cntWord <= sentence->getLength(); cntWord++)
 { 
  DTNode vCandidate = sentence->getWord(cntWord)->getDTNode();
  if (vCandidate->getParent() == this)
   pChildren.push_back(vCandidate);
  }
}

void DTree::getSiblings(vector<DTNode> &pSiblings)
{
 for (unsigned short cntWord = 1; cntWord <= sentence->getLength(); cntWord++)
 { 
  DTNode vCandidate = sentence->getWord(cntWord)->getDTNode();
  if ((vCandidate->getParent() == this->getParent()) && (!(vCandidate == this)))
   pSiblings.push_back(vCandidate);
  }
}

short DTree::getChildOrdinal()
{
 // if it is root node, it is not child, so return 0
 if (isRoot())
  return 0;
 
 vector<DTNode> vSiblings;	
 parent->getChildren(vSiblings);
 
 // it will return ordinal not the index of vector
 short vOrdinal = 1;
 
 for (vector<DTNode>::iterator itSibling = vSiblings.begin(); 
      itSibling < vSiblings.end(); 
      itSibling++, vOrdinal++)
  if ((*itSibling) == this)
   return vOrdinal;
 
 return -1;
}

DTNode DTree::getLeftSibling()
{
 // index of the left sibling in the siblings vector
 short vLeftSiblingIdx = getChildOrdinal() - 2;
 
 // if the index is not valid, there is no left sibling
 if (vLeftSiblingIdx < 0)
  return NULL;
 else
 { 
  vector<DTNode> vSiblings;	
  getSiblings(vSiblings);
  
  return vSiblings[vLeftSiblingIdx];
  }
}

DTNode DTree::getRightSibling()
{
 // index of the right sibling in the siblings vector
 unsigned short vRightSiblingIdx = getChildOrdinal();

 vector<DTNode> vSiblings;	
 getSiblings(vSiblings);

 // if the index is not valid, there is no right sibling
 if (vRightSiblingIdx >= vSiblings.size())
  return NULL;
 else
  return vSiblings[vRightSiblingIdx];
}

WordIdx DTree::getWordIdx()
{
 return sentence->getWordIdx(word);
}

WordPtr DTree::getWord()
{
 return word;
}

string DTree::getWordForm()
{
 return word->getWordForm();
}

string DTree::getWordLemma()
{
 return word->getWordLemma();
}

string DTree::getWordPOS()
{
 return word->getPOS();
}

string DTree::getDepRel()
{
 return depRel;
}

bool DTree::isGrandChildOf(DTNode const &pNode)
{
 if (parent)
  return (parent->parent == pNode);
 else
  return false; 
}

bool DTree::isGrandParentOf(DTNode const &pNode)
{
 if (pNode->parent)
  return (this == pNode->parent->parent);
 else
  return false; 
}

bool DTree::isSibling(DTNode const &pNode)
{
 return (parent == pNode->parent);
}

bool DTree::isCousin(DTNode const &pNode)
{
 return (!isSibling(pNode) && (parent->parent == pNode->parent->parent));
}

bool DTree::isDescendantOf(DTNode const &pNode)
{
 DTNode vCurr = this;
 
 while (!vCurr->isRoot()) 
 {
  if (vCurr == pNode)
   return true;
  else
   vCurr = vCurr->parent;
  }
 
 return false; 
}

unsigned short DTree::getPositionRelationTo(DTNode const &pOtherNode)
{
 WordIdx vThisIdx = getWordIdx();
 WordIdx vOtherIdx = pOtherNode->getWordIdx();
 	
 if (vThisIdx == vOtherIdx)
  return ALIGNED;
 else if (vThisIdx < vOtherIdx)
  return BEFORE;
 else
  return AFTER; 
}

/**
 *  finds the least common ancestor of this node and the parameter node
 *  in the same way as getPathTo()function (see below)
 */ 
DTNode DTree::getLCAncestorWith(DTNode const &pOtherNode)
{
 if (parent == pOtherNode)
  return pOtherNode;
 else if (this == pOtherNode->parent)
  return this;
 else if (isGrandChildOf(pOtherNode))
  return pOtherNode;
 else if (isGrandParentOf(pOtherNode))
  return this;
 else if (isSibling(pOtherNode))
  return parent;
 
 vector<DTNode> vcThisToRoot, vcOtherToRoot;
 
 vcThisToRoot.push_back(this);
 
 // climbing up from the source node (this)
 do 
 {
  if (vcThisToRoot.back()->parent == pOtherNode)
   return pOtherNode;
  else
   vcThisToRoot.push_back(vcThisToRoot.back()->parent);
    
  } while (!vcThisToRoot.back()->isRoot());
  

 vcOtherToRoot.push_back(pOtherNode);

 // climbing up from the other node
 do 
 {
  if (vcOtherToRoot.back()->parent == this)
   return this;
  else
   vcOtherToRoot.push_back(vcOtherToRoot.back()->parent);
    
  } while (!vcOtherToRoot.back()->isRoot());
  
 // climbing down from the root, while comparing two stacks (vectors) for
 // the first uncommon node, the node before which is the least common ancestor
 
 DTNode vLCAncestor;
 
 while (vcThisToRoot.back() == vcOtherToRoot.back())
 {
  vLCAncestor = vcThisToRoot.back();
  vcThisToRoot.pop_back();
  vcOtherToRoot.pop_back();
  }	 
  
 return vLCAncestor;
}


/**
 *  To find the path from this node to destination node, the tree is climbed up
 *  until the root node. Along the way, the nodes are checked against the
 *  destination node, since this node may be a grand child of the destination.
 *  The same thing is done for destination. Finally, if none of them was 
 *  faced during the process for the other, starting from the top of the 
 *  stacks, it compares them to find the 1st uncommon node and pops the 
 *  previous common node from stacks if the current node is also common.
 *  The node before the 1st uncommon node (the top of stacks) is the least 
 *  common ancestor, and the path is derived by combining 2 stacks.
 * 
 *  Note:
 *  - To retain efficiency and simplicity, the function first looks for simple
 *  relation between source and dest like child-parent, sibling, and grandchild-
 *  grandparent and cousin.
 */ 

string DTree::getRelPathTo(DTNode const &pDest)
{
 if (parent == pDest)
  return depRel + "/";
 else if (this == pDest->parent)
  return pDest->depRel + "\\";
 else if (isGrandChildOf(pDest))
  return depRel + "/" + parent->depRel + "/";
 else if (isGrandParentOf(pDest))
  return pDest->parent->depRel + "\\" + pDest->depRel + "\\";
 else if (isSibling(pDest))
  return depRel + "/"  + "\\" + pDest->depRel;
 
 vector<DTNode> vcSrcToRoot, vcDestToRoot;
 

 vcSrcToRoot.push_back(this);
 
 // climbing up from the source node (this)
 do 
 {
  if (vcSrcToRoot.back()->parent == pDest)
   return getRelString(vcSrcToRoot, true, "/");
  else
   vcSrcToRoot.push_back(vcSrcToRoot.back()->parent);
    
  } while (!vcSrcToRoot.back()->isRoot());
  

 vcDestToRoot.push_back(pDest);

 // climbing up from the destination node
 do 
 {
  if (vcDestToRoot.back()->parent == this)
   return getRelString(vcDestToRoot, false, "\\");
  else
   vcDestToRoot.push_back(vcDestToRoot.back()->parent);
    
  } while (!vcDestToRoot.back()->isRoot());
  
 // climbing down from the root, while comparing two stacks (vectors) for
 // the first uncommon node, the node before which is the least common ancestor
 
 while (vcSrcToRoot.back() == vcDestToRoot.back())
 {
  vcSrcToRoot.pop_back();
  vcDestToRoot.pop_back();
  }	 
  
 return getRelString(vcSrcToRoot, true, "/") + getRelString(vcDestToRoot, false, "\\");
}

string DTree::getRelString(vector<DTNode> &pNodes, bool pDirection, string pUpDownDir)
{
 string vRelString;

 // if direction is true, treat the vector as queue, otherwise as stack
 if (pDirection)
 {
  for (vector<DTNode>::iterator itNode = pNodes.begin(); 
       itNode < pNodes.end(); 
       itNode++)
   vRelString += (*itNode)->getDepRel() + pUpDownDir;
  }
 else
 {
  for (vector<DTNode>::reverse_iterator ritNode = pNodes.rbegin(); 
       ritNode < pNodes.rend(); 
       ritNode++)
   vRelString += pUpDownDir + (*ritNode)->getDepRel();
  } 

 return vRelString;
}

/** 
 *  Same as getRelPath() but the output path string include left/right 
 *  direction in addidtion to up/down, which is based on the word position
 *  (WordIdx) of the current word with respect to the next word in the path.
 */
string DTree::getARelPathTo(DTNode const &pDest)
{
 if (parent == pDest)
  return depRel + "/" + ((getPositionRelationTo(pDest) == BEFORE) ? ">" : "<");
 else if (this == pDest->parent)
  return pDest->depRel + "\\" + ((pDest->getPositionRelationTo(this) == BEFORE) ? ">" : "<");
 else if (isGrandChildOf(pDest))
  return depRel + "/" + ((getPositionRelationTo(parent) == BEFORE) ? ">" : "<") + 
         parent->depRel + "/"+ ((parent->getPositionRelationTo(pDest) == BEFORE) ? ">" : "<");
 else if (isGrandParentOf(pDest))
  return pDest->parent->depRel + "\\" + ((pDest->parent->getPositionRelationTo(parent) == BEFORE) ? ">" : "<") + 
         pDest->depRel + "\\" + ((pDest->getPositionRelationTo(this) == BEFORE) ? ">" : "<");
 else if (isSibling(pDest))
  return depRel + "/"  + ((getPositionRelationTo(parent) == BEFORE) ? ">" : "<") + 
         ((parent->getPositionRelationTo(pDest) == BEFORE) ? ">" : "<") + "\\" +
         pDest->depRel;
 
 vector<DTNode> vcSrcToRoot, vcDestToRoot;
 

 vcSrcToRoot.push_back(this);
 
 // climbing up from the source node (this)
 do 
 {
  if (vcSrcToRoot.back()->parent == pDest)
   return getARelString(vcSrcToRoot, true, "/");
  else
   vcSrcToRoot.push_back(vcSrcToRoot.back()->parent);
    
  } while (!vcSrcToRoot.back()->isRoot());
  

 vcDestToRoot.push_back(pDest);

 // climbing up from the destination node
 do 
 {
  if (vcDestToRoot.back()->parent == this)
   return getARelString(vcDestToRoot, false, "\\");
  else
   vcDestToRoot.push_back(vcDestToRoot.back()->parent);
    
  } while (!vcDestToRoot.back()->isRoot());
  
 // climbing down from the root, while comparing two stacks (vectors) for
 // the first uncommon node, the node before which is the least common ancestor
 
 while (vcSrcToRoot.back() == vcDestToRoot.back())
 {
  vcSrcToRoot.pop_back();
  vcDestToRoot.pop_back();
  }	 
  
 return getARelString(vcSrcToRoot, true, "/") + getARelString(vcDestToRoot, false, "\\");
}

string DTree::getARelString(vector<DTNode> &pNodes, bool pDirection, string pUpDownDir)
{
 string vRelString;

 // if direction is true, treat the vector as queue, otherwise as stack
 if (pDirection)
 {
  for (vector<DTNode>::iterator itNode = pNodes.begin(); 
       itNode < pNodes.end(); 
       itNode++)
   vRelString += (*itNode)->getDepRel() + pUpDownDir + 
                 (((*itNode)->getPositionRelationTo((*itNode)->parent) == BEFORE) ? ">" : "<");
  }
 else
 {
  for (vector<DTNode>::reverse_iterator ritNode = pNodes.rbegin(); 
       ritNode < pNodes.rend(); 
       ritNode++)
   vRelString += (((*ritNode)->getPositionRelationTo((*ritNode)->parent) == BEFORE) ? ">" : "<") + 
                 pUpDownDir + (*ritNode)->getDepRel();
  } 

 return vRelString;
}

/**
 *  Refer to getRelPathTo() comments
 */
string DTree::getPOSPathTo(DTNode const &pDest)
{
 if (parent == pDest)
  return getWordPOS() + "/";
 else if (this == pDest->parent)
  return pDest->getWordPOS() + "\\";
 else if (isGrandChildOf(pDest))
  return getWordPOS() + "/" + parent->getWordPOS() + "/";
 else if (isGrandParentOf(pDest))
  return pDest->parent->getWordPOS() + "\\" + pDest->getWordPOS() + "\\";
 else if (isSibling(pDest))
  return getWordPOS() + "/\\" + pDest->getWordPOS();
 
 vector<DTNode> vcSrcToRoot, vcDestToRoot;
 

 vcSrcToRoot.push_back(this);
 
 // climbing up from the source node (this)
 while (!vcSrcToRoot.back()->isRoot()) 
 {
  if (vcSrcToRoot.back()->parent == pDest)
   return getPOSString(vcSrcToRoot, true, "/");
  else
   vcSrcToRoot.push_back(vcSrcToRoot.back()->parent);
    
  }
  

 vcDestToRoot.push_back(pDest);

 // climbing up from the destination node
 while (!vcDestToRoot.back()->isRoot()) 
 {
  if (vcDestToRoot.back()->parent == this)
   return getPOSString(vcDestToRoot, false, "\\");
  else
   vcDestToRoot.push_back(vcDestToRoot.back()->parent);
    
  }
  
 // climbing down from the root, while comparing two stacks (vectors) for
 // the first uncommon node, the node before which is the least common ancestor
 
 while (vcSrcToRoot.back() == vcDestToRoot.back())
 {
  vcSrcToRoot.pop_back();
  vcDestToRoot.pop_back();
  }	 
  
 return getPOSString(vcSrcToRoot, true, "/") + getPOSString(vcDestToRoot, false, "\\");
}

string DTree::getPOSString(vector<DTNode> &pNodes, bool pDirection, string pUpDownDir)
{
 string vPOSString;

 // if direction is true, treat the vector as queue, otherwise as stack
 if (pDirection)
 {
  for (vector<DTNode>::iterator itNode = pNodes.begin(); 
       itNode < pNodes.end(); 
       itNode++)
   vPOSString += (*itNode)->getWordPOS() + pUpDownDir;
  }
 else
 {
  for (vector<DTNode>::reverse_iterator ritNode = pNodes.rbegin(); 
       ritNode < pNodes.rend(); 
       ritNode++)
   vPOSString += pUpDownDir + (*ritNode)->getWordPOS();
  } 

 return vPOSString;
}

/**
 *  Refer to getARelPathTo() comments
 */
string DTree::getAPOSPathTo(DTNode const &pDest)
{
 if (parent == pDest)
  return getWordPOS() + "/" + ((getPositionRelationTo(pDest) == BEFORE) ? ">" : "<");
 else if (this == pDest->parent)
  return pDest->getWordPOS() + "\\" + ((pDest->getPositionRelationTo(this) == BEFORE) ? ">" : "<");
 else if (isGrandChildOf(pDest))
  return getWordPOS() + "/" + ((getPositionRelationTo(parent) == BEFORE) ? ">" : "<") + 
         parent->getWordPOS() + "/"+ ((parent->getPositionRelationTo(pDest) == BEFORE) ? ">" : "<");
 else if (isGrandParentOf(pDest))
  return pDest->parent->getWordPOS() + "\\" + ((pDest->parent->getPositionRelationTo(parent) == BEFORE) ? ">" : "<") + 
         pDest->getWordPOS() + "\\" + ((pDest->getPositionRelationTo(this) == BEFORE) ? ">" : "<");
 else if (isSibling(pDest))
  return getWordPOS() + "/"  + ((getPositionRelationTo(parent) == BEFORE) ? ">" : "<") + 
         ((parent->getPositionRelationTo(pDest) == BEFORE) ? ">" : "<") + "\\" +
         pDest->getWordPOS();
 
 vector<DTNode> vcSrcToRoot, vcDestToRoot;
 

 vcSrcToRoot.push_back(this);
 
 // climbing up from the source node (this)
 do 
 {
  if (vcSrcToRoot.back()->parent == pDest)
   return getAPOSString(vcSrcToRoot, true, "/");
  else
   vcSrcToRoot.push_back(vcSrcToRoot.back()->parent);
    
  } while (!vcSrcToRoot.back()->isRoot());
  

 vcDestToRoot.push_back(pDest);

 // climbing up from the destination node
 do 
 {
  if (vcDestToRoot.back()->parent == this)
   return getAPOSString(vcDestToRoot, false, "\\");
  else
   vcDestToRoot.push_back(vcDestToRoot.back()->parent);
    
  } while (!vcDestToRoot.back()->isRoot());
  
 // climbing down from the root, while comparing two stacks (vectors) for
 // the first uncommon node, the node before which is the least common ancestor
 
 while (vcSrcToRoot.back() == vcDestToRoot.back())
 {
  vcSrcToRoot.pop_back();
  vcDestToRoot.pop_back();
  }	 
  
 return getAPOSString(vcSrcToRoot, true, "/") + getAPOSString(vcDestToRoot, false, "\\");
}

string DTree::getAPOSString(vector<DTNode> &pNodes, bool pDirection, string pUpDownDir)
{
 string vPOSString;

 // if direction is true, treat the vector as queue, otherwise as stack
 if (pDirection)
 {
  for (vector<DTNode>::iterator itNode = pNodes.begin(); 
       itNode < pNodes.end(); 
       itNode++)
   vPOSString += (*itNode)->getWordPOS() + pUpDownDir +
                 (((*itNode)->getPositionRelationTo((*itNode)->parent) == BEFORE) ? ">" : "<");
  }
 else
 {
  for (vector<DTNode>::reverse_iterator ritNode = pNodes.rbegin(); 
       ritNode < pNodes.rend(); 
       ritNode++)
   vPOSString += (((*ritNode)->getPositionRelationTo((*ritNode)->parent) == BEFORE) ? ">" : "<") + 
                 pUpDownDir + (*ritNode)->getWordPOS();
  } 

 return vPOSString;
}


/**
 *  Refer to getRelPathTo() comments
 */
string DTree::getLemmaPathTo(DTNode const &pDest)
{
 if (parent == pDest)
  return getWordLemma() + "/";
 else if (this == pDest->parent)
  return pDest->getWordLemma() + "\\";
 else if (isGrandChildOf(pDest))
  return getWordLemma() + "/" + parent->getWordLemma() + "/";
 else if (isGrandParentOf(pDest))
  return pDest->parent->getWordLemma() + "\\" + pDest->getWordLemma() + "\\";
 else if (isSibling(pDest))
  return getWordLemma() + "/\\" + pDest->getWordLemma();
 
 vector<DTNode> vcSrcToRoot, vcDestToRoot;
 

 vcSrcToRoot.push_back(this);
 
 // climbing up from the source node (this)
 while (!vcSrcToRoot.back()->isRoot()) 
 {
  if (vcSrcToRoot.back()->parent == pDest)
   return getLemmaString(vcSrcToRoot, true, "/");
  else
   vcSrcToRoot.push_back(vcSrcToRoot.back()->parent);
    
  }
  

 vcDestToRoot.push_back(pDest);

 // climbing up from the destination node
 while (!vcDestToRoot.back()->isRoot()) 
 {
  if (vcDestToRoot.back()->parent == this)
   return getLemmaString(vcDestToRoot, false, "\\");
  else
   vcDestToRoot.push_back(vcDestToRoot.back()->parent);
    
  }
  
 // climbing down from the root, while comparing two stacks (vectors) for
 // the first uncommon node, the node before which is the least common ancestor
 
 while (vcSrcToRoot.back() == vcDestToRoot.back())
 {
  vcSrcToRoot.pop_back();
  vcDestToRoot.pop_back();
  }	 
  
 return getLemmaString(vcSrcToRoot, true, "/") + getLemmaString(vcDestToRoot, false, "\\");
}

string DTree::getLemmaString(vector<DTNode> &pNodes, bool pDirection, string pUpDownDir)
{
 string vLemmaString;

 // if direction is true, treat the vector as queue, otherwise as stack
 if (pDirection)
 {
  for (vector<DTNode>::iterator itNode = pNodes.begin(); 
       itNode < pNodes.end(); 
       itNode++)
   vLemmaString += (*itNode)->getWordLemma() + pUpDownDir;
  }
 else
 {
  for (vector<DTNode>::reverse_iterator ritNode = pNodes.rbegin(); 
       ritNode < pNodes.rend(); 
       ritNode++)
   vLemmaString += pUpDownDir + (*ritNode)->getWordLemma();
  } 

 return vLemmaString;
}

unsigned short DTree::getPathLengthToLCA(DTNode &pDestNode)
{
 DTNode vLCANode = getLCAncestorWith(pDestNode);
 
 if (this == vLCANode)
  return 0;
 else
 { 
  string vPath = getRelPathTo(vLCANode);
 
  vector<string> tmpvcPath;
  tokenize(vPath, tmpvcPath, "/><\\");
  return tmpvcPath.size(); 
  }
}

unsigned short DTree::getFamRelTo(DTNode const &pDest)
{
 if (parent == pDest)
  return CHILD;
 else if (this == pDest->parent)
  return PARENT;
 else if (isGrandChildOf(pDest))
  return GRANDCHILD;
 else if (isGrandParentOf(pDest))
  return GRANDPARENT;
 else if (isSibling(pDest))
  return SIBLING;
 else if (isCousin(pDest))
  return COUSIN;
 else if (isDescendantOf(pDest))
  return DESCENDANT;
 else if (pDest->isDescendantOf(this))
  return ANCESTOR;
 else
  return 0; 
}


string DTree::getChldRelPattern()
{
 vector<DTNode> vChildren;	
 getChildren(vChildren);
 
 string vRelPattern;
	
 for (vector<DTNode>::iterator itChild = vChildren.begin(); 
      itChild < vChildren.end(); 
      itChild++)
  vRelPattern += (*itChild)->getDepRel() + "-";
  
 return (vRelPattern.substr(0, (vRelPattern.size() - 1)));
}

string DTree::getChldPOSPattern()
{
 vector<DTNode> vChildren;	
 getChildren(vChildren);
 
 string vPOSPattern;
	
 for (vector<DTNode>::iterator itChild = vChildren.begin(); 
      itChild < vChildren.end(); 
      itChild++)
  vPOSPattern += (*itChild)->getWordPOS() + "-";
  
 return (vPOSPattern.substr(0, (vPOSPattern.size() - 1)));
}

string DTree::getSibRelPattern()
{
 vector<DTNode> vSiblings;	
 getSiblings(vSiblings);
 
 string vRelPattern;
	
 for (vector<DTNode>::iterator itSibling = vSiblings.begin(); 
      itSibling < vSiblings.end(); 
      itSibling++)
  vRelPattern += (*itSibling)->getDepRel() + "-";
  
 return (vRelPattern.substr(0, (vRelPattern.size() - 1)));
}

string DTree::getSibPOSPattern()
{
 vector<DTNode> vSiblings;	
 getSiblings(vSiblings);
 
 string vPOSPattern;
	
 for (vector<DTNode>::iterator itSibling = vSiblings.begin(); 
      itSibling < vSiblings.end(); 
      itSibling++)
  vPOSPattern += (*itSibling)->getWordPOS() + "-";
  
 return (vPOSPattern.substr(0, (vPOSPattern.size() - 1)));
}

