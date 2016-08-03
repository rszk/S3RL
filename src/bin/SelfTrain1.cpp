/**
 *  @ RSK
 *  @ Version 1.1        17.06.2010
 * 
 *  Excutable file to self-train the classifiers with sentence-based approach;
 *  the granularity of labeling and selection of labeled data for the 
 *  set at each iteration is a sentence.
 * 
 */

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "CoNLLLoader.hpp"
#include "ULoader.hpp"
#include "CoNLLSaver.hpp"
#include "Sentence.hpp"
#include "SampleGenerator.hpp"
#include "Sample.hpp"
#include "Utils.hpp"
#include "Constants.hpp"
#include "Classifier.hpp"

using namespace std;


short loadCoNLLSentences(vector<Sentence *> &, string &, string &, string &,
                         string &, unsigned int, ofstream &);
short loadUnlabeledSentences(vector<Sentence *> &, string &, string &,
                             unsigned int, ofstream &);
short generateLabeledSamples(vector<Sentence *> &, unsigned int, unsigned int, 
                             unsigned short, string &, bool, bool, ofstream &);
short generateTestSamples(vector<Sentence *> &, unsigned short,
                          string &, bool, bool, ofstream &);
short generateUnlabeledSamples(vector<Sentence *> &, unsigned short, 
                               string &, bool, bool, ofstream &);
void selfTrain(ZMEClassifier &, vector<Sentence *> &, vector<Sentence *> &,
               vector<Sentence *> &, vector<Sentence *> &, vector<Sentence *> &,
               unsigned short, string &, double, bool, unsigned int, 
               unsigned short, unsigned short, double, unsigned short, 
               bool, unsigned short, unsigned short, unsigned short, 
               unsigned short, string &, string &, string &, ofstream &);
unsigned int addToTraining(vector<Sentence *> &, vector<Sentence *> &, 
                           unsigned int);
unsigned int moveToTraining(vector<Sentence *> &, vector<Sentence *> &, 
                            vector<Sentence *> &, unsigned, unsigned short, 
                            unsigned short, double, unsigned short,
                            unsigned short, unsigned short, string,
                            ofstream &);
unsigned int selectAndMove(vector<Sentence *> &, vector<Sentence *> &, 
                           double, unsigned short, unsigned short,
                           unsigned short, string &, ofstream &);
short saveCoNLLProps(vector<Sentence *> &, string, ofstream &);
void logNegatives(vector<Sample *> &);
void logPositives(vector<Sample *> &);
void logSamples(vector<Sample *> &);

static void usage(const char * execName)
{
 cout << "Usage: (some options cannot meet each other; e.g. pool size and explicit iteration number cannot both be non-zero" 
      << execName << " [the file containing input parameters]" << endl << endl
      << "possible parameters: (each in a new line in the parameter file)" << endl << endl
      << " <-s[Seed size]>" << endl 
      << " <-u[Unlabeled size (0: only base classifier)]>" << endl 
      << " <-mxp[Maximum positive samples to be generated and used]>" << endl 
      << " <-mxn[Maximum negative samples to be generated and used]>" << endl 
      << " <-fs[Feature set to use]>" << endl
      << " <-go[Global optimization (0: no; 1: yes)]>" << endl 
      << " <-mi[ME iteration number]>" << endl 
      << " <-mp[ME parameter estimation method (lbfgs or gis)]>" << endl 
      << " <-g[Gaussian parameter]>" << endl 
      << " <-sc[Selection criterion to use (0 if no selection)]>" << endl 
      << " <-p[Pool size (0 if not used)]>" << endl
      << " <-pu[Pool usage when selection is used (0: no pool used;" << endl
      << "						 				   1: iterate to select whole;" << endl
      << "                                         2: iterate once and remove unselected;" << endl
      << "                                         3: iterate once and return unselected to the begining of unlabeled data;" << endl
      << "                                         4: iterate once and return unselected to the end of unlabeled data)>" << endl
      << " <-pq[Quality selection for pool (0: not used; 1: shorters first; 2: mediate lengths first; 3: simplers first)]>" << endl 
      << " <-pt[Selection probability threshold (0 if no threshold)]>" << endl 
      << " <-nt[Selection number threshold (0 if no threshold)]>" << endl 
      << " <-r[Remove/Not Remove once labeled (1/0)]>" << endl 
      << " <-si[Explicit iteration number when p0, c0, r0 (0 if not used)]>" << endl 
      << " <-ls[Log labeling selection (0: don't log; 1: only selected; 2: selected and filtered)]>" << endl 
      << " <-tst[Testing frameworks (1: development; 2: development & wsj; 3: development, wsj, brown]>" << endl
      << " <-ltw[CoNLL training words file name]>" << endl
      << " <-lts[CoNLL training cfg syntax file name]>" << endl
      << " <-ltd[CoNLL training dependency syntax file name]>" << endl
      << " <-ltp[CoNLL training propositions file name]>" << endl
      << " <-utd[Unlabeled training data file name]>" << endl
      << " <-utds[Unlabeled training dependency syntax file name]>" << endl
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

 string vUnlabeledPath = DEFAULT_CONLL_UNLABELED_PATH;
 string vUnlabeledDataFile = vUnlabeledPath + "/" + DEFAULT_CONLL_UNLABELED_DATA_NAME;
 string vUnlabeledSyntDepFile = vUnlabeledPath + "/" + DEFAULT_CONLL_UNLABELED_SYNT_DEP_NAME;

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
 
 unsigned int vSeedSize = 0;
 unsigned int vUnlabeledSize = 0;
 unsigned int vMaxPositive = MAX_TRAIN_POSITIVE;
 unsigned int vMaxNegative = MAX_TRAIN_NEGATIVE;
 unsigned short vFeatureSet = 1;
 bool vGlobalOpt = false;
 unsigned int vMEIterations = 350;
 string vMEPEMethod = "lbfgs";
 double vGaussian = 1;
 unsigned int vPoolSize = 0;
 unsigned short vSelection = 0;
 unsigned short vPoolUsage = 3;
 double vProbThreshold = 0;
 unsigned short vNumberThreshold = 0;
 bool vRemoveLabeled = false;
 unsigned short vSTIterations = 0;
 unsigned short vPoolQuality = 0;
 unsigned short vLogSelection = 0;
 
 const string ctmpStr  = "help";
 if(argv[1] == ctmpStr.c_str())
 {
  usage(argv[0]);
  exit(-1);
  }
 
 string tmpLogFile = LOG_SELFTRAINING_OUTPUT_FILE_PREFIX;
 tmpLogFile += ".1.[" + getLocalTimeStr("YYYY-MM-DD-HH-MM") + "].log";
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

  if (vcParam[0] == "-s")
  { 
   vSeedSize = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-u")
  { 
   vUnlabeledSize = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-mxp")
  { 
   vMaxPositive = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-mxn")
  { 
   vMaxNegative = strToInt(vcParam[1]);
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
   
  if (vcParam[0] == "-p")
  { 
   vPoolSize = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-sc")
  { 
   vSelection = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-pu")
  { 
   vPoolUsage = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-pt")
  { 
   vProbThreshold = strToFloat(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-nt")
  { 
   vNumberThreshold = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-r")
  { 
   vRemoveLabeled = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-si")
  { 
   vSTIterations = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-pq")
  { 
   vPoolQuality = strToInt(vcParam[1]);
   continue;
   }

  if (vcParam[0] == "-ls")
  { 
   vLogSelection = strToInt(vcParam[1]);
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

  if (vcParam[0] == "-utd")
  { 
   vUnlabeledDataFile = vUnlabeledPath + "/" + vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-utds")
  { 
   vUnlabeledSyntDepFile = vUnlabeledPath + "/" + vcParam[1];
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

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


/**
 *   loading CoNLL labeled training data
 * 
 *   From entire the corpus, olny vSeedSize sentences will be loaded, 
 *   unless it is set to 0 in which case all sentences will be loaded.
 */
 
 vector<Sentence *> vcLabeledSntncs;

 mout("\nLoading CoNLL labeled training data ...\n", cout, log);
 if (loadCoNLLSentences(vcLabeledSntncs, vLabeledWordsFile, 
                        vLabeledSyntChaFile, vLabeledSyntDepFile,
                        vLabeledPropsFile, vSeedSize, log) != 0)
  return -1;

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


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
                         vDevWSJSyntDepFile, vDevWSJPropsFile, 0, log) != 0)
   return -1;
  }

 if (vTesting >= 2)
 {
  mout("\nLoading CoNLL WSJ test data ...\n", cout, log);
  if (loadCoNLLSentences(vcTestWSJSntncs, vTestWSJWordsFile, vTestWSJSyntChaFile,
                         vTestWSJSyntDepFile, vTestWSJPropsFile, 0, log) != 0)
   return -1;
  }
  
 if (vTesting >= 3)
 {
  mout("\nLoading CoNLL Brown test data ...\n", cout, log);
  if (loadCoNLLSentences(vcTestBrownSntncs, vTestBrownWordsFile, vTestBrownSyntChaFile,
                         vTestBrownSyntDepFile, vTestBrownPropsFile, 0, log) != 0)
   return -1;
  }
  
 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


/**
 *  generating training samples from labeled sentences for base classifier
 */

 if (generateLabeledSamples(vcLabeledSntncs, 
                            vMaxPositive, vMaxNegative, vFeatureSet, 
                            vLabeledWordsFile, true, false, log) != 0)
  return -1;

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


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
  
 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


/**
 *  training the base classifier
 */
 
 ZMEClassifier zmeClassifier;
 
/**
 *  here a vector of sentence for which the samples have been generated
 *  above is passed to trainer (instead of directly passing positive and
 *  negative samples themselves)
 * 
 *  The 4th parameter sets the use of gold labels in training, since this 
 *  is the base classifier
 */
 mout("\nTraining base classifier (" + getLocalTimeStr() + ") ...\n", cout, log);
 zmeClassifier.train(vcLabeledSntncs, 1, 1, true,
                     1, vMEIterations, vMEPEMethod, vGaussian, true, 
                     ZME_TRAINING_MODEL_FILE, log);
 mout("\nTraining the base classifier is done! (" +
      floatToStr(zmeClassifier.getElapsedTime()) + " sec)\n",
      cout, log);

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


/**
 *  testing the base classifier & writing the labeled samples into a CoNLL
 *  props file format to be evaluated afterward
 *  
 *  A ".0" suffix is added to the output props file name to indicate that
 *  this is the labeled data by the base classifier (iteration 0)
 * 
 *  here a vector of sentence for which the samples have been generated
 *  above is passed to test (instead of passing samples themselves)
 */
 
 if (vTesting >= 1)
 {
  mout("\nLabeling development data with base classifier ...\n", cout, log);
  zmeClassifier.classify(vcDevWSJSntncs, 1, vGlobalOpt, log);
  mout("Labeling development data is done! (" +
       floatToStr(zmeClassifier.getElapsedTime()) + " sec)\n",
       cout, log);

  mout("\nSaving labeled development props into CoNLL file ...\n", cout, log);
  if (saveCoNLLProps(vcDevWSJSntncs, vDevWSJOutput + ".0", log) != 0)
   return -1;
  }

 if (vTesting >= 2)
 {
  mout("\nLabeling WSJ test data with base classifier ...\n", cout, log);
  zmeClassifier.classify(vcTestWSJSntncs, 1, vGlobalOpt, log);
  mout("Labeling WSJ test data is done! (" +
       floatToStr(zmeClassifier.getElapsedTime()) + " sec)\n",
       cout, log);

  mout("\nSaving labeled WSJ test props into CoNLL file ...\n", cout, log);
  if (saveCoNLLProps(vcTestWSJSntncs, vTestWSJOutput + ".0", log) != 0)
   return -1;
  }     

 if (vTesting >= 3)
 {
  mout("\nLabeling Brown test data with base classifier ...\n", cout, log);
  zmeClassifier.classify(vcTestBrownSntncs, 1, vGlobalOpt, log);
  mout("Labeling Brown test data is done! (" +
       floatToStr(zmeClassifier.getElapsedTime()) + " sec)\n",
       cout, log);

  mout("\nSaving labeled Brown test props into CoNLL file ...\n", cout, log);
  if (saveCoNLLProps(vcTestBrownSntncs, vTestBrownOutput + ".0", log) != 0)
   return -1;
  }

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


/**
 *  Self-training
 */

 // if the specified size for unlabeled data is 0, ignore self-training
 if (vUnlabeledSize == 0)
 {
  cout << endl;
  log.close();
  return 0;
  }
 
 /**
  *  loading unlabeled training data
  * 
  *  From the entire corpus, only vUnlabeledSize sentences will be loaded, 
  *  unless it is set to 0 in which case all sentences will be loaded.
  */
 
 vector<Sentence *> vcUnlabeledSntncs;

 mout("\nLoading unlabeled training data ...\n", cout, log);
 if (loadUnlabeledSentences(vcUnlabeledSntncs, vUnlabeledDataFile,
                            vUnlabeledSyntDepFile, vUnlabeledSize, log) != 0)
  return -1;

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);

 /**
  *  generating unlabeled training samples for self-training
  */

 if (generateUnlabeledSamples(vcUnlabeledSntncs, vFeatureSet, vUnlabeledDataFile, false, false, log) != 0)
  return -1;

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
  
 mout("\nSelf-training started at " + getLocalTimeStr() + " ...\n", cout, log);
 selfTrain(zmeClassifier, vcLabeledSntncs, vcUnlabeledSntncs, 
           vcDevWSJSntncs, vcTestWSJSntncs, vcTestBrownSntncs,
           vMEIterations, vMEPEMethod, vGaussian, vGlobalOpt, vPoolSize,
           vSelection, vPoolUsage, vProbThreshold, vNumberThreshold, 
           vRemoveLabeled, vSTIterations, vPoolQuality, vLogSelection, 
           vTesting, vDevWSJOutput, vTestWSJOutput, vTestBrownOutput, log);
 mout("\nSelf-training is done at " + getLocalTimeStr() + "! ", cout, log);
 vEndClk = clock();
 mout("(" + floatToStr(((double) (vEndClk - vStartClk)) / (CLOCKS_PER_SEC * 60)), 
      cout, log);
 time(&vEndTime);
 mout(" min/" + floatToStr(difftime(vEndTime, vStartTime) / 60) + " min)\n",
      cout, log);

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);

 cout << endl;
 log.close();
}


/***********************************************************************
 * 
 *  funtion definitions
 * 
 ***********************************************************************
 */

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

short loadCoNLLSentences(vector<Sentence *> &pSentences, string &pWordsFile, 
                         string &pSyntFile, string &pDepFile, string &pPropsFile, 
                         unsigned int pMaxSntnc, ofstream &pLog)
{
 CoNLLLoader oCoNLLLoader(pWordsFile, 
                          pSyntFile,
                          pDepFile,
                          pPropsFile, 
                          pMaxSntnc);

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

short loadUnlabeledSentences(vector<Sentence *> &pSentences, 
                             string &pDataFile, string &pSyntDepFile, 
                             unsigned int pMaxSntnc, ofstream &pLog)
{
 ULoader oULoader(pDataFile, pSyntDepFile, pMaxSntnc);

 if (oULoader.loadData(pSentences) == 0)
 {
  oULoader.logLoadedData();

  pLog << pSentences.size() << " sentences loaded" << flush;
  mout("\nLoading unlabeled training data is done! (" +
       floatToStr(oULoader.getElapsedTime()) + " sec)\n",
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

/**
 *  generate training samples from labeled sentences for base classifier
 *  (Samples can be written into a file in ZME format optionally by setting
 *  the pWriteToFile to true)
 */

short generateLabeledSamples(vector<Sentence *> &pSentences,
                             unsigned int pMaxPositive,
                             unsigned int pMaxNegative,
                             unsigned short pFeatureSet,
                             string &pDatasetID,
                             bool pWriteToFile,
                             bool pLogSamples,
                             ofstream &pLog)                                     
{                                 
 mout("\nGenerating labeled training samples ...\n", cout, pLog);

 LabeledSampleGenerator oSampleGenerator(pMaxPositive,
                                         pMaxNegative);
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


/**
 *  generate unlabeled samples
 */
short generateUnlabeledSamples(vector<Sentence *> &pSentences,
                               unsigned short pFeatureSet,
                               string &pDatasetID,
                               bool pWriteToFile,
                               bool pLogSamples,
                               ofstream &pLog)
{                                 
 mout("\nGenerating unlabeled samples ...\n", cout, pLog);

 UnlabeledSampleGenerator oSampleGenerator;
 vector<Sample *> vcSamples;
 // converting pFeatureSet to an array to be a valid parameter for function
 unsigned short vFeatureSets[] = {pFeatureSet};
 if (oSampleGenerator.generateSamples(pSentences, vcSamples, 1, vFeatureSets,
                                      pWriteToFile) == 0)
 {
  oSampleGenerator.logSGAnalysis(pDatasetID);
  if (pLogSamples)
   logSamples(vcSamples);
 
  pLog << pSentences.size() << " sentences processed ("
       << vcSamples.size() << " samples)" << flush;
  mout("\nGenerating unlabeled samples is done! (" +
       floatToStr(oSampleGenerator.getElapsedTime()) + " sec)\n",
       cout, pLog);
  return 0;
  }
 else
 {
  pLog << pSentences.size() << " sentences processed ("
       << vcSamples.size() << " samples)" << flush;
  mout("\nGenerating unlabeled samples was not successful!\n", cerr, pLog);
  return -1;
  }
}  


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


/**
 *  self-train (the main function)
 * 
 *  For explanation of parameters, refer to the option list of the main
 *  function above
 * 
 */

void selfTrain(ZMEClassifier &pZMEClassifier,
               vector<Sentence *> &pTrainingSntncs,
               vector<Sentence *> &pUnlabeledSntncs,
               vector<Sentence *> &pDevWSJSntncs,
               vector<Sentence *> &pTestWSJSntncs,
               vector<Sentence *> &pTestBrownSntncs,
               unsigned short pMEIterations,
               string &pMEPEMethod,
               double pGaussian,
               bool pGlobalOpt,
               unsigned int pPoolSize,
               unsigned short pSelection,
               unsigned short pPoolUsage,
               double pProbThreshold,
               unsigned short pNumberThreshold,
               bool pRemoveLabeled,
               unsigned short pSTIterations,
               unsigned short pPoolQuality,
               unsigned short pLogSelection,
               unsigned short pTesting,
               string &pDevWSJOutput,
               string &pTestWSJOutput,
               string &pTestBrownOutput,
               ofstream &pLog)
{
 unsigned vSeedSize = pTrainingSntncs.size();
 unsigned vAllUSize = pUnlabeledSntncs.size();
 
 // overriding pool usage of no pool is used
 if (pPoolSize == 0)
  pPoolUsage = 0;
 
 mout("\n", cout, pLog);
 mout("Seed Size: " + intToStr(vSeedSize) + '\n', cout, pLog);
 mout("Unlabeled Size: " + intToStr(pUnlabeledSntncs.size()) + '\n', cout, pLog);
 mout("Pool Size: " + intToStr(pPoolSize) + '\n', cout, pLog);
 mout("Selection Criterion: " + intToStr(pSelection) + '\n', cout, pLog);
 mout("Pool Usage: " + intToStr(pPoolUsage) + '\n', cout, pLog);
 mout("Probability Threshold for Selection: " + floatToStr(pProbThreshold) + '\n', cout, pLog);
 mout("Number of Labeled Data for Selection: " + intToStr(pNumberThreshold) + '\n', cout, pLog);
 mout("Remove Once Labeled: " + intToStr(pRemoveLabeled) + '\n', cout, pLog);
 mout("Self-training Explicit Iteration number: " + intToStr(pSTIterations) + '\n', cout, pLog);
 mout("Pool Quality: " + intToStr(pPoolQuality) + '\n', cout, pLog);
 
 if (pPoolUsage > 0)
 {
  switch (pPoolQuality)
  {
   case (1):
    sort(pUnlabeledSntncs.begin(), pUnlabeledSntncs.end(), isShorter); 
    break;
   case (2):
    sort(pUnlabeledSntncs.begin(), pUnlabeledSntncs.end(), isMediate); 
    break;
   case (3):
     sort(pUnlabeledSntncs.begin(), pUnlabeledSntncs.end(), isSimpler); 
     break;
   }
  } 

 // if no pool is set to be used (pool size = 0) then treat whole unlabeled
 // set as one pool
 if (pPoolUsage == 0)
  pPoolSize = pUnlabeledSntncs.size();
  
 // The global iteration counter and pool counter
 unsigned short cntIteration = 1;
 unsigned short cntPool = 1;
 
 vector<Sentence *> vPool;
 // Iterating for loading pools until all unlabeled data are used.
 while (pUnlabeledSntncs.size() > 0)
 {
  
  /**
   * loading pool:
   * 
   * Unlabeled data are moved to pool; the pool may be empty (if remove
   * labeled is used and thus labeled pool is moved training set in following
   * lines) or full otherwise
   */ 

  // if the remaining unlabeled data is less than the size set for pool,
  // then set pool size to the remaining unlabeled data size:
  // - this may happen when the unlabeled data size is not dividable by 
  //   pool size (e.g. 10000 / 300)
  // - this operation is not correct when whole unlabeled data is used as 
  //   pool, but it does not make problem, as it is meet only once, so that
  //   it is not fired
  if (pUnlabeledSntncs.size() < pPoolSize)
   pPoolSize = pUnlabeledSntncs.size();

  vPool.insert(vPool.end(),
               pUnlabeledSntncs.begin(), 
               pUnlabeledSntncs.begin() + pPoolSize);
  pUnlabeledSntncs.erase(pUnlabeledSntncs.begin(), 
                         pUnlabeledSntncs.begin() + pPoolSize);
   
  // iterating to train classifiers using current pool until one of stop
  // criterion controling rules inside the loop fires (at the end of the 
  // loop)
  while (1)
  {
   mout("\nIteration " + intToStr(cntIteration) +
        " (Pool " + intToStr(cntPool) + "):\n", cout, pLog);

   /**
    *  labeling unlabeled samples (sentence-based)
    */
 
   mout("\nLabeling unlabeled samples ...\n", cout, pLog);
   unsigned vLabeledCount = pZMEClassifier.classify(vPool, 1, pGlobalOpt, pLog);
   mout("Labeling unlabeled samples is done! (" +
        floatToStr(pZMEClassifier.getElapsedTime()) + " sec)\n",
        cout, pLog);

   /**
    *  selecting and adding newly labeled data to training set
    */
 
   mout("\nAdding newly labeled data to training set ...\n", cout, pLog);
   
   unsigned int vAddedCount = 0;
   
   if (pRemoveLabeled)
    vAddedCount = moveToTraining(pTrainingSntncs, pUnlabeledSntncs, vPool,
                                 vLabeledCount, pSelection, pPoolUsage, 
                                 pProbThreshold, pNumberThreshold,
                                 DEFAULT_SELF_TRAINING_CONVERGENCE_SENTENCE_COUNT,
                                 pLogSelection, LOG_POOL_SELECTION_FILE_NAME +
                                 intToStr(cntIteration) + ".log",
                                 pLog);
   else 
    // Note:
    // When the labeled data should not be removed, selection may not 
    // be useful, since in all iterations almost the same data may be
    // selected.
    vAddedCount = addToTraining(pTrainingSntncs, vPool, vSeedSize);
    
   mout("Adding labeled data is done! (" + intToStr(vAddedCount) +
        " sentences)\n", cout, pLog);
 
   // Only if any new labeled data is added to training set, train a new
   // classifier 
   if (vAddedCount != 0)
   {
   
    /**
     *  training a new classifier with new labeled data
     * 
     *  The 4th parameter sets the use of predicted labels in training, since
     *  this is the self-trained classifier classifier
     */
  
    mout("\nTraining " + intToStr(cntIteration) + "th classifier  (" + getLocalTimeStr() + ") ...\n",
         cout, pLog);
    pZMEClassifier.train(pTrainingSntncs, 1, 1, false, 0, 
                         pMEIterations, pMEPEMethod, pGaussian, false, "", pLog);
    mout("\nTraining the " + intToStr(cntIteration) + "th classifier is done! (" +
         floatToStr(pZMEClassifier.getElapsedTime()) + " sec)\n", cout, pLog);

    /**
     *  testing the new classifier & writing the labeled samples into a 
     *  CoNLL props file format to be evaluated afterward
     *  
     *  A ".#" suffix is added to the output props file name to indicate the
     *  iteration
     */
    
    if (pTesting >= 1)
    {
     mout("\nLabeling development data with " + intToStr(cntIteration) + "th classifier ...\n",
          cout, pLog);
     pZMEClassifier.classify(pDevWSJSntncs, 1, pGlobalOpt, pLog);
     mout("Labeling development data is done! (" + floatToStr(pZMEClassifier.getElapsedTime()) +
          " sec)\n", cout, pLog); 

     mout("\nSaving labeled development props into CoNLL file ...\n", cout, pLog);
     saveCoNLLProps(pDevWSJSntncs, pDevWSJOutput + "." + intToStr(cntIteration), pLog);
     }
     
    if (pTesting >= 2)
    {
     mout("\nLabeling WSJ test data with " + intToStr(cntIteration) + "th classifier ...\n",
          cout, pLog);
     pZMEClassifier.classify(pTestWSJSntncs, 1, pGlobalOpt, pLog);
     mout("Labeling WSJ test data is done! (" + floatToStr(pZMEClassifier.getElapsedTime()) +
          " sec)\n", cout, pLog); 

     mout("\nSaving labeled WSJ test props into CoNLL file ...\n", cout, pLog);
     saveCoNLLProps(pTestWSJSntncs, pTestWSJOutput + "." + intToStr(cntIteration), pLog);
     }
     
    if (pTesting >= 3)
    {
     mout("\nLabeling Brown test data with " + intToStr(cntIteration) + "th classifier ...\n",
          cout, pLog);
     pZMEClassifier.classify(pTestBrownSntncs, 1, pGlobalOpt, pLog);
     mout("Labeling Brown test data is done! (" + floatToStr(pZMEClassifier.getElapsedTime()) +
          " sec)\n", cout, pLog); 

     mout("\nSaving labeled Brown test props into CoNLL file ...\n", cout, pLog);
     saveCoNLLProps(pTestBrownSntncs, pTestBrownOutput + "." + intToStr(cntIteration), pLog);
     }
     
    }
   
   cntIteration++; 
   
   
   /**
    *  logging the amount of data has been used so far
    *  The amount includes seed data plus the portion of unlabeled data
    *  has been removed when pRemoveLabeled is set or the size of current 
    *  pool when it is not set (and pool is used).
    *  Possible states are treated as following:
    */ 
   
   unsigned int vDataUsedSofar = vSeedSize;
   
   if (pRemoveLabeled != 0)
    vDataUsedSofar += vAllUSize - pUnlabeledSntncs.size() - vPool.size();
   else
   {
    if (pPoolUsage > 0)
     vDataUsedSofar += vPool.size();
    else
     vDataUsedSofar += pUnlabeledSntncs.size();
     }
   mout("\nData used so far: " + intToStr(vDataUsedSofar) + " sentences\n", cout, pLog);
     
  
   /**
    *  Checking stop criterion for looping inside the pool
    */

   // This is the stop criterion for the situation that pool items must be
   // removed after labeling (pRemove is set to 1)
   if (vPool.size() == 0)
    break;
   
   // This is the stop criterion for the situation that an explicit iteration
   // number is set, as it does not stop with criterion above.
   if (pSTIterations != 0)
    if (pSTIterations < cntIteration)
     break;
    
   // This is the stop criterion for the situation that the pool is used
   // but its content is not removed after labeling (incremental pool)
   if ((pPoolUsage > 0) && (pRemoveLabeled == 0))
    break;
   }
   
  cntPool++;
  }
}


/**
 *  adds all the labeled pool to the end of original seed training data
 *  i.e. replaces the predicted portion of training data with new predictions,
 *  since they are already relabeled and present in pool (no remove), then
 *  returns number of sentences added
 */ 
unsigned int addToTraining(vector<Sentence *> &pTrainingSntncs,
                           vector<Sentence *> &pPool,
                           unsigned int pSeedSize)
{                            
 if (pTrainingSntncs.size() != pSeedSize)
  pTrainingSntncs.resize(pSeedSize);
  
 pTrainingSntncs.insert(pTrainingSntncs.begin() + pSeedSize, 
                        pPool.begin(), pPool.end());
 
 return pPool.size();
}

/**
 *  moves labeled data from pool to training set (remove from pool after
 *  adding to training set)
 * 
 *  If selection has been set to be used it uses selectAndMove() to do it,
 *  and then post-process the pool and unlabeled set based on other parameters
 *  (see switch statement). Otherwise, simply move whole pool to the end of
 *  training set.
 */ 
unsigned int moveToTraining(vector<Sentence *> &pTrainingSntncs,
                            vector<Sentence *> &pUnlabeledSntncs,
                            vector<Sentence *> &pPool,
                            unsigned pLabeledCount,
                            unsigned short pSelection,
                            unsigned short pPoolUsage,
                            double pProbThreshold, 
                            unsigned short pNumberThreshold,
                            unsigned short pConvergenceThreshold,
                            unsigned short pLogSelection,
                            string pLogFileName,
                            ofstream &pLog)
{                            
 unsigned int vSelected = 0;
 
 // if no selection is used, add whole labeled pool to the end of previous
 // training set, and clear pool
 if (pSelection == 0)
 {
  pTrainingSntncs.insert(pTrainingSntncs.end(), pPool.begin(), pPool.end());
  vSelected = pPool.size();
  pPool.clear();
  }
 else
 {
  // select according to criteria and move selected from pPool to training set
  // beside returning the number of selected
  vSelected = selectAndMove(pPool, pTrainingSntncs, pProbThreshold,
                            pNumberThreshold, pConvergenceThreshold,
                            pLogSelection, pLogFileName, pLog);
  
  switch (pPoolUsage)
  {
   // If no pool is used and no labeled sentenced was selected, self-training
   // has converged, so clear the remaining sentences to end it.
   case (0):
    if (vSelected == 0)
     pPool.clear();    
    break;
   // Keep unselected in pool to be labeled in next iteration, unless no
   // labeled item was selected above. In that case, clear the pool to 
   // be loaded with newer data in the next iteration. (to prevent
   // wandering around some difficult or neutral (no proposition) sentences)
   case (1):
    if (vSelected == 0)
     pPool.clear();    
    break;
     
   // remove away the unselected sentences
   case (2):
    pPool.clear();    
    break;

   // return (add) unselected data to the begining of unlabeled sentences
   // (useful when unlabeled data is sorted) 
   // If there was no sentence selected (vSelected == 0), check if the
   // number of labeled data in this pool wasn't 0, then add to the end,
   // (instead of begining to prevent wandering around the same sentences
   // in the next iterations) otherwise the remaining data may not be 
   // useful and will cause infinit loop, so let them to be thrown away.
   case (3):
    if (vSelected == 0)
    {
     if (pLabeledCount != 0)
      pUnlabeledSntncs.insert(pUnlabeledSntncs.end(), pPool.begin(), pPool.end());
     }
    else
     pUnlabeledSntncs.insert(pUnlabeledSntncs.begin(), pPool.begin(), pPool.end());
     
    pPool.clear();
    break;

   // return (add) unselected data to the end of unlabeled sentences
   // (useful when unlabeled data is sorted)
   // If there was no sentence selected (vSelected == 0), check if the
   // number of labeled data in this pool wasn't 0, then add to the end,
   // otherwise the remaining data may not be useful and will cause infinit
   // loop, so let them to be thrown away.
   case (4):
    if (vSelected == 0)
    {
     if (pLabeledCount != 0)
      pUnlabeledSntncs.insert(pUnlabeledSntncs.end(), pPool.begin(), pPool.end());
     }
    else 
     pUnlabeledSntncs.insert(pUnlabeledSntncs.end(), pPool.begin(), pPool.end());
     
    pPool.clear();
    break;
   }	
  }
 
 return vSelected ;
} 
 
/**
 *  selects pNumberThreshold top most probable labeling of sentences if 
 *  their probability is not less than pProbThreshold and return the number
 *  of selected sentences
 * 
 *  The probability of labeling a sentences is the average probability 
 *  of the prediction of labels of all its samples. It is calculated
 *  implicitly when sorting the pool via isMoreLikelyLabeling() function
 * 
 *  Selection can be logged into a file (including the sentences surface
 *  form, its samples with their predicted label, surface form and 
 *  probability, and the average probability of sentence labeling, based
 *  on the value of pLogSelection:
 *  0: do not log
 *  1: log only selected data
 *  2: log selected and filtered data
 */ 
unsigned int selectAndMove(vector<Sentence *> &pPool, 
                           vector<Sentence *> &pTrainingSntncs, 
                           double pProbThreshold, 
                           unsigned short pNumberThreshold,
                           unsigned short pConvergenceThreshold,
                           unsigned short pLogSelection,
                           string &pLogFileName,
                           ofstream &pLog)
{
 ofstream strmSelection;
 if (pLogSelection > 0)
  strmSelection.open(pLogFileName.c_str());

 // Remember to always set the value of this global variable before using
 // the isMoreLikelyLabeling() and isMoreLikelyLabel() functions to determine
 // the feature view for which you want to get the labeling probability
 // For 1-view applications like self-training it' value is 1 which is the
 // default, so it is not necessary to set here, but it is done only as a 
 // reminder
 gvActiveFeatureView = 1;
 sort(pPool.begin(), pPool.end(), isMoreLikelyLabeling);
 
 unsigned int cntSortedPool;
 if ((pNumberThreshold == 0) || (pPool.size() < pNumberThreshold))
  cntSortedPool = pPool.size();
 else
  cntSortedPool = pNumberThreshold;
 
 // recognizing the least-probable labeling which is not under threshold to 
 // identify the selection boundary
 for (; cntSortedPool > 0; cntSortedPool--)
  if ((pPool[cntSortedPool - 1]->getLabelingProbability(1) >= pProbThreshold) &&
      (pPool[cntSortedPool - 1]->getLabelingProbability(1) != 0))
   break;
  
 unsigned int vSelectedCount = 0;
 double vMinProbability = 0;
 double vMaxProbability = 0;
 
 // If the number of selected sentences in this iteration is less than
 // a certaian amount, it means that self-training has converged and there
 // is no room to further improvment. So, no sentences is selected.
 //
 // Note that this will also discard all labeling with probability of 0 
 // (usually when there is no predicate and thus no sample for sentence)
 if (cntSortedPool < pConvergenceThreshold)
  vSelectedCount = 0;
 else 
  vSelectedCount = cntSortedPool;
  
 if (vSelectedCount != 0)
 {
  pTrainingSntncs.insert(pTrainingSntncs.end(), 
                         pPool.begin(), pPool.begin() + vSelectedCount); 
  vMinProbability = pPool[vSelectedCount - 1]->getLabelingProbability(1);
  vMaxProbability = pPool[0]->getLabelingProbability(1);
  
  if (pLogSelection > 0)
  {
   strmSelection << "Selected:" << endl;
    
   for (unsigned int cntr = 0; cntr < vSelectedCount; cntr++)
   {
    strmSelection << cntr + 1 << "- ";
    pPool[cntr]->logPrdLabeling(1, strmSelection);
    }
   }

  pPool.erase(pPool.begin(), pPool.begin() + vSelectedCount);

  mout("From " + floatToStr(vMinProbability) +
       " to " + floatToStr(vMaxProbability) + '\n',
       cout, pLog) ;
  }
 else 
  mout("No more room to improve (" + intToStr(pPool.size()) +
       " remaining sentences in pool discarded)\n", cout, pLog);
   
 if (pLogSelection > 1)
 {
  strmSelection << "Filtered:" << endl;
  
  for (unsigned int cntr = 0; cntr < pPool.size(); cntr++)
  {
   strmSelection << cntr + 1 << "- ";
   pPool[cntr]->logPrdLabeling(1, strmSelection);
   }
  }
  
 if (strmSelection.is_open()) 
  strmSelection.close(); 
  
 return vSelectedCount;
}
