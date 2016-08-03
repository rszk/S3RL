/**
 *  @ RSK
 *  @ Version 1.1        03.03.2010
 * 
 *  (refer to Sentence.hpp)
 * 
 */

#include <iostream>
#include <fstream>

#include "Sentence.hpp"
#include "Types.hpp"
#include "TBTree.hpp"
#include "DTree.hpp"
#include "Argument.hpp"
#include "DataAnalysis.hpp"
#include "SampleGenerator.hpp"
#include "Sample.hpp"
#include "Utils.hpp"


/**
 * 
 *  Sentence class definitions
 * 
 */

Sentence::Sentence()
{
 tbTree = NULL;
 
 // when a sentence is created, a DTree is immediately created and assigned 
 // to it (unlike TBTree), which is in fact the root node of the tree
 dTree = new DTree (*this);
 }

int Sentence::appendWord(Word * const &pWord)
{
 try
 {
  words.push_back(pWord);
  return words.size();
  }
 catch (bad_alloc &)
 {  
  cerr << "Memory allocation failed for sentence word!" << endl;
  return -1;
  }
}

WordPtr Sentence::getWord(WordIdx pWordIdx)
{
 return words[pWordIdx - 1];
}

string Sentence::getWordForm(WordIdx pWord)
{
 return words.at(pWord-1)->getWordForm();
}

WordIdx Sentence::getWordIdx(WordPtr pWord)
{
 for (vector<WordPtr>::iterator it = words.begin(); 
      it < words.end(); 
      it++)
  if (*it == pWord)
   return (it - words.begin() + 1);
 
 // this happens when the word is NULL (e.g. when the index of the root
 // node in a dependency tree (where the word member is NULL) is inquired
 return 0;
} 

string Sentence::getWordsStrInSpan(WordSpan &pSpan)
{
 string vStr = "";
 for (short i = pSpan.start-1 ; i < pSpan.end ; i++)
 {
  vStr = vStr + " " + words.at(i)->getWordForm();
  }
  
 return trim(vStr); 
}
  
void Sentence::setWordCnstnt(WordIdx pWord, Constituent &pCnstnt)
{
 words.at(pWord - 1)->setCnstnt(pCnstnt);
}

Constituent Sentence::getWordCnstnt(WordIdx pWord)
{
 return words.at(pWord - 1)->getCnstnt();
}

string Sentence::getWordsLemmaStrInSpan(WordSpan &pSpan)
{
 string vStr = "";
 for (short i = pSpan.start-1 ; i < pSpan.end ; i++)
 {
  vStr = vStr + " " + words.at(i)->getWordLemma();
  }
  
 return trim(vStr); 
}
  
void Sentence::clearWords()
{
 words.clear();
 }

short Sentence::getLength()
{
 return words.size();
 }

void Sentence::display(ostream &pStream)
{
 for (vector<Word *>::iterator it = words.begin(); 
      it < words.end(); 
      it++)
 {
  pStream << (*it)->getWordForm() << " ";
  }
 pStream << endl;
}

DTNode Sentence::getDTree()
{
 return dTree;
}

/**
 *  setting heads of dependency tree of the senetence
 */
void Sentence::setDTreeHeads(vector<WordIdx> &pHeadIdxs)
{
 for (vector<WordPtr>::iterator it = words.begin(); 
      it < words.end(); 
      it++)
 { 
  DTNode vParentNode;
  
  // if the head is root node, 
  if (pHeadIdxs[it - words.begin()] == 0)
   vParentNode = dTree;
  else
   vParentNode = words[pHeadIdxs[it - words.begin()] - 1]->getDTNode();

  (*it)->getDTNode()->setParent(vParentNode);
  }
}

void Sentence::displayDTree(ostream &pStream)
{
/////
/* for (vector<WordPtr>::iterator it = words.begin(); 
      it < words.end(); 
      it++)
 {
  pStream << (*it)->getDTNode()->getDepRel() 
          << "[" << (*it)->get->getHeadForm() << "]" << " ";
  }
 pStream << endl;
*/
}

void Sentence::setTBTree(TBTree * const  & pTBTree)
{
 tbTree = pTBTree;
}

TBTree* Sentence::getTBTree()
{
 return tbTree;
}

void Sentence::displayTree()
{
 tbTree->displayTree();
}

void Sentence::appendProp(const unsigned short &propCount)
{
 for (unsigned int i = 0; i < propCount; i++)
  try
  {
   props.push_back(new Proposition(this)) ;
   }
  catch (bad_alloc &)
  {  
   cerr << "Memory allocation failed for proposition!" << endl;
   }
}

void Sentence::getProps(vector<Proposition *> &pProps)
{
 pProps = props;
}

short Sentence::getPropCount()
{
 return props.size();
}

void Sentence::setPropPredWord(const short &pPropNo, const WordIdx &pWord)
{
 props[pPropNo-1]->setPredWord(pWord);
}

/**
 *  iterates propositions of the senetnce to gather the predicate word 
 *  index of each of them to compose a vector as a template
 * 
 *  Each vector is in the length of the sentence and the lemma of each
 *  predicate id put exaclty in the element that corresponds to its
 *  index in the sentence. other elements are left empty.
 */
void Sentence::getPredTemplate(vector<string> &pPredTemplate)
{
 pPredTemplate.resize(words.size());
 
 for (vector<Proposition *>::iterator itProp = props.begin();
      itProp < props.end(); 
      itProp++)
  pPredTemplate.at((*itProp)->getPredWordIdx()-1) = (*itProp)->getPredLemma();
}

void Sentence::setPropPredSpanStart(const short &pPropNo, const WordIdx &pStart)
{
 props[pPropNo-1]->setPredSpanStart(pStart);
}

void Sentence::setPropPredSpanEnd(const short &pPropNo, const WordIdx &pEnd)
{
 props[pPropNo-1]->setPredSpanEnd(pEnd);
}

void Sentence::setPropPredCSpanStart(const short &pPropNo, const WordIdx &pStart)
{
 props[pPropNo-1]->setPredCSpanStart(pStart);
}

void Sentence::setPropPredCSpanEnd(const short &pPropNo, const WordIdx &pEnd)
{
 props[pPropNo-1]->setPredCSpanEnd(pEnd);
}

/**
 *  iterating propositions to assign the passed constituent to the probable
 *  predicate continuation (C-V) of the proposition that aligns the span
 *  of the constituent 
 */
void Sentence::setProbablePropPredCCnstnt(Constituent &pCnstnt)
{
 for (vector<Proposition *>::iterator itProp = props.begin();
      itProp < props.end(); 
      itProp++)
  (*itProp)->setProbablePredCCnstnt(pCnstnt);
}

void Sentence::setPropPredLemma(const short &pPropNo, const string &pLemma)
{
 props[pPropNo-1]->setPredLemma(pLemma);
}

string Sentence::getPropPredLemma(const short &pPropNo)
{
 return props[pPropNo-1]->getPredLemma();
}

/**
 *  processes all propositions of the sentence to figure out whether their
 *  predicates is simple, compound or discontinues compound, and then 
 *  changes span end (if needed) and sets the continuation constituent 
 *  and its span too
 */
void Sentence::estimatePropPredBoundaries()
{
 for (vector<Proposition *>::iterator itProp = props.begin();
      itProp < props.end(); 
      itProp++)
  (*itProp)->estimatePredBoundaries();
}

void Sentence::addPropArgument(const short &pPropNo, const WordIdx &pStart, 
                               const string &pLabel)
{
 props[pPropNo-1]->addArgument(pStart, pLabel);
}

short Sentence::getPropArgCount(const short &pPropNo)
{
 return props[pPropNo-1]->getArgCount();
}

/**
 *  iterates propositions of the sentence to gather the predicates and
 *  predicted arguments of each of them in start/end format, then put 
 *  them in a vector specific for that proposition
 */
void Sentence::getPropsArgsSEStrs(vector< vector<string> > &pArgSEStrs,
                                  unsigned short pFeatureView)
{
 pArgSEStrs.resize(props.size());
 
 short cntProp = 0;
 
 for (vector<Proposition *>::iterator itProp = props.begin();
      itProp < props.end(); 
      itProp++, cntProp++)
 {
  pArgSEStrs[cntProp].resize(words.size());
  
  (*itProp)->getPredSEStr(pArgSEStrs[cntProp]);
  (*itProp)->getArgsSEStrs(pArgSEStrs[cntProp], pFeatureView);
  }
}

void Sentence::setPropArgSpanEnd(const short &pPropNo, 
                                 const short &pArgNo,
                                 const WordIdx &pEnd)
{
 props[pPropNo-1]->setArgSpanEnd(pArgNo, pEnd);
}

void Sentence::findPropArgsInSpan(vector<Argument *> &pArgs, 
                                  WordSpan &pSpan)
{
 for (vector<Proposition *>::iterator it = props.begin(); 
      it < props.end(); 
      it++)
 {
  if (Argument *vArg = (*it)->findArgInSpan(pSpan))
   pArgs.push_back(vArg);
  }
}

/**
 *  assigns the passed constituent to the predicate of the passed proposition
 */
void Sentence::assignToPropPred(const short &pPropNo, Constituent const &pCnstnt) 
{
 props[pPropNo-1]->setPredCnstnt(pCnstnt);
}

void Sentence::displayProps()
{
 for (vector<Proposition *>::iterator it = props.begin(); 
      it < props.end(); 
      it++)
 {
  cout << (*it)->getPredLemma() << " [" << (*it)->getPredCnstntSpan().start << "-"
                                        << (*it)->getPredCnstntSpan().end << "]"
                                   " [" << (*it)->getPredSpan().start << "-"
                                        << (*it)->getPredSpan().end << "] : ";
  (*it)->displayArgs();
  cout << endl;
  }
}

/**
 *  iterates propositions to analyze them for checking consistency of
 *  loading and also extracting statistics according to DataAnalysis 
 *  struture
 */
void Sentence::analyzeData(DataAnalysis *&pDataAnalysis)
{
 // analyzing propositions and their arguments
 for (vector<Proposition *>::iterator itProp = props.begin(); 
      itProp< props.end(); 
      itProp++)
  (*itProp)->analyzeData(pDataAnalysis); 
  
 // analyzing tree constituents
 tbTree->analyzeData(pDataAnalysis);
 
 pDataAnalysis->addSntncAnalyzed(); 
}

/**
 *  iterate propositions to generate samples for each proposition 
 *  (generating samples and extracting features for each sample) and
 *  gathers together the analysis and statistics returned by propositions
 * 
 *  If an stream is assigned to pstrmZME, generated samples will be 
 *  written into a file in the format of Zhang Maximum Entropy tool
 */

int Sentence::generateLabeledSamples(LabeledSampleGenerator * const &pSG,
                                     vector<Sample *> &pPositiveSamples, 
                                     vector<Sample *> &pNegativeSamples,
                                     unsigned short pViewCount,
                                     unsigned short pFeatureSets[],
                                     ofstream &pStrmZME)
{
 for (vector<Proposition *>::iterator itProp = props.begin(); 
      itProp< props.end(); 
      itProp++)
 {  
  if (pSG->reachedMaxPositive() && pSG->reachedMaxNegative())
   break;
  
  if ((*itProp)->generateLabeledSamples(pSG, pPositiveSamples, pNegativeSamples,
                                        pViewCount, pFeatureSets, pStrmZME) != 0)
   return -1;
  
  pSG->incPropCount();
  }

 return 0;
}


/**
 *  iterate propositions to generate samples for each proposition 
 *  (generating samples and extracting features for each sample) and
 *  gathers together the analysis and statistics returned by propositions
 * 
 *  If an stream is assigned to pstrmZME, generated samples will be 
 *  written into a file in the format of Zhang Maximum Entropy tool
 */
int Sentence::generateUnlabeledSamples(UnlabeledSampleGenerator *const &pSG,
                                       vector<Sample *> &pSamples,
                                       unsigned short pViewCount,
                                       unsigned short pFeatureSets[],
                                       ofstream &pStrmZME)
{
 for (vector<Proposition *>::iterator itProp = props.begin(); 
      itProp< props.end(); 
      itProp++)
 {  
  if ((*itProp)->generateUnlabeledSamples(pSG, pSamples, pViewCount, pFeatureSets, pStrmZME) != 0)
   return -1;
  
  pSG->incPropCount();
  }

 return 0;
}


/**
 *  iterate propositions to generate samples for each proposition 
 *  (generating samples and extracting features for each sample) and
 *  gathers together the analysis and statistics returned by propositions
 * 
 *  If an stream is assigned to pstrmZME, generated samples will be 
 *  written into a file in the format of Zhang Maximum Entropy tool
 */
int Sentence::generateTestSamples(TestSampleGenerator * const &pSG,
                                  vector<Sample *> &pSamples, 
                                  unsigned short pViewCount,
                                  unsigned short pFeatureSets[],
                                  ofstream &pStrmZME)
{
 for (vector<Proposition *>::iterator itProp = props.begin(); 
      itProp< props.end(); 
      itProp++)
 {  
  if ((*itProp)->generateTestSamples(pSG, pSamples, pViewCount, pFeatureSets, pStrmZME) != 0)
   return -1;
  
  pSG->incPropCount();
  }

 return 0;
}

unsigned short Sentence::getSampleCount()
{
 unsigned short vSampleCount = 0;
 
 for (vector<Proposition *>::iterator itProp = props.begin(); 
      itProp< props.end(); 
      itProp++)
  vSampleCount += (*itProp)->getSampleCount();

 return vSampleCount;
} 

double Sentence::getLabelingProbability(unsigned short pFeatureView)
{
 unsigned int cntSamples = 0;
 double probSum = 0;
 
 for (vector<Proposition *>::iterator itProp = props.begin(); 
      itProp< props.end(); 
      itProp++)
 {  
  vector<Sample *> vSamples;
  (*itProp)->getSamples(vSamples);	     
  
  for (vector<Sample *>::iterator itSample = vSamples.begin(); 
       itSample < vSamples.end(); 
       itSample++)
  {
   probSum += (*itSample)->getPrdArgLabelProb(pFeatureView);
   cntSamples++;   
   }
  } 
 
 if (cntSamples == 0)
  return 0;
 else
  return probSum / cntSamples;
}

double Sentence::getLabelingAgreement()
{
 unsigned int cntSamples = 0;
 double agreementSum = 0;
 
 for (vector<Proposition *>::iterator itProp = props.begin(); 
      itProp< props.end(); 
      itProp++)
 {  
  vector<Sample *> vSamples;
  (*itProp)->getSamples(vSamples);	     
  
  for (vector<Sample *>::iterator itSample = vSamples.begin(); 
       itSample < vSamples.end(); 
       itSample++)
  {
   if ((*itSample)->isPrdArgLabelsAgreed())
    agreementSum++;
   cntSamples++;   
   }
  } 
 
 if (cntSamples == 0)
  return 0;
 else
  return agreementSum / cntSamples;
}

void Sentence::setSelectedPrdLabels(unsigned short pSelectionMethod,
                                    unsigned short pPreferedView)
{
 for (vector<Proposition *>::iterator itProp = props.begin(); 
      itProp< props.end(); 
      itProp++)
 {  
  vector<Sample *> vSamples;
  (*itProp)->getSamples(vSamples);	     
  
  for (vector<Sample *>::iterator itSample = vSamples.begin(); 
       itSample < vSamples.end(); 
       itSample++)
   (*itSample)->setSelectedPrdLabel(pSelectionMethod, pPreferedView);
  } 
}

void Sentence::logPrdLabeling(unsigned short pFeatureView, ofstream &pStrm)
{
 if(pStrm.is_open())
 {
  pStrm.seekp(pStrm.tellp(),fstream::beg);
  
  unsigned int cntSamples = 0;
  double probSum = 0;

  display(pStrm);
        
  for (vector<Proposition *>::iterator itProp = props.begin(); 
       itProp< props.end(); 
       itProp++)
  {
   pStrm << (*itProp)->getPredForm() << ":" << endl;
   	  
   vector<Sample *> vSamples;
   (*itProp)->getSamples(vSamples);	     
   
   for (vector<Sample *>::iterator itSample = vSamples.begin(); 
        itSample < vSamples.end(); 
        itSample++)
   {
    probSum += (*itSample)->getPrdArgLabelProb(pFeatureView);
    cntSamples++;
    
    pStrm << cntSamples << ": " 
          << (*itSample)->getPrdArgLabel(pFeatureView) << "; "
          << (*itSample)->getWordsStr() << "; "
          << (*itSample)->getPrdArgLabelProb(pFeatureView)
          << endl;
    }
   } 
   
  pStrm << ((cntSamples == 0) ? 0 : (probSum / cntSamples ))
        << endl << endl; 
  }
}
  
void Sentence::logPrdLabelings(unsigned short pViewCount, ofstream &pStrm)
{
 if(pStrm.is_open())
 {
  pStrm.seekp(pStrm.tellp(),fstream::beg);
  
  unsigned int cntSamples = 0;
  double probSum[pViewCount];

  for (unsigned short cntView = 0; cntView <= pViewCount; cntView++)
   probSum[cntView] = 0;

  display(pStrm);
        
  for (vector<Proposition *>::iterator itProp = props.begin(); 
       itProp< props.end(); 
       itProp++)
  {
   pStrm << (*itProp)->getPredForm() << ":" << endl;
   	  
   vector<Sample *> vSamples;
   (*itProp)->getSamples(vSamples);	     
   
   for (vector<Sample *>::iterator itSample = vSamples.begin(); 
        itSample < vSamples.end(); 
        itSample++)
   {
    for (unsigned short cntView = 0; cntView <= pViewCount; cntView++)
     probSum[cntView] += (*itSample)->getPrdArgLabelProb(cntView);
    
    cntSamples++;
    
    pStrm << cntSamples << ": ";
    
    for (unsigned short cntView = 1; cntView <= pViewCount; cntView++)
    {
     pStrm << (*itSample)->getPrdArgLabel(cntView) << ":";
     pStrm << (*itSample)->getPrdArgLabelProb(cntView) << ";" ;
     }
        
    // selected common label
    pStrm << "[" << (*itSample)->getPrdArgLabel(0) << ":";
    pStrm << (*itSample)->getPrdArgLabelProb(0) << "];";
    
    pStrm << " " << (*itSample)->getWordsStr() << "; ";

    pStrm << endl;
    }
   } 
   
  for (unsigned short cntView = 1; cntView <= pViewCount; cntView++)
   pStrm << ((cntSamples == 0) ? 0 : (probSum[cntView] / cntSamples )) << ";";
  
  // selection average probability
  pStrm << "[" << ((cntSamples == 0) ? 0 : (probSum[0] / cntSamples )) << "];"
        << endl;
  
  pStrm << "Agreement: " << getLabelingAgreement() << endl 
        << "Selection Probability: " << ((cntSamples == 0) ? 0 : (probSum[0] / cntSamples )) << endl
        << endl; 
  }
}
  
/**
 * 
 *  Word class definitions
 * 
 */

Word::Word ()
{
 wordCnstnt = NULL;
 wordDTNode = NULL;
}

void Word::setWordForm(string pWordForm)
{
 wordForm = pWordForm;
} 

string Word::getWordForm()
{
 return wordForm;
}

string Word::getPOS()
{
 return getCnstnt()->getSyntTag();
}

void Word::setWordLemma(string pLemma)
{
 wordLemma = pLemma;
} 

string Word::getWordLemma()
{
 return wordLemma;
}

void Word::setCnstnt(Constituent &pCnstnt)
{
 wordCnstnt = pCnstnt;
}

Constituent  Word::getCnstnt()
{
 return wordCnstnt;
}

void Word::setDTNode(DTNode &pDTNode)
{
 wordDTNode = pDTNode;
}

DTNode Word::getDTNode()
{
 return wordDTNode;
}

WordIdx Word::getDepHeadWordIdx()
{
 DTNode vParentNode = getDTNode()->getParent();
 
 // if the head (parent) is root, return 0
 if (vParentNode->isRoot())
  return 0;
 else
  return vParentNode->getWordIdx();
}

/**
 * 
 *  Proposition class definitions
 * 
 */

Proposition::Proposition(Sentence * const &pSentence)
{
 sentence = pSentence;
 predCnstnt = NULL;
 predCSpan.start = 0;
 predCSpan.end = 0;
 predCCnstnt = NULL;
}

Sentence *Proposition::getSentence()
{
 return sentence;
}

void Proposition::setPredWord(const WordIdx &pWord)
{
 predWord = pWord;
}

WordIdx Proposition::getPredWordIdx()
{
 return predWord;
}

WordPtr Proposition::getPredWord()
{
 return sentence->getWord(predWord);
}

void Proposition::setPredSpanStart(const WordIdx &pStart)
{
 predSpan.start = pStart;
}

void Proposition::setPredSpanEnd(const WordIdx &pEnd)
{
 predSpan.end = pEnd;
}

void Proposition::setPredCSpanStart(const WordIdx &pStart)
{
 predCSpan.start = pStart;
}

void Proposition::setPredCSpanEnd(const WordIdx &pEnd)
{
 predCSpan.end = pEnd;
}

WordSpan Proposition::getPredSpan()
{
 return predSpan;
}

WordSpan Proposition::getPredCnstntSpan()
{
 return predCnstnt->getWordSpan();
}

short Proposition::getPredLength()
{
 return predSpan.end - predSpan.start + 1;
}

void Proposition::setPredLemma(const string &pLemma)
{
 predLemma = pLemma;
}

string Proposition::getPredLemma()
{
 return predLemma;
}

string Proposition::getPredForm()
{
 return sentence->getWordForm(predWord);
}

void Proposition::setPredCnstnt(Constituent const &pCnstnt)
{
 predCnstnt = pCnstnt;
}

Constituent Proposition::getPredCnstnt()
{
 return predCnstnt;
}

Constituent Proposition::getPredCCnstnt()
{
 return predCCnstnt;
}

/**
 *  looking for particles ("RP") after verb predicate to figure out whether
 *  it is compound or not
 * 
 *  If the particle is just after the verb, it extends the predicate span
 *  to include it. If it is 2 words after the verb, the verb is considered
 *  as discontinious compound and its continuation constituent and span
 *  is set. Only 1 word distance is treated for discontinious compound here
 *  (e.g. "give it up")
 */
void Proposition::estimatePredBoundaries()
{
 // if the predicate is the last word, do nothing
 if (predWord == sentence->getLength())
  return;
  
 // cheking for continious compund verbs
 if (sentence->getWordCnstnt(predWord + 1)->getSyntTag() == "RP")
 {
  predSpan.end++; 
  return;
  }

 // checking for discontinious compound verbs with 1 word distance between
 // verb and particle
 if (predWord <= sentence->getLength() - 2)
  if (sentence->getWordCnstnt(predWord + 2)->getSyntTag() == "RP")
  {
   predCSpan.start = predWord + 2;
   predCSpan.end = predWord + 2; 
   predCCnstnt = sentence->getWordCnstnt(predWord + 2);
   return;
   }
}

string Proposition::getPredPOS()
{
 return predCnstnt->getSyntTag();
}

/**
 *  The algorithm for detecting predicate voice is adapted from 
 *  Igo & Rillof (2008). (../../references/Sean Igo Passive Voice Detection Rules.htm)
 * 
 *  The rules are 2 part: the 1st part consists of 2 rules that detects 
 *  ordinary passive verbs in which a passive auxiliary precede the past
 *  participle verb (VBN), and the 2nd part consisting of 4 rules detects
 *  reduced passive verbs which lack of any auxiliary.
 * 
 *  A function for each rule is called by this function if the previous 
 *  one could not detect the passive voice. If finally none of functions 
 *  recognize the passive voice, it is considered as active and the boolean
 *  true value is returned.
 * 
 *  Note:
 *  For some of the examples in the rules web page, the rule does not apply
 *  to the example according to Charniak parses, because those are sampled
 *  from gold Penn trees. Interestingly, other rules may appley in this cases.
 */
bool Proposition::getPredVoice()
{
 string vPredPOS;
 vPredPOS = predCnstnt->getSyntTag();
 
 if ((vPredPOS != "VBN"))
  return true;
 else if (isIgoOrdinaryPassive1())
  return false;
 else if (isIgoOrdinaryPassive2())
  return false;
 else if (isIgoReducedPassive1())
  return false;
 else if (isIgoReducedPassive2())
  return false;
 else if (isIgoReducedPassive3())
  return false;
 else if (isIgoReducedPassive4())
  return false;
 else
  return true;
}

/**
 *  detecting the ordinary passive by rule 1:
 * 
 *  - Parent is a VP
 *  - Starting with parent and climbing nested VP ancestors, the closest
 *    verb sibling before any VP ancestor is a passive auxiliary
 */ 
bool Proposition::isIgoOrdinaryPassive1()
{
 Constituent vCurrNode = predCnstnt->getParent();
 Constituent vParent;
 // this consition is for protective purpose (getting parent of a NULL node)
 if	(vCurrNode != NULL)
  vParent = vCurrNode->getParent();	
 else
  return false;
 
 while ((vCurrNode != NULL) && (vParent != NULL))
 {
  if ((vCurrNode->getSyntTag() == "VP") && (vParent != NULL))
  {
   short childOrdinal = vCurrNode->getChildOrdinal();
   // looking at the left siblings to find a passive auxiliary
   for (short cntLSibling = childOrdinal - 1; 
       cntLSibling >= 1; 
       cntLSibling--)
   {
    if ((vParent->getChild(cntLSibling)->getSyntTag().substr(0,2) == "VB") ||
        (vParent->getChild(cntLSibling)->getSyntTag().substr(0,3) == "AUX"))
    {
     if (isPassiveAux(vParent->getChild(cntLSibling)->getWordsStr()))
      return true;
     else 
      return false;
     }
   	} 
   
   vCurrNode = vCurrNode->getParent();
   vParent = vCurrNode->getParent();	
   }
  else
   break;	    
  }

 return false;
}

/**
 *  detecting the ordinary passive by rule 2:
 * 
 *  - Parent (and nested ancestors) is ADJP
 *  - Oldest ADJP ancestor’s parent is VP
 *  - Closest verb sibling before oldest ADJP ancestor is a passive auxiliary
 */ 
bool Proposition::isIgoOrdinaryPassive2()
{
 Constituent vCurrNode = predCnstnt->getParent();
 Constituent vParent;
 // this consition is for protective purpose (getting parent of a NULL node)
 if	(vCurrNode != NULL)
  vParent = vCurrNode->getParent();	
 else
  return false;
 
 while ((vCurrNode != NULL) && (vParent != NULL))
 {
  if ((vCurrNode->getSyntTag() == "ADJP") && (vParent != NULL))
  {
   if ((vParent->getSyntTag() == "ADJP"))
   {
    vCurrNode = vCurrNode->getParent();
    vParent = vCurrNode->getParent();	
    continue;
    }  
   else if ((vParent->getSyntTag() == "VP")) 
   {
    short childOrdinal = vCurrNode->getChildOrdinal();
    // looking at the left siblings to find a passive auxiliary
    for (short cntLSibling = childOrdinal - 1; 
        cntLSibling >= 1; 
        cntLSibling--)
     if ((vParent->getChild(cntLSibling)->getSyntTag().substr(0,2) == "VB") ||
         (vParent->getChild(cntLSibling)->getSyntTag().substr(0,3) == "AUX"))
     {
      if (isPassiveAux(vParent->getChild(cntLSibling)->getWordsStr()))
       return true;
      else 
       return false;
      }
    
    break; 
    }
   else
    break;  
   }
  else
   break; 	    
  }

 return false;
}

/**
 *  detecting the reduced passive by rule 1:
 * 
 *  - Parent and any nested ancestors are VPs
 *  - None of VP ancestors’ preceding siblings is verb
 *  - Parent of oldest VP ancestor is NP
 */ 
bool Proposition::isIgoReducedPassive1()
{
 Constituent vCurrNode = predCnstnt->getParent();
 Constituent vParent;
 // this consition is for protective purpose (getting parent of a NULL node)
 if	(vCurrNode != NULL)
  vParent = vCurrNode->getParent();	
 else
  return false;
 
 while ((vCurrNode != NULL) && (vParent != NULL))
 {
  if ((vCurrNode->getSyntTag() == "VP") && (vParent != NULL))
  {
   if ((vParent->getSyntTag() != "VP") &&
       (vParent->getSyntTag() != "NP"))
    return false;
    
   short childOrdinal = vCurrNode->getChildOrdinal();
   // looking at the left siblings to find any verb
   for (short cntLSibling = childOrdinal - 1; 
        cntLSibling >= 1; 
        cntLSibling--)
     if (vParent->getChild(cntLSibling)->getSyntTag().substr(0,2) == "VB")
      return false;

   if (vParent->getSyntTag() == "NP")
    return true;
   else
   {
    vCurrNode = vCurrNode->getParent();
    vParent = vCurrNode->getParent();	
    }
   }
  else 
   break;
  }

 return false;
}

/**
 *  detecting the reduced passive by rule 2:
 * 
 *  - Parent is a PP
 */ 
bool Proposition::isIgoReducedPassive2()
{
 if (predCnstnt->getParent() != NULL)
  if (predCnstnt->getParent()->getSyntTag() == "PP")
   return true;
 
 return false;
}

/**
 *  detecting the reduced passive by rule 3:
 * 
 *  - Parent is VP and Grandparent is Sentence (clause)
 *  - Great-grandparent is clause, NP, VP, or PP
 */ 
bool Proposition::isIgoReducedPassive3()
{
 if (predCnstnt->getParent() != NULL)
  if (predCnstnt->getParent()->getSyntTag() == "VP")
   if (predCnstnt->getParent()->getParent() != NULL)
    if (predCnstnt->getParent()->getParent()->getSyntTag()[0] == 'S')
     if (predCnstnt->getParent()->getParent()->getParent() != NULL)
      if ((predCnstnt->getParent()->getParent()->getParent()->getSyntTag()[0] == 'S') ||
          (predCnstnt->getParent()->getParent()->getParent()->getSyntTag() == "NP") ||
          (predCnstnt->getParent()->getParent()->getParent()->getSyntTag() == "VP") ||
          (predCnstnt->getParent()->getParent()->getParent()->getSyntTag() == "PP"))
       return true;  
      
 return false;
}

/**
 *  detecting the reduced passive by rule 4:
 * 
 *  - Parent (and nested ancestors) is ADJP
 *  - None of oldest ADJP ancestor’s preceding siblings is a determiner
 *  - None of oldest ADJP ancestor’s following siblings is a noun or NP
 */ 
bool Proposition::isIgoReducedPassive4()
{
 Constituent vCurrNode = predCnstnt->getParent();
 Constituent vParent;
 // this consition is for protective purpose (getting parent of a NULL node)
 if	(vCurrNode != NULL)
  vParent = vCurrNode->getParent();	
 else
  return false;
 
 while ((vCurrNode != NULL) && (vParent != NULL))
 {
  if ((vCurrNode->getSyntTag() == "ADJP") && (vParent != NULL))
  {
   if ((vParent->getSyntTag() == "ADJP"))
   {
    vCurrNode = vCurrNode->getParent();
    vParent = vCurrNode->getParent();	
    continue;
    }  
   else
   {
    short childOrdinal = vCurrNode->getChildOrdinal();
    
    // looking at the left siblings to find a determiner
    for (short cntLSibling = childOrdinal - 1; 
        cntLSibling >= 1; 
        cntLSibling--)
     if (vParent->getChild(cntLSibling)->getSyntTag() == "DT")
      return false;

    // looking at the right siblings to find a noun or NP
    for (short cntRSibling = childOrdinal + 1; 
        cntRSibling <= vParent->getChildrenCount(); 
        cntRSibling++)
     if ((vParent->getChild(cntRSibling)->getSyntTag() == "NP") ||
         (vParent->getChild(cntRSibling)->getSyntTag()[0] == 'N'))
      return false;
      
     return true;
    }
   }
  else
   break;  
  }

 return false;
}
 
void Proposition::setProbablePredCCnstnt(Constituent const &pCnstnt)
{
 if ((predCSpan.start == pCnstnt->getWordSpan().start) && 
     (predCSpan.end == pCnstnt->getWordSpan().end))
  predCCnstnt = pCnstnt;
}

void Proposition::addArgument(const WordIdx &pStart, const string &pLabel)
{
 try
 {
  args.push_back(new Argument(this, pStart, pLabel)) ;
  }
 catch (bad_alloc &)
 {  
  cerr << "Memory allocation failed for argument!" << endl;
  }
}

short Proposition::getArgCount()
{
 return args.size();
}

void Proposition::setArgSpanEnd(const short &pArgNo, const WordIdx &pEnd)
{
 args[pArgNo-1]->setSpanEnd(pEnd);
}

/**
 *  puts the predicate (V*) and its probable continuation (C-V*) in 
 *  start/end format in a vector
 * 
 *  The vector is in the length of the sentence and is filled with strings 
 *  representing the starting of the predicate, predicate continuation 
 *  and predicted arguments (e.g. "(A0*") and their ending (e.g. "*)"). 
 *  Other elements are filled with "*".
 */
void Proposition::getPredSEStr(vector<string> &pArgSEStrs)
{
 // setting predicate start/end string
 if (predSpan.start == predSpan.end)
 {
  pArgSEStrs[predSpan.start - 1] = "(V*)";
  }
 else
 {
  pArgSEStrs[predSpan.start - 1] = "(V* ";
  pArgSEStrs[predSpan.end - 1] = "*)";
  }

 // setting predicate continuation start/end string if any
 if (predCSpan.start != 0)
 {
  if (predCSpan.start == predCSpan.end)
  {
   pArgSEStrs[predCSpan.start - 1] = "(C-V*)";
   }
  else
  {
   pArgSEStrs[predCSpan.start - 1] = "(C-V* ";
   pArgSEStrs[predCSpan.end - 1] = "*)";
   }
  }
}

/**
 *  iterates samples of the propositions to gather the predicted arguments
 *  of each of them in start/end format, then put them in a vector
 * 
 *  The vector is in the length of the sentence and is filled with strings 
 *  representing the starting of the predicate, predicate continuation 
 *  and predicted arguments (e.g. "(A0*") and their ending (e.g. "*)"). 
 *  Other elements are filled with "*".
 */
void Proposition::getArgsSEStrs(vector<string> &pArgSEStrs, unsigned short pFeatureView)
{
 for (vector<Sample *>::iterator itSample = samples.begin(); 
      itSample < samples.end(); 
      itSample++)
 {
  if (!(*itSample)->isPrdArgument(pFeatureView))
   continue;
   
  /////////////////////////////////////////////////////////////////////
  // this is for temporarily treating overlaping arguments which happens 
  // in two cases and causes error when using CoNLL srl-eval:
  // 1st when no global constraint is applied on labeled predictions
  // 2nd when the wrong predictions causes conflic between label and 
  // compound verb predicate boundary
  if ((pArgSEStrs[(*itSample)->getWordSpan().start - 1] != "") ||
      (pArgSEStrs[(*itSample)->getWordSpan().end - 1] != ""))
   continue;  
  ///////////////////////////////////////////////////////////////////// 
   
  // if the argument span is only one word write start and end together,
  // otherwise, write start to the element according to argument's span 
  // start and end according to its span end
  if ((*itSample)->getWordSpan().start == 
      (*itSample)->getWordSpan().end)
  {
   pArgSEStrs[(*itSample)->getWordSpan().start - 1] = "("+
                                                      (*itSample)->getPrdArgLabel(pFeatureView) +
                                                      "*)";
   }
  else
  {
   pArgSEStrs[(*itSample)->getWordSpan().start - 1] = "(" + 
                                                      (*itSample)->getPrdArgLabel(pFeatureView) + 
                                                      "* ";
   pArgSEStrs[(*itSample)->getWordSpan().end - 1] = "*)";
   }
  }
}

Argument* Proposition::findArgInSpan(WordSpan &pSpan)
{
 for (vector<Argument *>::iterator it = args.begin(); 
      it < args.end(); 
      it++)
 {
  WordSpan vSpan = (*it)->getSpan();
  if ((vSpan.start == pSpan.start) && (vSpan.end == pSpan.end))
   return (*it);
  } 
 return NULL;
}

Argument* Proposition::findArgMatch(Constituent &pCnstnt)
{
 for (vector<Argument *>::iterator it = args.begin(); 
      it < args.end(); 
      it++)
 {
  if ((*it)->getArgCnstnt() == pCnstnt)
   return (*it);
  } 
 return NULL;
}

void Proposition::displayArgs()
{
 for (vector<Argument *>::iterator it = args.begin(); 
      it < args.end(); 
      it++)
 {
  cout << (*it)->getLabel(); 

  if (Constituent vArgCnstnt = (*it)->getArgCnstnt())
   cout << "[" << vArgCnstnt->getSyntTag() << "]";
  else
   cout << "[NULL]";
  
  cout << "[" << (*it)->getSpan().start << "-"
                                    << (*it)->getSpan().end << "]; ";
  }
}

/**
 *  iterates arguments and process predicate to analyze them for checking
 *  consistency of loading and also extracting statistics according to
 *  DataAnalysis struture
 */
void Proposition::analyzeData(DataAnalysis *&pDataAnalysis)
{
 for (vector<Argument *>::iterator itArg = args.begin(); 
      itArg < args.end(); 
      itArg++)
  (*itArg)->analyzeData(pDataAnalysis);
  
 // checks whether predicate is compound
 if (predCSpan.start != 0)
  pDataAnalysis->addPredDscntCompound(this);
 else if (getPredLength() > 1)
  pDataAnalysis->addPredCntCompound(this);

 pDataAnalysis->addPropAnalyzed(); 
}

/**
 *  generates positive and negative samples by calling related functions
 */
int Proposition::generateLabeledSamples(LabeledSampleGenerator * const &pSG,
                                        vector<Sample *> &pPositiveSamples, 
                                        vector<Sample *> &pNegativepSamples,
                                        unsigned short pViewCount,
                                        unsigned short pFeatureSets[],
                                        ofstream &pStrmZME)
{
 if (!pSG->reachedMaxPositive())
  if (generatePositives(pSG, pPositiveSamples, pViewCount, pFeatureSets, pStrmZME) != 0)
   return -1;
 
 if (!pSG->reachedMaxNegative())
  if (generateNegatives(pSG, pNegativepSamples, pViewCount, pFeatureSets, pStrmZME) != 0)
   return -1;
 
 return 0;
}

/**
 *  generates ALL positive samples by iterating arguments of proposition
 *  (generating samples and extracting features for each sample) until 
 *  it reaches the limit of positive samples number (i.e. ALL means
 *  not pruning any candidate) and gathers together the analysis and
 *  statistics returned by propositions
 * 
 *  It skips the arguments with no match to a constituent
 * 
 *  If an stream is assigned to pstrmZME, generated samples will be 
 *  written into a file in the format of Zhang Maximum Entropy tool
 */
int Proposition::generatePositives(LabeledSampleGenerator * const &pSG,
                                   vector<Sample *> &pSamples,
                                   unsigned short pViewCount,
                                   unsigned short pFeatureSets[],
                                   ofstream &pStrmZME)
{
 for (vector<Argument *>::iterator itArg = args.begin(); 
      (itArg < args.end()) && (!pSG->reachedMaxPositive()); 
      itArg++)
 {
  Constituent tmpCnstnt = (*itArg)->getArgCnstnt();
  // skip this argument if it does not match any constituent
  if (!tmpCnstnt)
   continue;
  
  ArgLabel tmpLabel = (*itArg)->getLabel();
  Sample *tmpSample = NULL;
  
  try
  {
   tmpSample = new Sample (this, tmpCnstnt, pViewCount, pFeatureSets, tmpLabel, pStrmZME);
   }
  catch (bad_alloc &)
  {  
   cerr << "Memory allocation failed for sample!" << endl;
   return -1;
   }
 
  pSamples.push_back(tmpSample);
  pSG->incPositiveCount();
  }
  
 return 0;
}

/**
 *  collects potential argument-bearing constituents using Xue & Palmer
 *  PRUNING heuristic and generates negative samples by selecting from 
 *  these constituents those which are not arguments of this proposition
 *  (positive).
 * 
 *  It checks for positive arguments of the proposition that are missed
 *  by XP heuristic
 */
int Proposition::generateNegatives(LabeledSampleGenerator * const &pSG,
                                   vector<Sample *> &pSamples,
                                   unsigned short pViewCount,
                                   unsigned short pFeatureSets[],
                                   ofstream &pStrmZME)
{
 vector<Constituent> vXPCnstnts;
 // to store positive arguments of this proposition collected by XP
 // for analysis purpose
 vector<Argument *> vXPPositives;
 
 predCnstnt->collectXPCnstnts(vXPCnstnts);
  
 for (vector<Constituent>::iterator itCnstnt = vXPCnstnts.begin(); 
      (itCnstnt < vXPCnstnts.end()) && (!pSG->reachedMaxNegative()); 
      itCnstnt++)
 {
  // ignores XP constituent that is positive argument
  if (Argument * vXPPositive = this->findArgMatch(*itCnstnt))
  {
   vXPPositives.push_back(vXPPositive);
   continue;
   }
  
  Constituent tmpCnstnt = *itCnstnt;
  ArgLabel tmpLabel = "NULL";
  Sample *tmpSample = NULL;
  
  try
  {
   tmpSample = new Sample (this, tmpCnstnt, pViewCount, pFeatureSets, tmpLabel, pStrmZME);
   }
  catch (bad_alloc &)
  {  
   cerr << "Memory allocation failed for sample!" << endl;
   return -1;
   }
 
  pSamples.push_back(tmpSample);
  pSG->incNegativeCount();
  }
  
 // if the number of argumnents of this proposition is not equal to the
 // number of positive arguments collected by XP, then check it to find 
 // argument missed by XP and add to sample generation analysis data
 if (args.size() != vXPPositives.size())
  checkForXPMissedArgs(pSG, vXPPositives);
  
 return 0;
}


/**
 *  collects potential argument-bearing constituents using Xue & Palmer
 *  PRUNING heuristic and generates unlabeled samples
 * 
 *  It checks for positive arguments of the proposition that are missed
 *  by XP heuristic
 */
int Proposition::generateUnlabeledSamples(UnlabeledSampleGenerator * const &pSG,
                                          vector<Sample *> &pSamples, 
                                          unsigned short pViewCount,
                                          unsigned short pFeatureSets[],
                                          ofstream &pStrmZME)
{
 vector<Constituent> vXPCnstnts;
 predCnstnt->collectXPCnstnts(vXPCnstnts);
  
 for (vector<Constituent>::iterator itCnstnt = vXPCnstnts.begin(); 
      itCnstnt < vXPCnstnts.end(); 
      itCnstnt++)
 {
  Constituent tmpCnstnt = *itCnstnt;
  Sample *tmpSample = NULL;
  
  try
  {
   ArgLabel tmpArgLabel = "NULL";	
   tmpSample = new Sample (this, tmpCnstnt, pViewCount, pFeatureSets, tmpArgLabel, pStrmZME);
   }
  catch (bad_alloc &)
  {  
   cerr << "Memory allocation failed for sample!" << endl;
   return -1;
   }
 
  pSamples.push_back(tmpSample);
  pSG->incSampleCount();
  }
  
 return 0;
}


/**
 *  collects potential argument-bearing constituents using Xue & Palmer
 *  PRUNING heuristic and generates test samples
 * 
 *  It checks for positive arguments of the proposition that are missed
 *  by XP heuristic
 */
int Proposition::generateTestSamples(TestSampleGenerator * const &pSG,
                                     vector<Sample *> &pSamples, 
                                     unsigned short pViewCount,
                                     unsigned short pFeatureSets[],
                                     ofstream &pStrmZME)
{
 vector<Constituent> vXPCnstnts;
 // to store (positive) arguments of this proposition collected by XP
 // for analysis purpose
 vector<Argument *> vXPPositives;
 
 predCnstnt->collectXPCnstnts(vXPCnstnts);
  
 for (vector<Constituent>::iterator itCnstnt = vXPCnstnts.begin(); 
      itCnstnt < vXPCnstnts.end(); 
      itCnstnt++)
 {
  Constituent tmpCnstnt = *itCnstnt;
  ArgLabel tmpLabel = "NULL";
  Sample *tmpSample = NULL;
  
  // if this is an argument use its label for sample and keep it to use
  // for analysis
  Argument * vXPPositive = this->findArgMatch(*itCnstnt);
  if (vXPPositive)
  {
   vXPPositives.push_back(vXPPositive);
   tmpLabel = vXPPositive->getLabel();
   }
  
  try
  {
   tmpSample = new Sample (this, tmpCnstnt, pViewCount, pFeatureSets, tmpLabel, pStrmZME);
   }
  catch (bad_alloc &)
  {  
   cerr << "Memory allocation failed for sample!" << endl;
   return -1;
   }
 
  pSamples.push_back(tmpSample);
  pSG->incSampleCount();
  }
  
 // if the number of argumnents of this proposition is not equal to the
 // number of positive arguments collected by XP, then check it to find 
 // argument missed by XP and add to sample generation analysis data
 if (args.size() != vXPPositives.size())
  checkForXPMissedArgs(pSG, vXPPositives);

 return 0;
}


void Proposition::checkForXPMissedArgs(LabeledSampleGenerator * const &pSG,
                                       vector<Argument *> &pXPArgs)
{
 bool isFoundInXPArgs;
 
 for (vector<Argument *>::iterator itArg = args.begin(); 
      itArg < args.end(); 
      itArg++)
 {
  // if the arg has no matched constituent (due to parse errors) ignore it
  // and do not consider as XP-missed
  if (!(*itArg)->getArgCnstnt())
   continue;
  
  isFoundInXPArgs = false;
  	
  for (vector<Argument *>::iterator itXPArg = pXPArgs.begin(); 
       itXPArg < pXPArgs.end(); 
       itXPArg++)
  {
   if ((*itXPArg) == (*itArg))
   {
   	isFoundInXPArgs = true;
    break;
    }
   }   
  
  if (!isFoundInXPArgs)
   pSG->addArgXPMissed(*itArg);
  }
}

void Proposition::checkForXPMissedArgs(TestSampleGenerator * const &pSG,
                                       vector<Argument *> &pXPArgs)
{
 bool isFoundInXPArgs;
 
 for (vector<Argument *>::iterator itArg = args.begin(); 
      itArg < args.end(); 
      itArg++)
 {
  // if the arg has no matched constituent (due to parse errors) ignore it
  // and do not consider as XP-missed
  if (!(*itArg)->getArgCnstnt())
   continue;
  
  isFoundInXPArgs = false;
  	
  for (vector<Argument *>::iterator itXPArg = pXPArgs.begin(); 
       itXPArg < pXPArgs.end(); 
       itXPArg++)
  {
   if ((*itXPArg) == (*itArg))
   {
   	isFoundInXPArgs = true;
    break;
    }
   }   
  
  if (!isFoundInXPArgs)
   pSG->addArgXPMissed(*itArg);
  }
}

void Proposition::addSample(Sample * const &pSample)
{
 samples.push_back(pSample);
}

void Proposition::getSamples(vector<Sample *> &pSamples)
{
 pSamples = samples;
} 

unsigned short Proposition::getSampleCount()
{
 return samples.size();
} 
