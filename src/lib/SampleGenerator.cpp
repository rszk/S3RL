/**
 *  @ RSK
 *  @ Version 1.1        17.06.2010
 * 
 *  (refer to SampleGenerator.hpp)
 * 
 */

#include <fstream>
#include <iostream>

#include "SampleGenerator.hpp"
#include "Types.hpp"
#include "Sentence.hpp"
#include "Argument.hpp"
#include "Constants.hpp"
#include "Utils.hpp"


/**
 * 
 *   definitions of sample generator classes
 * 
 */

/**
 *   LabeledSampleGenerator class definitions
 */

LabeledSampleGenerator::LabeledSampleGenerator()
{
 maxPositive = MAX_TRAIN_POSITIVE;
 maxNegative = MAX_TRAIN_NEGATIVE;
 
 sgAnalysis = new LabeledSGAnalysis;
 }

LabeledSampleGenerator::LabeledSampleGenerator(const int &pMax, const int &pMin)
{
 maxPositive = pMax;
 maxNegative = pMin;
 
 sgAnalysis = new LabeledSGAnalysis;
 }

LabeledSampleGenerator::~LabeledSampleGenerator()
{
 delete sgAnalysis;
 }

/**
 *  iterates sentences to generate samples for each sentence (generating
 *  samples and extracting features for each sample) for the feature sets
 *  specified and collects some analysis of the generation along the way
 * 
 *  If the value of pWriteZME is true, generated samples will be written
 *  into a file in the format of Zhang Maximum Entropy tool
 */
int LabeledSampleGenerator::generateSamples(vector<Sentence *> &pSentences,
                                            vector<Sample *> &pPositiveSamples,
                                            vector<Sample *> &pNegativeSamples,
                                            unsigned short pViewCount,
                                            unsigned short pFeatureSets[],
                                            bool pWriteZME)
{
 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
 
 ofstream strmZME;
 if (pWriteZME)
  strmZME.open(ZME_TRAINING_SAMPLES_FILE);
    
 for (vector<Sentence *>::iterator itSentence = pSentences.begin(); 
      itSentence < pSentences.end(); 
      itSentence++)
 {  
  if (reachedMaxPositive() && reachedMaxNegative())
  {
   break;
   }
  
  if ((*itSentence)->generateLabeledSamples(this, pPositiveSamples, 
                                            pNegativeSamples,
                                            pViewCount,
                                            pFeatureSets,
                                            strmZME) !=0 )
   return -1;
  
  sgAnalysis->incSentenceCount();
  cout << "\r" << sgAnalysis->getSentenceCount() << " sentences processed" << flush;
  }
  
 if (strmZME.is_open()) 
  strmZME.close(); 

 vEndClk = clock();
 cpuUsage = ((double) (vEndClk - vStartClk)) / CLOCKS_PER_SEC;
 time(&vEndTime);
 elapsedTime = difftime(vEndTime, vStartTime);

 return 0;
}

void LabeledSampleGenerator::incPropCount()
{
 sgAnalysis->incPropCount();
}

void LabeledSampleGenerator::incPositiveCount()
{
 sgAnalysis->incPositiveCount();
}

void LabeledSampleGenerator::incNegativeCount()
{
 sgAnalysis->incNegativeCount();
}

bool LabeledSampleGenerator::reachedMaxPositive()
{
 // If no maximum for positive samples is set, returns false to allow 
 // unlimited number of positive samples. Otherwise, finds if it's reached.
 if (maxPositive == 0)
  return false;
 else 
  return ((maxPositive - sgAnalysis->getPositiveCount()) <= 0);
}

bool LabeledSampleGenerator::reachedMaxNegative()
{
 // If no maximum for negative samples is set, returns false to allow 
 // unlimited number of negative samples. Otherwise, finds if it's reached.
 if (maxNegative == 0)
  return false;
 else 
  return ((maxNegative - sgAnalysis->getNegativeCount()) <= 0);
}

void LabeledSampleGenerator::addArgXPMissed(Argument * const &pArg)
{
 sgAnalysis->addArgXPMissed(pArg);
}

double LabeledSampleGenerator::getElapsedTime()
{
 return elapsedTime;
}

/**
 *  Logs the result of analysis of generated samples
 */
void LabeledSampleGenerator::logSGAnalysis(string &pDatasetID, bool pLogXPMissed)
{
 sgAnalysis->logSGAnalysis(cpuUsage, elapsedTime, pDatasetID, pLogXPMissed);
}


/**
 *   UnlabeledSampleGenerator class definitions
 */

UnlabeledSampleGenerator::UnlabeledSampleGenerator()
{
 sgAnalysis = new UnlabeledSGAnalysis;
}

UnlabeledSampleGenerator::~UnlabeledSampleGenerator()
{
 delete sgAnalysis;
}

/**
 *  iterates sentences to generate samples for each sentence (generating
 *  samples and extracting features for each sample) and collects some 
 *  analysis of the generation along the way
 * 
 *  If the value of pWriteZME is true, generated samples will be written
 *  into a file in the format of Zhang Maximum Entropy tool
 */
int UnlabeledSampleGenerator::generateSamples(vector<Sentence *> &pSentences,
                                              vector<Sample *> &pSamples,
                                              unsigned short pViewCount,
                                              unsigned short pFeatureSets[],
                                              bool pWriteZME)
{
 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
 
 ofstream strmZME;
 if (pWriteZME)
  strmZME.open(ZME_TRAINING_SAMPLES_FILE);
    
 for (vector<Sentence *>::iterator itSentence = pSentences.begin(); 
      itSentence < pSentences.end(); 
      itSentence++)
 {  
  if ((*itSentence)->generateUnlabeledSamples(this, pSamples, pViewCount, 
                                              pFeatureSets, strmZME) !=0 )
   return -1;

  if ((*itSentence)->getSampleCount() == 0)
  {
   pSentences.erase(itSentence);
   itSentence--;
   } 

  sgAnalysis->incSentenceCount();
  cout << "\r" << sgAnalysis->getSentenceCount() << " sentences processed" << flush;
  }
  
 if (strmZME.is_open()) 
  strmZME.close(); 

 vEndClk = clock();
 cpuUsage = ((double) (vEndClk - vStartClk)) / CLOCKS_PER_SEC;
 time(&vEndTime);
 elapsedTime = difftime(vEndTime, vStartTime);

 return 0;
}

void UnlabeledSampleGenerator::incPropCount()
{
 sgAnalysis->incPropCount();
}

void UnlabeledSampleGenerator::incSampleCount()
{
 sgAnalysis->incSampleCount();
}

double UnlabeledSampleGenerator::getElapsedTime()
{
 return elapsedTime;
}

/**
 *  Logs the result of analysis of generated samples
 */
void UnlabeledSampleGenerator::logSGAnalysis(string &pDatasetID)
{
 sgAnalysis->logSGAnalysis(cpuUsage, elapsedTime, pDatasetID);
}


/**
 *   TestSampleGenerator class definitions
 */

TestSampleGenerator::TestSampleGenerator()
{
 sgAnalysis = new TestSGAnalysis;
}

TestSampleGenerator::~TestSampleGenerator()
{
 delete sgAnalysis;
}

/**
 *  iterates sentences to generate samples for each sentence (generating
 *  samples and extracting features for each sample) and collects some 
 *  analysis of the generation along the way
 * 
 *  If the value of pWriteZME is true, generated samples will be written
 *  into a file in the format of Zhang Maximum Entropy tool
 */
int TestSampleGenerator::generateSamples(vector<Sentence *> &pSentences,
                                     vector<Sample *> &pSamples,
                                     unsigned short pViewCount,
                                     unsigned short pFeatureSets[],
                                     bool pWriteZME)
{
 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
 
 ofstream strmZME;
 if (pWriteZME)
  strmZME.open(ZME_TEST_SAMPLES_FILE);
    
 for (vector<Sentence *>::iterator itSentence = pSentences.begin(); 
      itSentence < pSentences.end(); 
      itSentence++)
 {  
  if ((*itSentence)->generateTestSamples(this, pSamples, pViewCount, pFeatureSets, strmZME) !=0 )
   return -1;

  sgAnalysis->incSentenceCount();
  cout << "\r" << sgAnalysis->getSentenceCount() << " sentences processed" << flush;
  }
  
 if (strmZME.is_open()) 
  strmZME.close(); 

 vEndClk = clock();
 cpuUsage = ((double) (vEndClk - vStartClk)) / CLOCKS_PER_SEC;
 time(&vEndTime);
 elapsedTime = difftime(vEndTime, vStartTime);

 return 0;
}

void TestSampleGenerator::incPropCount()
{
 sgAnalysis->incPropCount();
}

void TestSampleGenerator::incSampleCount()
{
 sgAnalysis->incSampleCount();
}

void TestSampleGenerator::addArgXPMissed(Argument * const &pArg)
{
 sgAnalysis->addArgXPMissed(pArg);
}

double TestSampleGenerator::getElapsedTime()
{
 return elapsedTime;
}

/**
 *  Logs the result of analysis of generated samples
 */
void TestSampleGenerator::logSGAnalysis(string &pDatasetID, bool pLogXPMissed)
{
 sgAnalysis->logSGAnalysis(cpuUsage, elapsedTime, pDatasetID, pLogXPMissed);
}




/**
 * 
 *   definitions of sample generation analysis classes
 * 
 */

/**
 *   LabeledSGAnalysis class definitions
 */

LabeledSGAnalysis::LabeledSGAnalysis()
{
 sentenceCount = 0;
 propCount = 0;
 positiveCount = 0;
 negativeCount = 0;
}

void LabeledSGAnalysis::incSentenceCount()
{
 sentenceCount++;
}

int LabeledSGAnalysis::getSentenceCount()
{
 return sentenceCount;
}

void LabeledSGAnalysis::incPropCount()
{
 propCount++;
}

void LabeledSGAnalysis::incPositiveCount()
{
 positiveCount++;
}

int LabeledSGAnalysis::getPositiveCount()
{
 return positiveCount;
}

void LabeledSGAnalysis::incNegativeCount()
{
 negativeCount++;
}

int LabeledSGAnalysis::getNegativeCount()
{
 return negativeCount;
}

void LabeledSGAnalysis::addArgXPMissed(Argument * const &pArg)
{
 argsXPMissed.push_back(pArg);
}

/**
 *  Logs the result of analysis of generated samples
 */
void LabeledSGAnalysis::logSGAnalysis(const double &pCPUUsage, 
                                      const double &pElapsedTime,
                                      string &pDatasetID,
                                      bool pLogXPMissed)
{
 string tmpLogFileName = DEFAULT_LOG_PATH;
 tmpLogFileName += "/sg.analysis." + extractFileName(pDatasetID, true) + ".log";
 ofstream strmLog (tmpLogFileName.c_str());
 if(strmLog.is_open())
 {
  strmLog << "Processed:" << endl
          << endl
          << sentenceCount << " senetnces" << endl
          << propCount << " propositions" << endl
          << endl << endl
          << "Generated in " << pCPUUsage << "/" << pElapsedTime << " seconds: " << endl
          << endl
          << positiveCount << " positive samples" << endl
          << negativeCount << " negative samples: " << endl
          << endl;

  strmLog << argsXPMissed.size() 
          << " argument(s) were missed by XP heuristic" << endl
          << endl;
  
  if (pLogXPMissed)        
   logArgsXPMissed(strmLog);
   
  strmLog << endl;

  strmLog.close(); 
  }
}

void LabeledSGAnalysis::logArgsXPMissed(ostream &pStream)
{
 Sentence *vLastSntnc = NULL;
 Sentence *vCurrSntnc = NULL;
 
 for (vector<Argument *>::iterator itArg = argsXPMissed.begin(); 
      itArg < argsXPMissed.end(); 
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


/**
 *   UnlabeledSGAnalysis class definitions
 */

UnlabeledSGAnalysis::UnlabeledSGAnalysis()
{
 sentenceCount = 0;
 propCount = 0;
 sampleCount = 0;
}

void UnlabeledSGAnalysis::incSentenceCount()
{
 sentenceCount++;
}

int UnlabeledSGAnalysis::getSentenceCount()
{
 return sentenceCount;
}

void UnlabeledSGAnalysis::incPropCount()
{
 propCount++;
}

void UnlabeledSGAnalysis::incSampleCount()
{
 sampleCount++;
}

int UnlabeledSGAnalysis::getSampleCount()
{
 return sampleCount;
}

/**
 *  Logs the result of analysis of generated samples
 */
void UnlabeledSGAnalysis::logSGAnalysis(const double &pCPUUsage, 
                                        const double &pElapsedTime,
                                        string &pDatasetID)
{
 string tmpLogFileName = DEFAULT_LOG_PATH;
 tmpLogFileName += "/sg.analysis." + extractFileName(pDatasetID, true) + ".log";
 ofstream strmLog (tmpLogFileName.c_str());
 if(strmLog.is_open())
 {
  strmLog << "Processed:" << endl
          << endl
          << sentenceCount << " senetnces" << endl
          << propCount << " propositions" << endl
          << endl << endl
          << "Generated in " << pCPUUsage << "/" << pElapsedTime << " seconds: " << endl
          << endl
          << sampleCount << " samples" << endl
          << endl << endl;

  strmLog.close(); 
  }
}



/**
 *   TestSGAnalysis class definitions
 */

TestSGAnalysis::TestSGAnalysis()
{
 sentenceCount = 0;
 propCount = 0;
 sampleCount = 0;
}

void TestSGAnalysis::incSentenceCount()
{
 sentenceCount++;
}

int TestSGAnalysis::getSentenceCount()
{
 return sentenceCount;
}

void TestSGAnalysis::incPropCount()
{
 propCount++;
}

void TestSGAnalysis::incSampleCount()
{
 sampleCount++;
}

int TestSGAnalysis::getSampleCount()
{
 return sampleCount;
}

void TestSGAnalysis::addArgXPMissed(Argument * const &pArg)
{
 argsXPMissed.push_back(pArg);
}

/**
 *  Logs the result of analysis of generated samples
 */
void TestSGAnalysis::logSGAnalysis(const double &pCPUUsage, 
                                   const double &pElapsedTime,
                                   string &pDatasetID,
                                   bool pLogXPMissed)
{
 string tmpLogFileName = DEFAULT_LOG_PATH;
 tmpLogFileName += "/sg.analysis." + extractFileName(pDatasetID, true) + ".log";
 ofstream strmLog (tmpLogFileName.c_str());
 if(strmLog.is_open())
 {
  strmLog << "Processed:" << endl
          << endl
          << sentenceCount << " senetnces" << endl
          << propCount << " propositions" << endl
          << endl << endl
          << "Generated in " << pCPUUsage << "/" << pElapsedTime << " seconds: " << endl
          << endl
          << sampleCount << " samples" << endl
          << endl;

  strmLog << argsXPMissed.size() 
          << " argument(s) were missed by XP heuristic" << endl
          << endl;

  if (pLogXPMissed)
   logArgsXPMissed(strmLog);

  strmLog << endl;

  strmLog.close(); 
  }
}

void TestSGAnalysis::logArgsXPMissed(ostream &pStream)
{
 Sentence *vLastSntnc = NULL;
 Sentence *vCurrSntnc = NULL;
 
 for (vector<Argument *>::iterator itArg = argsXPMissed.begin(); 
      itArg < argsXPMissed.end(); 
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
