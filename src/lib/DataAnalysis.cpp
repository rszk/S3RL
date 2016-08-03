/**
 *  @ RSK
 *  @ Version 0.1        03.12.2009
 * 
 *  (refer to DataAnalysis.hpp)
 * 
 */

#include <fstream>

#include "DataAnalysis.hpp"
#include "Constants.hpp"
#include "Sentence.hpp"
#include "Argument.hpp"
#include "Utils.hpp"


DataAnalysis::DataAnalysis()
{
 sntncsAnalyzed = 0;
 propsAnalyzed = 0;
 cnttntsAnalyzed = 0;
 argsAnalyzed = 0;
}

void DataAnalysis::addSntncAnalyzed()
{
 sntncsAnalyzed++;
}

void DataAnalysis::addPropAnalyzed()
{
 propsAnalyzed++;
}

void DataAnalysis::addCnttntAnalyzed()
{
 cnttntsAnalyzed++;
}

void DataAnalysis::addArgAnalyzed()
{
 argsAnalyzed++;
}

  
void DataAnalysis::addArgNoPointBack(Argument * const &pArg)
{
 argsNoPointBack.push_back(pArg);
}

void DataAnalysis::addArgOnLeaf(Argument * const &pArg)
{
 argsOnLeaf.push_back(pArg);
}

void DataAnalysis::addArgNoMatch(Argument * const &pArg)
{
 argsNoMatch.push_back(pArg);
}

void DataAnalysis::addCnstntNoPointBack(TBTree * const &pCnstnt)
{
 cnstntsNoPointBack.push_back(pCnstnt);
}

void DataAnalysis::addPredDscntCompound(Proposition * const &pProp)
{
 predsDscntCompound.push_back(pProp);
}

void DataAnalysis::addPredCntCompound(Proposition * const &pProp)
{
 predsCntCompound.push_back(pProp);
}

/**
 *  Logs the result of analysis of data loaded from corpora
 */
void DataAnalysis::logDataAnalysis(string &pDatasetID)
{
 string tmpLogFileName = DEFAULT_LOG_PATH;
 tmpLogFileName += "/analysis." + extractFileName(pDatasetID, true) + ".log";
 ofstream strmLog (tmpLogFileName.c_str());
 if(strmLog.is_open())
 {
  strmLog << "Analyzed:" << endl
          << endl
          << sntncsAnalyzed << " senetnces" << endl
          << propsAnalyzed << " propositions" << endl
          << argsAnalyzed << " arguments" << endl
          << cnttntsAnalyzed << " constituents" << endl
          << endl
          << endl;
  
  strmLog << "Found:" << endl
          <<endl;
  
  strmLog << argsNoMatch.size() 
          << " argument(s) had no match to any constituent" << endl
          << endl;
           
  strmLog << argsOnLeaf.size() 
          << " argument(s) were assigned to leaves:" << endl
          << endl;
  //logArgsOnLeaf(strmLog);
  //strmLog << endl;

  strmLog << argsNoPointBack.size() 
          << " argument(s) had not been pointed back by their assigned constituents" << endl
          << endl;
          
  strmLog << cnstntsNoPointBack.size() 
          << " constituent(s) had not been pointed back by their assigned arguments" << endl
          << endl;
          
  strmLog << predsDscntCompound.size() 
          << " discontinious compound predicate(s) were encountered" << endl
          << endl;
          
  strmLog << predsCntCompound.size() 
          << " continious compound predicate(s) were encountered" << endl
          << endl;
  
  strmLog.close();
  }
}

void DataAnalysis::logArgsOnLeaf(ostream &pStream)
{
 Sentence *vLastSntnc = NULL;
 Sentence *vCurrSntnc = NULL;
 
 for (vector<Argument *>::iterator itArg = argsOnLeaf.begin(); 
      itArg < argsOnLeaf.end(); 
      itArg++)
 {
  vCurrSntnc = (*itArg)->getProp()->getSentence();
  if (vLastSntnc != vCurrSntnc)
  {
   pStream << endl;
   vLastSntnc = vCurrSntnc;
   vCurrSntnc->display(pStream);
   } 
  pStream << endl
          << (*itArg)->getProp()->getPredLemma() << ": "
          << (*itArg)->getLabel()
          << " [" << (*itArg)->getWordsStr() << "]"
          << endl;
  }
}  

