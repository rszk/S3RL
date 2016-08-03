/**
 *  @ RSK
 *  @ Version 0.1        01.05.2010
 * 
 *  Excutable file to train and test with incremental amount of labeled
 *  training data to obtain the related learning curve
 * 
 */

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "CoNLLLoader.hpp"
#include "CoNLLSaver.hpp"
#include "Sentence.hpp"
#include "SampleGenerator.hpp"
#include "Sample.hpp"
#include "Utils.hpp"
#include "Constants.hpp"
#include "Classifier.hpp"

using namespace std;


void extractSizeSeri(string &, vector<unsigned> &);
void logSizeSeri(vector<unsigned int> &, ofstream &); 
short loadCoNLLSentences(vector<Sentence *> &, string &, string &, 
                         string &, string &, ofstream &);
void shuffleSentences(vector<Sentence *> &, unsigned short); 
short generateLabeledSamples(vector<Sentence *> &, unsigned short, 
                             string &, bool, bool, ofstream &);
short generateTestSamples(vector<Sentence *> &, unsigned short, 
                          string &, bool, bool, ofstream &);
short saveCoNLLProps(vector<Sentence *> &, string, ofstream &);
void logNegatives(vector<Sample *> &);
void logPositives(vector<Sample *> &);
void logSamples(vector<Sample *> &);

static void usage(const char * execName)
{
 cout << "Usage: " 
      << execName << " [the file containing input parameters]" << endl << endl
      << "possible parameters: (each in a new line in the parameter file)" << endl << endl
      << " <-ss[The labeled sentences size seri; e.g. 50;100-1000,100;2000 means: " << endl
      << "                                       1st step 50 sentences" << endl
      << "                                       next steps starts from 100 to 1000 by steps of 100" << endl
      << "                                       last step 2000 sentences]>" << endl 
      << " <-shf[Number of times to shuffle the labeled data (the most reliable way found to randomize in C++)]" << endl
      << " <-fs[Feature set to use]>" << endl
      << " <-go[Global optimization (0: no; 1: yes)]>" << endl 
      << " <-mi[ME iteration number]>" << endl 
      << " <-mp[ME parameter estimation method (lbfgs or gis)]>" << endl 
      << " <-g[Gaussian parameter]>" << endl 
      << " <-tst[Testing frameworks (1: development; 2: development & wsj; 3: development, wsj, brown]>" << endl
      << " <-ltw[CoNLL training words file name]>" << endl
      << " <-lts[CoNLL training cfg syntax file name]>" << endl
      << " <-ltd[CoNLL training dependency syntax file name]>" << endl
      << " <-ltp[CoNLL training propositions file name]>" << endl
      << " <-twsjw[CoNLL wsj test words file name]>" << endl
      << " <-twsjs[CoNLL wsj test cfg syntax file name]>" << endl
      << " <-twsjd[CoNLL wsj test dependency syntax file name]>" << endl
      << " <-twsjp[CoNLL wsj test propositions file name]>" << endl
      << " <-dwsjw[CoNLL development words file name]>" << endl
      << " <-dwsjs[CoNLL development cfg syntax file name]>" << endl
      << " <-dwsjd[CoNLL development dependency syntax file name]>" << endl
      << " <-dwsjp[CoNLL development propositions file name]>" << endl
      << " <-tbrww[CoNLL Brown test words file name]>" << endl
      << " <-tbrws[CoNLL Brown test cfg syntax file name]>" << endl
      << " <-tbrwd[CoNLL Brown test dependency syntax file name]>" << endl
      << " <-tbrwp[CoNLL Brown test propositions file name]>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 // setting defaults for input executable arguments
 
 string vLabeledPath = DEFAULT_CONLL_LABELED_PATH;
 string vLabeledWordsFile = vLabeledPath + "/" + DEFAULT_CONLL_TRAIN_WORDS_NAME;
 string vLabeledSyntChaFile = vLabeledPath + "/" + DEFAULT_CONLL_TRAIN_SYNT_CHA_NAME;
 string vLabeledSyntDepFile = vLabeledPath + "/" + DEFAULT_CONLL_TRAIN_SYNT_DEP_NAME;
 string vLabeledPropsFile = vLabeledPath + "/" + DEFAULT_CONLL_TRAIN_PROPS_NAME;

 string vTestPath = DEFAULT_CONLL_TEST_DATA_PATH;
 string vTestWSJWordsFile = vTestPath + "/" + DEFAULT_CONLL_TEST_WSJ_WORDS_NAME;
 string vTestWSJSyntChaFile = vTestPath + "/" + DEFAULT_CONLL_TEST_WSJ_SYNT_CHA_NAME;
 string vTestWSJSyntDepFile = vTestPath + "/" + DEFAULT_CONLL_TEST_WSJ_SYNT_DEP_NAME;
 string vTestWSJPropsFile = vTestPath + "/" + DEFAULT_CONLL_TEST_WSJ_PROPS_NAME;
 string vDevWSJWordsFile = vTestPath + "/" + DEFAULT_CONLL_DEV_WSJ_WORDS_NAME;
 string vDevWSJSyntChaFile = vTestPath + "/" + DEFAULT_CONLL_DEV_WSJ_SYNT_CHA_NAME;
 string vDevWSJSyntDepFile = vTestPath + "/" + DEFAULT_CONLL_DEV_WSJ_SYNT_DEP_NAME;
 string vDevWSJPropsFile = vTestPath + "/" + DEFAULT_CONLL_DEV_WSJ_PROPS_NAME;
 string vTestBrownWordsFile = vTestPath + "/" + DEFAULT_CONLL_TEST_BROWN_WORDS_NAME;
 string vTestBrownSyntChaFile = vTestPath + "/" + DEFAULT_CONLL_TEST_BROWN_SYNT_CHA_NAME;
 string vTestBrownSyntDepFile = vTestPath + "/" + DEFAULT_CONLL_TEST_BROWN_SYNT_DEP_NAME;
 string vTestBrownPropsFile = vTestPath + "/" + DEFAULT_CONLL_TEST_BROWN_PROPS_NAME;
 
 string vOutputPath = DEFAULT_CONLL_TEST_OUTPUT_PATH;
 string vTestWSJOutput = vOutputPath + "/" + DEFAULT_CONLL_TEST_WSJ_PROPS_NAME;
 string vDevWSJOutput = vOutputPath + "/" + DEFAULT_CONLL_DEV_WSJ_PROPS_NAME;
 string vTestBrownOutput = vOutputPath + "/" + DEFAULT_CONLL_TEST_BROWN_PROPS_NAME;
 unsigned short vTesting = 1;
 
 vector<unsigned int> vSizeSeri;
 unsigned short vShfl = 0;
 unsigned short vFeatureSet = 1;
 bool vGlobalOpt = false;
 unsigned int vMEIterations = 350;
 string vMEPEMethod = "lbfgs";
 double vGaussian = 1;

 const string ctmpStr  = "help";
 if(argv[1] == ctmpStr.c_str())
 {
  usage(argv[0]);
  exit(-1);
  }
 
 string tmpLogFile = LOG_LEARNING_CURVE_OUTPUT_FILE_PREFIX;
 tmpLogFile += ".[" + getLocalTimeStr("YYYY-MM-DD-HH-MM") + "].log";
 ofstream log (tmpLogFile.c_str());
 if (!log)
  cerr << "Cannot create output log file: " << tmpLogFile
       << endl; 

 if (argc < 1)
 {
  cerr << "No input parameter file is specified!" << endl;
  exit(-1);
  }
 
 string vParamFile = argv[1];
 
 /**
  *  reading parameters
  */
 
 ifstream strmParams (vParamFile.c_str());
     
 if(!strmParams)
 {
  cerr << "Can't open file: " << vParamFile << endl;
  return false;
  }

 string vParam;
 vector<string> vcParam;
 
 while (!strmParams.eof())
 {
  getline(strmParams, vParam);
  
  // treating # as comment sign
  if (!vParam.empty() && (vParam[0] == '#'))
   continue; 
  
  vcParam.clear();
  tokenize(vParam, vcParam, " \t");
  
  if (vcParam.size() != 0)
   log << vParam << endl;

  if (vcParam[0] == "-ss")
  { 
   extractSizeSeri(vcParam[1], vSizeSeri);
   continue;
   }
   
  if (vcParam[0] == "-go")
  { 
   vGlobalOpt = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-fs")
  { 
   vFeatureSet = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-mi")
  { 
   vMEIterations = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-mp")
  { 
   vMEPEMethod = vcParam[1];
   continue; 
   }
   
  if (vcParam[0] == "-g")
  { 
   vGaussian = strToFloat(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-tst")
  { 
   vTesting = strToInt(vcParam[1]);
   continue; 
   }

  if (vcParam[0] == "-ltw")
  { 
   vLabeledWordsFile = vLabeledPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-lts")
  { 
   vLabeledSyntChaFile = vLabeledPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-ltd")
  { 
   vLabeledSyntDepFile = vLabeledPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-ltp")
  { 
   vLabeledPropsFile = vLabeledPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-twsjw")
  { 
   vTestWSJWordsFile = vTestPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-twsjs")
  { 
   vTestWSJSyntChaFile = vTestPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-twsjd")
  { 
   vTestWSJSyntDepFile = vTestPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-twsjp")
  { 
   vTestWSJPropsFile = vTestPath + "/" + vcParam[1];
   continue; 
   }
 
  if (vcParam[0] == "-dwsjw")
  { 
   vDevWSJWordsFile = vTestPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-dwsjs")
  { 
   vDevWSJSyntChaFile = vTestPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-dwsjd")
  { 
   vDevWSJSyntDepFile = vTestPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-dwsjp")
  { 
   vDevWSJPropsFile = vTestPath + "/" + vcParam[1];
   continue; 
   }
  
  if (vcParam[0] == "-tbrww")
  { 
   vTestBrownWordsFile = vTestPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-tbrws")
  { 
   vTestBrownSyntChaFile = vTestPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-tbrwd")
  { 
   vTestBrownSyntDepFile = vTestPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-tbrwp")
  { 
   vTestBrownPropsFile = vTestPath + "/" + vcParam[1];
   continue; 
   }
  }

  
 logSizeSeri(vSizeSeri, log); 

/**
 *   loading CoNLL labeled training data
 */
 
 vector<Sentence *> vcLabeledSntncs;

 mout("\nLoading CoNLL labeled training data ...\n", cout, log);
 if (loadCoNLLSentences(vcLabeledSntncs, vLabeledWordsFile, 
                        vLabeledSyntChaFile, vLabeledSyntDepFile,
                        vLabeledPropsFile, log) != 0)
  return -1;

 if (vShfl > 0)
 {
  mout("\nShuffling loaded sentences ...\n", cout, log);
  shuffleSentences(vcLabeledSntncs, vShfl); 
  }


/**
 *   loading CoNLL development and test data based on user options
 */
  
 vector<Sentence *> vcDevWSJSntncs;
 vector<Sentence *> vcTestWSJSntncs;
 vector<Sentence *> vcTestBrownSntncs;

 if (vTesting >= 1)
 {
  mout("\nLoading CoNLL WSJ development data ...\n", cout, log);
  if (loadCoNLLSentences(vcDevWSJSntncs, vDevWSJWordsFile, vDevWSJSyntChaFile, 
                         vDevWSJSyntDepFile, vDevWSJPropsFile, log) != 0)
   return -1;
  }

 if (vTesting >= 2)
 {
  mout("\nLoading CoNLL WSJ test data ...\n", cout, log);
  if (loadCoNLLSentences(vcTestWSJSntncs, vTestWSJWordsFile, vTestWSJSyntChaFile,
                         vTestWSJSyntDepFile, vTestWSJPropsFile, log) != 0)
   return -1;
  }
  
 if (vTesting >= 3)
 {
  mout("\nLoading CoNLL Brown test data ...\n", cout, log);
  if (loadCoNLLSentences(vcTestBrownSntncs, vTestBrownWordsFile, vTestBrownSyntChaFile,
                         vTestBrownSyntDepFile, vTestBrownPropsFile, log) != 0)
   return -1;
  }
  
/**
 *  generating training samples from labeled sentences for base classifier
 */

 if (generateLabeledSamples(vcLabeledSntncs, vFeatureSet, vLabeledWordsFile, true, false, log) != 0)
  return -1;


/**
 *  generating test samples to test base and self-trained classifiers
 */

 if (vTesting >= 1)
 {
  mout("\nGenerating WSJ development samples ...\n", cout, log);
  if (generateTestSamples(vcDevWSJSntncs, vFeatureSet, vDevWSJWordsFile, true, false, log) != 0)
   return -1;
  }
  
 if (vTesting >= 2)
 {
  mout("\nGenerating WSJ test samples ...\n", cout, log);
  if (generateTestSamples(vcTestWSJSntncs, vFeatureSet, vTestWSJWordsFile, true, false, log) != 0)
   return -1;
  }
  
 if (vTesting >= 3)
 {
  mout("\nGenerating Brown test samples ...\n", cout, log);
  if (generateTestSamples(vcTestBrownSntncs, vFeatureSet, vTestBrownWordsFile, true, false, log) != 0)
   return -1;
  }
  

/**
 *  training and testing the classifiers
 * 
 *  Trained model file and labeled test files are saved with an extention
 *  indicating the training size.
 */
 
 ZMEClassifier zmeClassifier;
 
 // training with specified sizes of labeled data

 for (unsigned i = 0; i < vSizeSeri.size(); i++)
 {
 
  mout("\nTraining the classifier with " + intToStr(vSizeSeri[i]) + " sentences (" + getLocalTimeStr() + ") ...\n", cout, log);
 
  vector<Sentence *> vcTrainingSntncs;
  vcTrainingSntncs.assign(vcLabeledSntncs.begin(), vcLabeledSntncs.begin() + vSizeSeri[i]);
  zmeClassifier.train(vcTrainingSntncs, 1, 1, true,
                      1, vMEIterations, vMEPEMethod, vGaussian, false, 
                      ZME_TRAINING_MODEL_FILE, log);
  mout("\nTraining is done! (" +
       floatToStr(zmeClassifier.getElapsedTime()) + " sec)\n",
       cout, log);

  if (vTesting >= 1)
  {
   mout("\nLabeling development data with the classifier of " + intToStr(vSizeSeri[i]) + " sentences ...\n", cout, log);
   zmeClassifier.classify(vcDevWSJSntncs, 1, vGlobalOpt, log);
   mout("Labeling development data is done!\n", cout, log);

   mout("\nSaving labeled development props into CoNLL file ...\n", cout, log);
   if (saveCoNLLProps(vcDevWSJSntncs, vDevWSJOutput + "." + intToStr(vSizeSeri[i]), log) != 0)
    return -1;
   }

  if (vTesting >= 2)
  {
   mout("\nLabeling WSJ test data with the classifier of " + intToStr(vSizeSeri[i]) + " sentences ...\n", cout, log);
   zmeClassifier.classify(vcTestWSJSntncs, 1, vGlobalOpt, log);
   mout("Labeling WSJ test data is done!\n", cout, log);

   mout("\nSaving labeled WSJ test props into CoNLL file ...\n", cout, log);
   if (saveCoNLLProps(vcTestWSJSntncs, vTestWSJOutput + "." + intToStr(vSizeSeri[i]), log) != 0)
    return -1;
   }     

  if (vTesting >= 3)
  {
   mout("\nLabeling Brown test data with the classifier of " + intToStr(vSizeSeri[i]) + " sentences ...\n", cout, log);
   zmeClassifier.classify(vcTestBrownSntncs, 1, vGlobalOpt, log);
   mout("Labeling Brown test data is done!\n", cout, log); 

   mout("\nSaving labeled Brown test props into CoNLL file ...\n", cout, log);
   if (saveCoNLLProps(vcTestBrownSntncs, vTestBrownOutput + "." + intToStr(vSizeSeri[i]), log) != 0)
    return -1;
   }
  } 

 // training with all labeled data

 mout("\nTraining the classifier with " + intToStr(vcLabeledSntncs.size()) + " sentences (" + getLocalTimeStr() + ") ...\n", cout, log);
 
 vector<Sentence *> vcTrainingSntncs;
 zmeClassifier.train(vcLabeledSntncs, 1, 1, true,
                     1, vMEIterations, vMEPEMethod, vGaussian, false, 
                     ZME_TRAINING_MODEL_FILE + '.' + intToStr(vcLabeledSntncs.size()), log);
 mout("\nTraining is done! (" +
      floatToStr(zmeClassifier.getElapsedTime()) + " sec)\n",
      cout, log);

 if (vTesting >= 1)
 {
  mout("\nLabeling development data with the classifier of " + intToStr(vcLabeledSntncs.size()) + " sentences ...\n", cout, log);
  zmeClassifier.classify(vcDevWSJSntncs, 1, vGlobalOpt, log);
  mout("Labeling development data is done!\n", cout, log);

  mout("\nSaving labeled development props into CoNLL file ...\n", cout, log);
  if (saveCoNLLProps(vcDevWSJSntncs, vDevWSJOutput + "." + intToStr(vcLabeledSntncs.size()), log) != 0)
   return -1;
  }

 if (vTesting >= 2)
 {
  mout("\nLabeling WSJ test data with the classifier of " + intToStr(vcLabeledSntncs.size()) + " sentences ...\n", cout, log);
  zmeClassifier.classify(vcTestWSJSntncs, 1, vGlobalOpt, log);
  mout("Labeling WSJ test data is done!\n", cout, log);

  mout("\nSaving labeled WSJ test props into CoNLL file ...\n", cout, log);
  if (saveCoNLLProps(vcTestWSJSntncs, vTestWSJOutput + "." + intToStr(vcLabeledSntncs.size()), log) != 0)
   return -1;
  }     

 if (vTesting >= 3)
 {
  mout("\nLabeling Brown test data with the classifier of " + intToStr(vcLabeledSntncs.size()) + " sentences ...\n", cout, log);
  zmeClassifier.classify(vcTestBrownSntncs, 1, vGlobalOpt, log);
  mout("Labeling Brown test data is done!\n", cout, log); 

  mout("\nSaving labeled Brown test props into CoNLL file ...\n", cout, log);
  if (saveCoNLLProps(vcTestBrownSntncs, vTestBrownOutput + "." + intToStr(vcLabeledSntncs.size()), log) != 0)
   return -1;
  }

 cout << endl;
 log.close();
}


/***********************************************************************
 * 
 *  funtion definitions
 * 
 ***********************************************************************
 */

void extractSizeSeri(string &pSeriStr, vector<unsigned> &pSeri)
{
 vector<string> vcRanges;
 vector<string> vcRange;
 
 tokenize(pSeriStr, vcRanges, " ;");
 
 for (unsigned i = 0; i < vcRanges.size(); i++)
 {
  vcRange.clear();
  tokenize(vcRanges[i], vcRange, " -,");
  
  unsigned vRangeStart = strToInt(vcRange[0]);
  pSeri.push_back(vRangeStart);
  
  if (vcRange.size() > 1)
  {
   unsigned vRangeEnd = strToInt(vcRange[1]);
   unsigned vStep = strToInt(vcRange[2]);
   unsigned vNextItem = vRangeStart + vStep;
   
   while (vNextItem <= vRangeEnd)
   {
    pSeri.push_back(vNextItem);
    
    vNextItem += vStep;
    }
   } 
  }
 
}

///---------------------------------------------------------------------
void logSizeSeri(vector<unsigned int> &pSizeSeri, ofstream &strmLog)
{
 strmLog << endl;
 
 for (unsigned i = 0; i < pSizeSeri.size(); i++)
 {
  strmLog << pSizeSeri[i] << ", ";
  cout << pSizeSeri[i] << ", ";
  }

 strmLog << "all labeled data" << endl;
 cout << "all labeled data" << endl;
}

///---------------------------------------------------------------------
void logPositives(vector<Sample *> &pSamples)
{
 ofstream strmLog (LOG_POSITIVE_SAMPLES_FILE);
 if(strmLog.is_open())
 {
  for (vector<Sample *>::iterator itSample = pSamples.begin(); 
      itSample < pSamples.end(); 
      itSample++)
  {
   strmLog << (*itSample)->getProp()->getPredLemma() << ":"
           << " [" << (*itSample)->getWordsStr() << "]"
           << " [" << (*itSample)->getGoldArgLabel() << "]"
           << " [" << (*itSample)->getZMEContextStr(1) << "]"
           << endl;
   }
  strmLog.close(); 
  }
}

///---------------------------------------------------------------------
void logNegatives(vector<Sample *> &pSamples)
{
 ofstream strmLog (LOG_NEGATIVE_SAMPLES_FILE);
 if(strmLog.is_open())
 {
  for (vector<Sample *>::iterator itSample = pSamples.begin(); 
      itSample < pSamples.end(); 
      itSample++)
  {
   strmLog << (*itSample)->getProp()->getPredLemma() << ":"
           << " [" << (*itSample)->getWordsStr() << "]"
           << " [" << (*itSample)->getZMEContextStr(1) << "]"
           << endl;
   }
  strmLog.close(); 
  }
}

///---------------------------------------------------------------------
void logSamples(vector<Sample *> &pSamples)
{
 ofstream strmLog (LOG_TEST_SAMPLES_FILE);
 int cntSample = 1; 

 if(strmLog.is_open())
 {
  for (vector<Sample *>::iterator itSample = pSamples.begin();
       itSample < pSamples.end(); 
       itSample++, cntSample++)
  {
   strmLog << (*itSample)->getProp()->getPredLemma() << ":"
           << " [" << (*itSample)->getWordsStr() << "]"
           << " [" << (*itSample)->getZMEContextStr(1) << "]"
           << endl;
 
   cout << "\r" << cntSample << " samples logged" << flush;
   }
  strmLog.close(); 
  }
}

///---------------------------------------------------------------------
short loadCoNLLSentences(vector<Sentence *> &pSentences, string &pWordsFile, 
                         string &pSyntFile, string &pDepFile, string &pPropsFile, 
                         ofstream &pLog)
{
 CoNLLLoader oCoNLLLoader(pWordsFile, 
                          pSyntFile,
                          pDepFile,
                          pPropsFile, 
                          0);

 if (oCoNLLLoader.loadData(pSentences) == 0)
 {
  oCoNLLLoader.logLoadedData();
  oCoNLLLoader.logDataAnalysis();

  pLog << pSentences.size() << " sentences loaded" << flush;
  mout("\nLoading CoNLL data is done! (" +
       floatToStr(oCoNLLLoader.getElapsedTime()) + " sec)\n",
       cout, pLog);

  return 0;
  }
 else
 {
  pLog << pSentences.size() << " sentences loaded" << flush;
  mout("\nLoading data was not successful\n", cerr, pLog);
  return -1;
  }
}


///---------------------------------------------------------------------
/**
 *  shuffling sentences to randomize the data for selection in each learning 
 *  iteration 
 */
void shuffleSentences(vector<Sentence *> &pSentences, unsigned short pShflNo)
{
 for (unsigned short i = 1; i <= pShflNo; i++)
  random_shuffle(pSentences.begin(), pSentences.end());
}


///---------------------------------------------------------------------
/**
 *  generate training samples from labeled sentences for base classifier
 *  (Samples can be written into a file in ZME format optionally by setting
 *  the pWriteToFile to true)
 */
short generateLabeledSamples(vector<Sentence *> &pSentences,
                             unsigned short pFeatureSet,
                             string &pDatasetID,
                             bool pWriteToFile,
                             bool pLogSamples,
                             ofstream &pLog)                                     
{                                 
 mout("\nGenerating labeled training samples ...\n", cout, pLog);

 LabeledSampleGenerator oSampleGenerator;
 vector<Sample *> vcPositiveSamples;
 vector<Sample *> vcNegativeSamples;
 // converting pFeatureSet to an array to be a valid parameter for function
 unsigned short vFeatureSets[] = {pFeatureSet};
 if (oSampleGenerator.generateSamples(pSentences, 
                                      vcPositiveSamples,
                                      vcNegativeSamples,
                                      1, vFeatureSets,
                                      pWriteToFile) == 0)
 {
  oSampleGenerator.logSGAnalysis(pDatasetID, false);
  if (pLogSamples)
  {
   logPositives(vcPositiveSamples);
   logNegatives(vcNegativeSamples);
   }
  
  pLog << pSentences.size() << " sentences processed ("
       << vcPositiveSamples.size() + vcNegativeSamples.size()
       << " samples)" << flush;
  mout("\nGenerating labeled training samples is done! (" +
       floatToStr(oSampleGenerator.getElapsedTime()) + " sec)\n",
       cout, pLog);
  return 0;
  }
 else
 {
  pLog << pSentences.size() << " sentences processed ("
       << vcPositiveSamples.size() + vcNegativeSamples.size()
       << " samples)" << flush;
  mout("\nGenerating training samples was not successful!\n", cerr, pLog);
  return -1;
  }
}  

///---------------------------------------------------------------------
/**
 *  generate test samples
 *  (Samples can be written into a file in ZME format optionally by setting
 *  the pWriteToFile to true)
 */
short generateTestSamples(vector<Sentence *> &pSentences,
                          unsigned short pFeatureSet,  
                          string &pDatasetID,
                          bool pWriteToFile,
                          bool pLogSamples,
                          ofstream &pLog)
{                                 
 TestSampleGenerator oSampleGenerator;
 vector<Sample *> vcSamples;
 // converting pFeatureSet to an array to be a valid parameter for function
 unsigned short vFeatureSets[] = {pFeatureSet};
 if (oSampleGenerator.generateSamples(pSentences, vcSamples, 1, vFeatureSets,
                                      pWriteToFile) == 0)
 {
  oSampleGenerator.logSGAnalysis(pDatasetID, false);
  if (pLogSamples)
   logSamples(vcSamples);
 
  pLog << pSentences.size() << " sentences processed ("
       << vcSamples.size() << " samples)" << flush;
  mout("\nGenerating samples is done! (" +
       floatToStr(oSampleGenerator.getElapsedTime()) + " sec)\n",
       cout, pLog);
  return 0;
  }
 else
 {
  pLog << pSentences.size() << " sentences processed ("
       << vcSamples.size() << " samples)" << flush;
  mout("\nGenerating test samples was not successful!\n", cout, pLog);
  return -1;
  }
}  

///---------------------------------------------------------------------
/**
 *  writing the labeled samples into a CoNLL props file format to be 
 *  evaluated afterward
 */
short saveCoNLLProps(vector<Sentence *> &pLabeledSntncs,
                     string pOutputPropsFile, ofstream &pLog)
{
 CoNLLSaver oCoNLLSaver(pOutputPropsFile);
 
 if (oCoNLLSaver.saveProps(pLabeledSntncs, 1) == 0)
 {
  pLog << pLabeledSntncs.size() << " sentences processed" << flush;
  mout("\nSaving labeled props is done! (" +
       floatToStr(oCoNLLSaver.getElapsedTime()) + " sec)\n", cout, pLog);
  
  return 0;     
  }
 else
 {
  pLog << pLabeledSntncs.size() << " sentences processed" << flush;
  mout("\nSaving labeled props was not successful!\n", cerr, pLog);
  return -1;
  }
}
