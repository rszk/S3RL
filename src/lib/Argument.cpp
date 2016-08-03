/**
 *  @ RSK
 *  @ Version 0.1        20.11.2009
 * 
 *  (refer to Argumnet.hpp)
 * 
 */

#include <string>

#include "Argument.hpp"
#include "Sentence.hpp"
#include "TBTree.hpp"
#include "DataAnalysis.hpp"


Argument::Argument(Proposition * const & pProp, 
                   const WordIdx &pStart, 
                   const ArgLabel &pLabel)
{
 prop = pProp;
 wordSpan.start = pStart;
 wordSpan.end = 0;
 label = pLabel;
 argCnstnt = NULL;
 }

Proposition *Argument::getProp()
{
 return prop;
}

void Argument::setSpanEnd(const WordIdx &pEnd)
{
 wordSpan.end = pEnd;
}

/**
 *  Assigns the constituent passed to it to the argument, but 1st checks
 *  if the argument has already been assigned by a constituent
 *  (may occur when the privious constituent is the child of current
 *  constituent and their span is exectly aligned!), if so, it removes
 *  this argument from the arguments vector of the previous constituent
 *  and replace the constituent of argument with the new one. (parent is
 *  prefered)
 */
void Argument::setArgCnstnt(const Constituent &pCnstnt)
{
 if (argCnstnt)
  argCnstnt->deleteArg(this);
 argCnstnt = pCnstnt;
}

WordSpan Argument::getSpan()
{
 return wordSpan;
}

short Argument::getLength()
{
 return wordSpan.end - wordSpan.start + 1;
}

ArgLabel Argument::getLabel()
{
 return label;
}

string Argument::getWordsStr()
{
 return prop->getSentence()->getWordsStrInSpan(wordSpan);
}

Constituent Argument::getArgCnstnt()
{
 return argCnstnt;
}

bool Argument::hasCnstnt(Constituent const &pCnstnt)
{
 return (argCnstnt == pCnstnt);
}

/**
 *  analyze argument for checking consistency of loading and also
 *  extracting statistics according to DataAnalysis struture
 */
void Argument::analyzeData(DataAnalysis *&pDataAnalysis)
{
 // If the argument points to a constituent, check that the constituent
 // also points to it to confirm the loading consistency.
 // Otherwise, there is a mismach, so examine whether argument is assigned
 // to leaf node or there is no match (due to inconsistency between
 // PropBank and automatic parses)
 if (Constituent vArgCnstnt = getArgCnstnt())
 {
  if (!(vArgCnstnt->hasArgument(this)))
   pDataAnalysis->addArgNoPointBack(this);

  if (vArgCnstnt->isTerminal())
   pDataAnalysis->addArgOnLeaf(this);
  }
 else
  pDataAnalysis->addArgNoMatch(this);
 
 pDataAnalysis->addArgAnalyzed(); 
}   
