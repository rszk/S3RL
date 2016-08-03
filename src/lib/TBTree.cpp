/**
 *  @ RSK
 *  @ Version 0.1        24.11.2009
 * 
 *  (refer to TBTree.hpp)
 * 
 */

#include <iostream>
#include <deque>

#include "Sentence.hpp"
#include "TBTree.hpp"
#include "DTree.hpp"
#include "Argument.hpp"
#include "DataAnalysis.hpp"
#include "Utils.hpp"
#include "Constants.hpp"


/**
 * 
 *  TBTree class definitions
 * 
 */

TBTree::TBTree(Sentence &pSentence, 
               const string &pSyntTag, 
               const WordIdx &pFirstWord)
{
 sentence = &pSentence;
 syntTag = pSyntTag;
 parent = NULL;
 headChild = NULL;
 contentChild = NULL;
 depArgWord = NULL;
 wordSpan.start = pFirstWord;
 wordSpan.end = 0;
}

TBTree::TBTree(Sentence &pSentence, TBTree &pTree, 
               const string &pSyntTag, const WordIdx &pFirstWord)
{
 sentence = &pSentence;
 parent = &pTree;
 headChild = NULL;
 contentChild = NULL;
 depArgWord = NULL;
 // setting this node as a child of parent
 parent->children.push_back(this);
 syntTag = pSyntTag;
 wordSpan.start = pFirstWord;
 wordSpan.end = 0;
}

TBTree *TBTree::getParent()
{
 return parent;
}

bool TBTree::isRoot()
{
 return (parent == NULL);
}

bool TBTree::isTerminal()
{
 return (children.size() == 0);
}

/**
 *  This function has been adapted from Surdeanu et al. (2008) algorithm
 *  of is-Coordinated() which has been used for finding the head child 
 *  of a constituent. "Croosed by an apposition link" condition is
 *  ignored here.
 * 
 *  Note:
 *  This put the child ordinal in pConjOrdinal not the vector index
 */
bool TBTree::isCoordinated(short &pConjOrdinal)
{
 pConjOrdinal = 0;
 
 // skip the leftmost child by setting cntChild = 1
 for (size_t cntChild = 1; cntChild <= children.size() - 1; cntChild++)
 {
  if ((children[cntChild]->syntTag == "CC") ||
      (children[cntChild]->syntTag == "CONJP"))
  {
   pConjOrdinal = cntChild + 1;
   break;
   }
  
  // ignore the rightmost child for this two
  if (cntChild != (children.size() - 1))
   if ((children[cntChild]->syntTag == ",") ||
       (children[cntChild]->syntTag == ":"))
   {
    pConjOrdinal = cntChild + 1;
    break;
    }
  }
    
 return (pConjOrdinal != 0)	;
}

TBTree *TBTree::getChild(short &pChildOrdinal)
{
 return children[pChildOrdinal-1];	
} 

short TBTree::getChildOrdinal()
{
 // if it is root node, it is not child, so return 0
 if (!parent)
  return 0;
 
 // it will return ordinal not the index of vector
 short vOrdinal = 1;
 
 for (vector<TBTree *>::iterator itSibling = parent->children.begin(); 
      itSibling < parent->children.end(); 
      itSibling++, vOrdinal++)
  if ((*itSibling) == this)
   return vOrdinal;
 
 return -1;
}

short TBTree::getChildrenCount()
{
 return children.size();
}

/**
 *  To find the path from this node to destination node, it looks for the 
 *  least common ancesstor of the two nodes by climbing up from this node
 *  until the word span of the parent includes the destination node, 
 *  gathering the path string on the way. This node is considered as least
 *  common ancestor. The other half of the path is gathered by climbing up 
 *  to the common ancestor from destination node.
 */
string TBTree::getPathTo(TBTree * const &pDest)
{
 string vPath;
 vector<string> vHalfPath;
 TBTree *vLeastCommonAncestor;
 TBTree *vCurrNode = this;
 
 // climbing up from this node until least common ancestor, which is the
 // first parent node which its span includes the destination node, and 
 // collecting path string 
 do
 {
  if (!vCurrNode->isTerminal())
   vPath = vPath + vCurrNode->syntTag + '>';
  vCurrNode = vCurrNode->parent;
  }
 while ((getSpansRelation(vCurrNode->wordSpan, pDest->wordSpan) != INCLUDE));
 
 vPath = vPath + vCurrNode->syntTag;
 vLeastCommonAncestor = vCurrNode;
 
 // climbing up from destination node until least common ancestor, which
 // is found in previous loop, and collecting syntactic tags to then create
 // the other half of path
 vCurrNode = pDest;
 while (vCurrNode != vLeastCommonAncestor)
 {
  if (!vCurrNode->isTerminal())
   vHalfPath.push_back(vCurrNode->syntTag);
  vCurrNode = vCurrNode->parent;
  }
  
 for (short cntHalfPath = vHalfPath.size() - 1; 
      cntHalfPath >= 0; 
      cntHalfPath--)
  vPath = vPath + '<' + vHalfPath[cntHalfPath];
  
 return vPath;
}

WordPtr TBTree::getHeadWord()
{
 Constituent vHeadWord = getHeadWordCnstnt();
 
 if (vHeadWord)
  return sentence->getWord((vHeadWord->getWordSpan()).start); 
 else
  return NULL; 
}

string TBTree::getHeadWordForm()
{
 Constituent vHeadWord = getHeadWordCnstnt();
 
 if (vHeadWord)
  return vHeadWord->getWordsStr(); 
 else
  return ""; 
}

string TBTree::getHeadWordLemma()
{
 Constituent vHeadWord = getHeadWordCnstnt();
 
 if (vHeadWord)
  return vHeadWord->getWordsLemmaStr(); 
 else
  return ""; 
}

/**
 *  The head word of a phrase id the head word of its head child. This is
 *  used to recursively find the head word of this phrase which is found 
 *  when it reachs a terminal head child which is the head word of current
 *  phrase
 *  
 *  Note:
 *  This use getHeadChild() to access head child, because it may not be 
 *  set yet and getHeadChild() uses setHeadWord() to set it in that case.
 */
Constituent TBTree::getHeadWordCnstnt()
{
 Constituent vHeadChild = getHeadChild();
 
 if (vHeadChild)
 {
  if (vHeadChild->isTerminal())
   return vHeadChild;
  else 
   return vHeadChild->getHeadWordCnstnt();
  }
 else
  return NULL; 
}

/**
 *  This checks whether the headWord is set, and if not, calls setHeadChild()
 *  to set it.
 *  Setting headword when needed and storing it in class help to increase
 *  efficiency.
 */
Constituent TBTree::getHeadChild()
{
 if (headChild)
  return headChild; 
 else
  return setHeadChild();
}

/**
 *  If this is terminal, its head child is itself. Otherwise using head 
 *  word rules in Surdeanu et al. (2008) it finds, set, and returns the
 *  head child of this phrase.
 *  
 *  In Surdeanu et al. (2008), it first finds the first occurance of a 
 *  conjunctor child to process only from the first child to that child.
 *  But here, we observed that it very slightly (0.04 points) degraded 
 *  the overal F1 when using F1 as feature. So, it is currently disabled.
 *  
 *  Note:
 *  Due to inconsistency between Surdeanu et al. (2008) and Johansson &
 *  Negues (2007), where the latter is the reference of former, we adapted
 *  to variations of this rules for NAC and WHNP. The desired one can be 
 *  called from inside this function. However, using the former set of 
 *  rules gained 0.01 points better result.
 *  
 */
Constituent TBTree::setHeadChild()
{
 // the head child of terminal node is itself
 if (isTerminal())
 {
  headChild = this;
  return headChild;
  }
  
 short lastChild;
/* 
 Currently disabled!
 // If this is a coordinated phrase, convert the conjunction child ordinal
 // to vector index and set it as last child to be processed, otherwise 
 // set the last child to the leftmost child.
 if (isCoordinated(lastChild)) 
  lastChild--;
 else 
*/
 lastChild = children.size() - 1;
  
 
 // rule for ADJP (Adjective Phrase)
 if (syntTag == "ADJP")
 {
  string priority [] = {"NNS", "QP", "NN", "$", "ADVP", "JJ", "VBN", "VBG",
                        "ADJP", "JJR", "NP", "JJS", "DT", "FW", "RBR", 
                        "RBS", "SBAR", "RB", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for ADVP (Adverb Phrase)
 if (syntTag == "ADVP")
 {
  string priority [] = {"RB", "RBR", "RBS", "FW", "ADVP", "TO", "CD", 
                        "JJR", "JJ", "IN", "NP", "JJS", "NN", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = 0; cntChild <= lastChild; cntChild++)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }

 // rule for CONJP (Conjunction Phrase)
 if (syntTag == "CONJP")
 {
  string priority [] = {"CC", "RB", "IN", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = 0; cntChild <= lastChild; cntChild++)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }

 // rule for FRAG (Fragment)
 if (syntTag == "FRAG")
 {
  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if ((children[cntChild]->syntTag.substr(0,2) == "NN") ||
       (children[cntChild]->syntTag == "NP"))
   {
    headChild = children[cntChild];
    return headChild;
    }

  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if (children[cntChild]->syntTag[0] == 'W')
   {
    headChild = children[cntChild];
    return headChild;
    }

  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if (children[cntChild]->syntTag == "SBAR")
   {
    headChild = children[cntChild];
    return headChild;
    }

  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if ((children[cntChild]->syntTag == "PP") ||
       (children[cntChild]->syntTag == "IN"))
   {
    headChild = children[cntChild];
    return headChild;
    }

  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if ((children[cntChild]->syntTag == "ADJP") ||
       (children[cntChild]->syntTag == "JJ"))
   {
    headChild = children[cntChild];
    return headChild;
    }

  string priority [] = {"ADVP", "RB", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = 0; cntChild <= lastChild; cntChild++)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }

 // rule for INTJ (Interjection e.g. "say" in "If there is, say, 1000 of ...")
 if (syntTag == "INTJ")
 {
  headChild = children[lastChild];
  return headChild;
  }
 	
 // rule for LST (List Marker e.g. "1:" in "1: effects of parse error")
 if (syntTag == "LST")
 {
  string priority [] = {"LS", ":", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = 0; cntChild <= lastChild; cntChild++)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }

 // rule for NAC (Not a Constituent), WHNP (WH NP e.g. "who") using
 // Surdeanu et al. (2008) or Johansson & Negues (2007) version
 if ((syntTag == "NAC") || (syntTag == "WHNP"))
 {
  headChild = findCoNLLHeadOfNACWHNP(lastChild);
  //headChild = findJNHeadOfNACWHNP(lastChild);
  return headChild;
  }

 // rule for NP, NX (Incomplete NP)
 if ((syntTag == "NP") || (syntTag == "NX"))
 {
  for (short cntChild = lastChild; cntChild >= 0; cntChild--)
   if ((children[cntChild]->syntTag.substr(0,2) == "NN") ||
       (children[cntChild]->syntTag == "NX"))
   {
    headChild = children[cntChild];
    return headChild;
    }

  // NP-e: there is no funtion tag in CoNLL version of PropBank
  
  string priority [] = {"JJR", "CD", "JJ", "JJS", "RB", "QP", "NP", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for PP, WHPP (WH PP e.g. "of which")
 if ((syntTag == "PP") || (syntTag == "WHPP"))
 {
  string priority [] = {"IN", "TO", "VBG", "VBN", "RP", "FW", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = 0; cntChild <= lastChild; cntChild++)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }

 // rule for PRN (Parenthetical)
 if (syntTag == "PRN")
 {
  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if (children[cntChild]->syntTag[0] == 'S')
   {
    headChild = children[cntChild];
    return headChild;
    }

  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if (children[cntChild]->syntTag[0] == 'N')
   {
    headChild = children[cntChild];
    return headChild;
    }

  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if (children[cntChild]->syntTag[0] == 'W')
   {
    headChild = children[cntChild];
    return headChild;
    }

  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if ((children[cntChild]->syntTag == "PP") ||
       (children[cntChild]->syntTag == "IN"))
   {
    headChild = children[cntChild];
    return headChild;
    }

  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if ((children[cntChild]->syntTag == "ADJP") ||
       (children[cntChild]->syntTag.substr(0,2) == "JJ"))
   {
    headChild = children[cntChild];
    return headChild;
    }

  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if ((children[cntChild]->syntTag == "ADVP") ||
       (children[cntChild]->syntTag.substr(0,2) == "RB"))
   {
    headChild = children[cntChild];
    return headChild;
    }
  }
  
 // rule for PRT (Particle)
 if (syntTag == "PRT")
 {
  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if (children[cntChild]->syntTag == "RP")
   {
    headChild = children[cntChild];
    return headChild;
    }
  }
 
 // rule for QP (Quantifier Phrase)
 if (syntTag == "QP")
 {
  string priority [] = {"$", "IN", "NNS", "NN", "JJ", "RB", "DT", "CD",
                        "NCD", "QP", "JJR", "JJS", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for RRC (Reduced Relative Clause)
 if (syntTag == "RRC")
 {
  string priority [] = {"VP", "NP", "ADVP", "ADJP", "PP", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = 0; cntChild <= lastChild; cntChild++)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for S
 if (syntTag == "S")
 {
  for (short cntChild = lastChild; cntChild >= 0; cntChild--)
   if (children[cntChild]->syntTag == "VP")
   {
    headChild = children[cntChild];
    return headChild;
    }
  
  for (short cntChild = lastChild; cntChild >= 0; cntChild--)
   if (children[cntChild]->syntTag.find("-PRD") != string::npos)
   {
    headChild = children[cntChild];
    return headChild;
    }
  
  string priority [] = {"S", "SBAR", "ADJP", "UCP", "NP", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for SBAR
 if (syntTag == "SBAR")
 {
  string priority [] = {"S", "SQ", "SINV", "SBAR", "FRAG", "IN", "DT", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for SBARQ (Direct WH question)
 if (syntTag == "SBARQ")
 {
  string priority [] = {"SQ", "S", "SINV", "SBARQ", "FRAG", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for SINV (Inverted Declarative Sentence; i.e. the subject follows the tensed verb or modal)
 if (syntTag == "SINV")
 {
  string priority1 [] = {"VBZ", "VBD", "VBP", "VB", "MD", "VP", ""};
  for (short cntPriority = 0; !priority1[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority1[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  
  for (short cntChild = lastChild; cntChild >= 0; cntChild--)
   if (children[cntChild]->syntTag.find("-PRD") != string::npos)
   {
    headChild = children[cntChild];
    return headChild;
    }
  
  string priority2 [] = {"S", "SINV", "ADJP", "NP", ""};
  for (short cntPriority = 0; !priority2[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority2[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for SQ (Inverted yes/no question)
 if (syntTag == "SQ")
 {
  string priority1 [] = {"VBZ", "VBD", "VBP", "VB", "MD", ""};
  for (short cntPriority = 0; !priority1[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority1[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  
  for (short cntChild = lastChild; cntChild >= 0; cntChild--)
   if (children[cntChild]->syntTag.find("-PRD") != string::npos)
   {
    headChild = children[cntChild];
    return headChild;
    }
  
  string priority2 [] = {"VP", "SQ", ""};
  for (short cntPriority = 0; !priority2[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority2[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for UCP (Unlike Coordinated Phrase; e.g. "Coca-Cola and other sodas")
 if (syntTag == "UCP")
 {
  headChild = children[0];
  return headChild;
  }
 	
 // rule for VP 
 if (syntTag == "VP")
 {
  string priority1 [] = {"VBD", "VBN", "MD", "VBZ", "VB", "VBG", "VBP", 
                         "VP", ""};
  for (short cntPriority = 0; !priority1[cntPriority].empty(); cntPriority++)
   for (short cntChild = 0; cntChild <= lastChild; cntChild++)
    if (children[cntChild]->syntTag == priority1[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  
  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if (children[cntChild]->syntTag.find("-PRD") != string::npos)
   {
    headChild = children[cntChild];
    return headChild;
    }
  
  string priority2 [] = {"ADJP", "NN", "NNS", "NP", ""};
  for (short cntPriority = 0; !priority2[cntPriority].empty(); cntPriority++)
   for (short cntChild = 0; cntChild <= lastChild; cntChild++)
    if (children[cntChild]->syntTag == priority2[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for WHADJP
 if (syntTag == "WHADJP")
 {
  string priority [] = {"CC", "WRB", "JJ", "ADJP", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = lastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for WHADVP
 if (syntTag == "WHADVP")
 {
  string priority [] = {"CC", "WRB", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = 0; cntChild <= lastChild; cntChild++)
    if (children[cntChild]->syntTag == priority[cntPriority])
    {
     headChild = children[cntChild];
     return headChild;
     }
  }
  
 // rule for X (Unknown, uncertain, or unbracketable)
 if (syntTag == "X")
 {
  headChild = children[0];
  return headChild;
  }
 	
 // the first word is considered the default head if rules were not fired
 return children[0]; 	 
}

Constituent TBTree::findCoNLLHeadOfNACWHNP(short &pLastChild)
{
 if ((syntTag == "NAC") || (syntTag == "WHNP"))
 {
  for (short cntChild = pLastChild; cntChild >= 0; cntChild--)
   if ((children[cntChild]->syntTag.substr(0,2) == "NN") ||
       (children[cntChild]->syntTag == "NX"))
    return children[cntChild];

  // NP-e: there is no funtion tag in CoNLL version of PropBank
  
  string priority [] = {"JJR", "CD", "JJ", "JJS", "RB", "QP", "NP", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = pLastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority[cntPriority])
     return children[cntChild];
  }

 // the first word is considered the default head if rules were not fired
 return children[0]; 	 
}

Constituent TBTree::findJNHeadOfNACWHNP(short &pLastChild)
{
 if (syntTag == "NAC")
 {
  for (short cntChild = pLastChild; cntChild >= 0; cntChild--)
   if (children[cntChild]->syntTag.substr(0,2) == "NN")
    return children[cntChild];

  string priority [] = {"NP", "NAC", "EX", "$", "CD", "QP", "PRP", "VBG",
                        "JJ", "JJS", "JJR", "ADJP", "FW", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = pLastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority[cntPriority])
     return children[cntChild];
  }

 if (syntTag == "WHNP")
 {
  for (short cntChild = pLastChild; cntChild >= 0; cntChild--)
   if (children[cntChild]->syntTag.substr(0,2) == "NN")
    return children[cntChild];

  string priority [] = {"WDT", "WP", "WD$", "WHADJP", "WHPP", "WHNP", ""};
  for (short cntPriority = 0; !priority[cntPriority].empty(); cntPriority++)
   for (short cntChild = pLastChild; cntChild >= 0; cntChild--)
    if (children[cntChild]->syntTag == priority[cntPriority])
     return children[cntChild];
  }

 // the first word is considered the default head if rules were not fired
 return children[0]; 	 
}

string TBTree::getContentWordForm()
{
 Constituent vContentWord = getContentWord();
 
 if (vContentWord)
  return vContentWord->getWordsStr(); 
 else
  return ""; 
}

string TBTree::getContentWordLemma()
{
 Constituent vContentWord = getContentWord();
 
 if (vContentWord)
  return vContentWord->getWordsLemmaStr(); 
 else
  return ""; 
}

/**
 *  The content word of a phrase is the content word of its content child.
 *  This is used to recursively find the content word of this phrase which
 *  is found when it reaches a terminal content child which is the content
 *  word of current phrase
 *  
 *  Note:
 *  This uses getContentChild() to access content child, because it may 
 *  not be set yet and getContentChild() uses setContentWord() to set it
 *  in that case.
 */
Constituent TBTree::getContentWord()
{
 Constituent vContentChild = getContentChild();

 if (vContentChild)
 {
  if (vContentChild->isTerminal())
   return vContentChild;
  else 
   return vContentChild->getContentWord();
  }
 else
  return NULL; 
}

/**
 *  This checks whether the content Word is set, and if not, calls 
 *  setContentChild() to set it.
 *  Setting content word when needed and storing it in class help to 
 *  increase efficiency.
 */
Constituent TBTree::getContentChild()
{
 if (contentChild)
  return contentChild; 
 else
  return setContentChild();
}

/**
 *  If this is terminal, its content child is itself. Otherwise using 
 *  content word heuristics in Surdeanu et al. (2003) it finds, set, and
 *  returns the content child of this phrase.
 */
Constituent TBTree::setContentChild()
{
 // the content child of terminal node is itself
 if (isTerminal())
 {
  contentChild = this;
  return contentChild;
  }
  
 short lastChild;
 lastChild = children.size() - 1;
  
 // H1: heuristic for PP
 if (syntTag == "PP")
 {
  contentChild = children[lastChild];
  return contentChild;
  }
  
 // H2: heuristic for SBAR
 if (syntTag == "SBAR")
 {
  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if (children[cntChild]->syntTag[0] == 'S')
   {
    contentChild = children[cntChild];
    return contentChild;
    }
  }

 // H3: heuristic for VP
 if (syntTag == "VP")
 {
  for (short cntChild = 0; cntChild <= lastChild; cntChild++)
   if (children[cntChild]->syntTag == "VP")
   {
    contentChild = children[cntChild];
    return contentChild;
    }
   
  contentChild = getHeadChild();
  return contentChild;
  }

 // H4: heuristic for ADVP
 if (syntTag == "ADVP")
 {
  for (short cntChild = lastChild; cntChild >= 0; cntChild--)
   if ((children[cntChild]->syntTag != "IN") && 
       (children[cntChild]->syntTag != "TO"))
   {
    contentChild = children[cntChild];
    return contentChild;
    }
  }
   
 // H5: heuristic for ADJP
 if (syntTag == "ADJP")
 {
  for (short cntChild = lastChild; cntChild >= 0; cntChild--)
   if ((children[cntChild]->syntTag.substr(0, 2) == "JJ") ||
       (children[cntChild]->syntTag[0] == 'V') ||
       (children[cntChild]->syntTag[0] == 'N') ||
       (children[cntChild]->syntTag == "ADJP"))
   {
    contentChild = children[cntChild];
    return contentChild;
    }
  }

 // for all other phrase types, the content word is identical to head word
 return getHeadChild();
}

string TBTree::getSubcat()
{
 if (!parent)
  return "";
  
 string vSubcat = parent->getSyntTag() + "->";
 
 for (vector<TBTree *>::iterator itChild = parent->children.begin(); 
      itChild < parent->children.end(); 
      itChild++)
    vSubcat += (*itChild)->getSyntTag() + "-";

 return (vSubcat.substr(0, (vSubcat.size() - 1)));
}

string TBTree::getGovCat()
{
 TBTree *vParent = parent;
 while (vParent)
  if ((vParent->syntTag[0] == 'S') ||
      (vParent->syntTag == "VP"))
   return vParent->syntTag;
  else
   vParent = vParent->parent;
   
 return "";   
}

string TBTree::getSyntTag()
{
 return syntTag;
}

void TBTree::setLastWord(const WordIdx &pWord)
{
 wordSpan.end = pWord;
}

WordSpan  TBTree::getWordSpan()
{
 return wordSpan;
}

string TBTree::getWordsStr()
{
 return sentence->getWordsStrInSpan(wordSpan);
}

string TBTree::getWordsLemmaStr()
{
 return sentence->getWordsLemmaStrInSpan(wordSpan);
}

unsigned short TBTree::getWordsCount()
{
 return (wordSpan.end - wordSpan.start + 1);
}

/**
 *  This checks whether the dependency argument word is set, and if not, calls setHeadChild()
 *  to set it.
 *  Setting headword when needed and storing it in class help to increase
 *  efficiency.
 */
WordPtr TBTree::getDepArgWord()
{
 if (depArgWord)
  return depArgWord; 
 else
  return setDepArgWord();
}


/**
 *  Dependency argument word is the argument in dependency-based SRL. Here
 *  it is derived from constituent-based arguments and samples using the 
 *  rules used in Surdeanu et al. (2008) in preparation of ConLL 2008 data:
 *  the dependency argument is assigned to a word inside the argument 
 *  constituent whose head is outside the constituent boundary.
 *  See, the boddy comments for more exception handlings.
 */
WordPtr TBTree::setDepArgWord()
{
 // if the constituent is terminal node, the arg word is the only word
 // in the constituent span
 if (isTerminal())
 {
  depArgWord = sentence->getWord(wordSpan.start);
  return depArgWord;
  }
 
 vector<WordPtr> vcDAWCandidates;
 
 // looping for all words inside the constituent boundary
 for (unsigned short cntWord = wordSpan.start; 
      cntWord <= wordSpan.end;
      cntWord++)
 {
  WordIdx vHeadIdx = (sentence->getWord(cntWord))->getDepHeadWordIdx();

  // collecting words whose head is outside the boundary
  // there may be more than 1 word, so they are collected in a vector
  // to resolve the conflict later
  if ((vHeadIdx < wordSpan.start) || (vHeadIdx > wordSpan.end))
   vcDAWCandidates.push_back(sentence->getWord(cntWord));
  }	     
  
 // We anticipate situation that there was no such word (may be because 
 // dependency parsing errors). So we assign the head word as argument word.
 if (vcDAWCandidates.size() == 0)
 {
  /////////////
  ////sentence->display(cout);
  ////cout << "No arg word for: " << getWordsStr() << endl;
  
  depArgWord = getHeadWord();
  }
 // For the constituents that more than 1 word with their head outside 
 // the boundary, which is specially the case with the result of Malt parser, 
 // CoNLL 2008 has separated the argument into two X/C-X arguments, one
 // for each of those words.
 // We cannot do that way, because we should keep consistency between
 // CB and DB systems. So, we simply used the head word of the constituent 
 // for breaking this tie.
 else if (vcDAWCandidates.size() > 1)
 {
  /////////////
  ////cout << endl;
  ////sentence->display(cout);
  ////cout << vcDAWCandidates.size() << " arg words for: " << getWordsStr() << endl;
  ////for (int i = 0; i < vcDAWCandidates.size(); i++)
  //// cout << vcDAWCandidates[i]->getWordForm() << endl;
  
  depArgWord = getHeadWord();
  }
 else 
  depArgWord = vcDAWCandidates[0];
 
 return depArgWord;
}

/**
 *  looking for an argument that aligns the node's span and, if any, 
 *  adding it to the list of arguments of this constituent and assigning
 *  itself to the argument too
 */
void TBTree::setProbableArgs()
{
 sentence->findPropArgsInSpan(args, wordSpan);
 for (vector<Argument *>::iterator it = args.begin(); 
      it < args.end(); 
      it++)
 {
   (*it)->setArgCnstnt(this);
  }
}

void TBTree::deleteArg(Argument * const &pArg)
{
 for (vector<Argument *>::iterator it = args.begin(); 
      it < args.end(); 
      it++)
 {
   if ((*it) == pArg)
    args.erase(it);
  }
}

void TBTree::assignToPropPred(const short &pPropNo)
{
 sentence->assignToPropPred(pPropNo, this);
}

bool TBTree::hasArgument(Argument * const &pArg)
{
 for (vector<Argument *>::iterator it = args.begin(); 
      it < args.end(); 
      it++)
  if ((*it) == pArg) 
   return true;
 
 return false;
}

/**
 *  iterates Treebank tree constituents to analyze them for checking
 *  consistency of loading and also extracting statistics according 
 *  to DataAnalysis struture
 */
void TBTree::analyzeData(DataAnalysis *&pDataAnalysis)
{
 // analyzes itself if it is not the topmost node
 if (!isRoot())
 {
  for (vector<Argument *>::iterator itArg = args.begin(); 
       itArg < args.end(); 
       itArg++)
   if (!((*itArg)->hasCnstnt(this)))
    pDataAnalysis->addCnstntNoPointBack(this);

 pDataAnalysis->addCnttntAnalyzed(); 
 }
 
 // now analyzes children (depth-first)
 for (vector<TBTree *>::iterator itChild = children.begin(); 
      itChild< children.end(); 
      itChild++)
   (*itChild)->analyzeData(pDataAnalysis);   

}

void TBTree::collectChildren(vector<Constituent> &pChildren)
{
 for (vector<TBTree *>::iterator itChild = children.begin(); 
      itChild < children.end(); 
      itChild++)
  pChildren.push_back(*itChild);
}

/**
 *  starts from a node recursively iterates parents of the node until reachs
 *  root node and at each iteration collects the sibilings of the current
 *  node plus their immediate children if the current node is PP
 * 
 *  It ignores all siblings in coordination with current node (siblings 
 *  on the other side of a CC sister). To accomplish this, it treats
 *  left and right siblings separetly, which is also good for analyctical 
 *  purposes
 * 
 *  We ignore all single constituent punctuations.
 *  
 */
void TBTree::collectXPCnstnts(vector<Constituent> &pCnstnt)
{
 if (parent)
 {
  short childOrdinal = getChildOrdinal();
  short vLastCoord;
  
  vLastCoord = childOrdinal-1;
  // decending iteration of left siblings (to make possible the look for 
  // coordinations)
  for (short cntLSibling = childOrdinal - 2; 
       cntLSibling >= 0; 
       cntLSibling--)
  { 
   // If this is "CC", look at the previous siblinig, if it was in coordination
   // with this node (or the node itself), then the next sibling will 
   // also in coordination  with this node; ignore this and the next node
   // and continue.
   if (parent->children[cntLSibling]->syntTag == "CC")
    if ((cntLSibling + 1) == vLastCoord)
    {
     vLastCoord = cntLSibling - 1;
     cntLSibling -= 2;
     continue;
     }   
  
   if (parent->children[cntLSibling]->syntTag == "PP")
    parent->children[cntLSibling]->collectChildren(pCnstnt);
   
   // ignore single constituent punctuation
   if ((parent->children[cntLSibling]->syntTag == "(")||
       (parent->children[cntLSibling]->syntTag == ")")||
       (parent->children[cntLSibling]->syntTag == ",")||
       (parent->children[cntLSibling]->syntTag == ":")||
       (parent->children[cntLSibling]->syntTag == "``")||
       (parent->children[cntLSibling]->syntTag == "''")||
       (parent->children[cntLSibling]->syntTag == "."))           
    continue;
    
   pCnstnt.push_back(parent->children[cntLSibling]);  
   }
  
  vLastCoord = childOrdinal-1;
  // decending iteration of right siblings (to make possible the look for 
  // coordinations)
  for (unsigned short cntLSibling = childOrdinal; 
       cntLSibling < parent->children.size(); 
       cntLSibling++)
  { 
   // If this is "CC", look at the previous siblinig, if it was in coordination
   // with this node (or the node itself), then the next sibling will 
   // also in coordination  with this node; ignore this and the next node
   // and continue.
   if (parent->children[cntLSibling]->syntTag == "CC")
    if ((cntLSibling - 1) == vLastCoord)
    {
     vLastCoord = cntLSibling + 1;
     cntLSibling += 2;
     continue;
     }   

   if (parent->children[cntLSibling]->syntTag == "PP")
    parent->children[cntLSibling]->collectChildren(pCnstnt);
   
   // ignore single constituent punctuation
   if ((parent->children[cntLSibling]->syntTag == "(")||
       (parent->children[cntLSibling]->syntTag == ")")||
       (parent->children[cntLSibling]->syntTag == ",")||
       (parent->children[cntLSibling]->syntTag == ":")||
       (parent->children[cntLSibling]->syntTag == "``")||
       (parent->children[cntLSibling]->syntTag == "''")||
       (parent->children[cntLSibling]->syntTag == "."))           
    continue;
    
   pCnstnt.push_back(parent->children[cntLSibling]);  
   }
    
  parent->collectXPCnstnts(pCnstnt);
  }
}

/**
 *  priniting a tree with each level in new line
 */
struct Node_Level
{
 TBTree * node;
 short level;
 };

void TBTree::displayTree()
{
 deque<struct Node_Level> qNodes;
 Node_Level vCurrNL;
 short lastLevel = 0;

 if (!this)
  return;

 Node_Level vNL;
 vNL.node = this;
 vNL.level = 1;
 
 qNodes.push_back(vNL);
 while (!qNodes.empty())
 {
  vCurrNL = qNodes.front();
  qNodes.pop_front();
  if (vCurrNL.level > lastLevel)
  {
   cout << endl;
   lastLevel++;
   }
  
  cout << vCurrNL.node->syntTag;
  
  for (vector<Argument *>::iterator it = vCurrNL.node->args.begin(); 
       it < vCurrNL.node->args.end(); 
       it++)
  {
   cout << "[" <<  (*it)->getLabel() << "]";
   };  
    
  cout << "[" << vCurrNL.node->wordSpan.start 
       << "-" << vCurrNL.node->wordSpan.end 
       << "] ; ";

  for (vector<TBTree *>::iterator it = vCurrNL.node->children.begin(); 
       it < vCurrNL.node->children.end(); 
       it++)
  {
   vNL.node = *it;
   vNL.level = vCurrNL.level + 1;
   qNodes.push_back(vNL);
   }
  }
 qNodes.clear();
 cout << endl;
}

