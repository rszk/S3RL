/**
 *  @ RSK
 *  @ Version 1.1        26.02.2010
 * 
 *  Classes to store arguments
 * 
 *  A Proposition consists of some Arguments (see Sentence.hpp)
 *  An Argument belongs to a proposition
 *  An Argument matches to (currenly only) a constituent in the related TBTree
 */

#ifndef ARGUMENT_HPP
#define ARGUMENT_HPP

#include "Types.hpp"

using namespace std;


class Proposition;
class DataAnalysis;

/**
 *  Labels extracted from PropBank CoNLL 2005 data (see Swirl SRL system
 *  and Carreras & Marquez (2005)) with all frequencies (53 labels 
 *  including an AM-TM label, which seems as a mistaken AM-TMP and is 
 *  ignored here since its frequency is 2)
 * 
 *  However, the following labels have frequencies less than 10:
 *  CA4, CA5
 *  RA4, RA5
 *  AM, AM_TM
 *  CAM_ADV, CAM_EXT, CAM_TMP, CAM_LOC, CAM_CAU, CAM_DIS, CAMDIR, 
 *   CAM_NEG, CAM_PNC
 *  RAM_ADV, RAM_EXT, RAM_DIR
 * 
 *  Note: not all of the following labels have to be used in practice; 
 *        it will be decided based on complexity of the implemented system
 * 
 * A0, A1, A2, A3, A4, A5, AA,
 * CA0, CA1, CA2, CA3, CA4, CA5,
 * RA0, RA1, RA2, RA3, RA4, RAA,
 * AM, AM_TMP, AM_MOD, AM_ADV, AM_MNR, AM_LOC, AM_DIS,
 * AM_NEG, AM_PNC, AM_CAU, AM_DIR, AM_EXT, AM_PRD, AM_REC,
 * CAM_TMP, CAM_ADV, CAM_MNR, CAM_LOC, CAM_DIS, CAM_NEG, 
 * CAM_PNC, CAM_CAU, CAM_DIR, CAM_EXT,
 * RAM_TMP, RAM_ADV, RAM_MNR, RAM_LOC, RAM_PNC, RAM_CAU, 
 * RAM_DIR, RAM_EXT,
 * CV
 * 
 */
  
class Argument
{
 private:
  
  Proposition *prop;
  WordSpan wordSpan;
  ArgLabel label;
  Constituent argCnstnt;
  // argWord is the argument word in dependency-based SRL. Currently,
  // we use constituent-based argument to derive dependency-based argument
  // using the rules used for CoNLL 2008 data (See Surdeanu et al.), thus 
  // do not use it here.
  // The derivation is done by keeping the argWord in Constituent class,
  // which can be accessed by both Argument and Sample classes
  //WordPtr argWord;
 
 public:
  
  Argument(Proposition * const &, const WordIdx &, const ArgLabel &);
  
  Proposition *getProp();

  void setSpanEnd(const WordIdx &);
  WordSpan getSpan();
  short getLength();
  ArgLabel getLabel();
  string getWordsStr();
  
  void setArgCnstnt(const Constituent &);
  Constituent getArgCnstnt();
  // checks whether the passed constituent is assigend to this argument
  bool hasCnstnt(Constituent const &);
  
  void analyzeData(DataAnalysis *&);
};

#endif /* ARGUMENT_HPP */
