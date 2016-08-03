/**
 *  @ RSK
 *  @ Version 1.0        06.05.2010
 * 
 *  Excutable file to co-train the classifiers with bothe common and 
 *  separate training sets, and sentence-based approach
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
                             unsigned short [], string &, bool, bool, ofstream &);
short generateTestSamples(vector<Sentence *> &, unsigned short [], 
                          string &, bool, bool, ofstream &);
short generateUnlabeledSamples(vector<Sentence *> &, unsigned short [], 
                               string &, bool, bool, ofstream &);
void coTrainCommon(vector<ZMEClassifier> &, vector<Sentence *> &, vector<Sentence *> &,
                   vector<Sentence *> &, vector<Sentence *> &, vector<Sentence *> &,
                   unsigned short, string &, double, bool, unsigned int, 
                   unsigned short, unsigned short, unsigned short, unsigned short,
                   double, double, unsigned short, bool, unsigned short, 
                   unsigned short, unsigned short, unsigned short, string &, 
                   string &, string &, ofstream &);
void coTrainSeparate(vector<ZMEClassifier> &, vector<Sentence *> &, vector<Sentence *> &,
                     vector<Sentence *> &, vector<Sentence *> &, vector<Sentence *> &,
                     unsigned short, string &, double, bool, unsigned int, 
                     unsigned short, unsigned short, unsigned short, 
                     unsigned short, double, unsigned short, bool, 
                     unsigned short, unsigned short, unsigned short, 
                     unsigned short, string &, string &, string &, ofstream &);
unsigned int addToTraining(vector<Sentence *> &, vector<Sentence *> &, 
                           unsigned int);
unsigned int moveToTraining(vector<Sentence *> &, vector<Sentence *> &, 
                            vector<Sentence *> &, unsigned short, unsigned short,
                            unsigned short, unsigned short, double, double, 
                            unsigned short, unsigned short, unsigned short,
                            string, ofstream &);
unsigned int selectAndMoveConfident(vector<Sentence *> &, vector<Sentence *> &, 
                                    double, unsigned short, unsigned short, 
                                    unsigned short, string &, ofstream &);
unsigned int selectAndMoveAgreed(vector<Sentence *> &, vector<Sentence *> &, 
                                 double, unsigned short, unsigned short, 
                                 unsigned short, string &, ofstream &);
unsigned int selectAndMove(vector<Sentence *> &, vector<Sentence *> &, 
                           unsigned short, double, unsigned short, unsigned short,
                           unsigned short, string &, ofstream &);
short saveCoNLLProps(vector<Sentence *> &, unsigned short, string, ofstream &);
void setSelectedPrdLabeling(vector<Sentence *> &, unsigned short, unsigned short,
                            ofstream &);

void logNegatives(vector<Sample *> &);
void logPositives(vector<Sample *> &);
void logSamples(vector<Sample *> &);

static void usage(const char * execName)
{
 cout << "Usage: (some options cannot meet each other; e.g. pool size and explicit iteration number cannot both be non-zero" 
      << execName << " [the file containing input parameters]" << endl << endl
      << "possible parameters: (each in a new line in the parameter file)" << endl << endl
      << " <-c[Co-training method (1: Common training set; 2: Separate training set)]>" << endl 
      << " <-s[Seed size]>" << endl 
      << " <-u[Unlabeled size (0: only base classifier)]>" << endl 
      << " <-mxp[Maximum positive samples to be generated and used]>" << endl 
      << " <-mxn[Maximum negative samples to be generated and used]>" << endl 
      << " <-fs[Feature sets for each view (e.g. -fs 1,3: 1 for view 1 and 3 for view 2)]>" << endl
      << " <-go[Global optimization (0: no; 1: yes)]>" << endl 
      << " <-mi[ME iteration number]>" << endl 
      << " <-mp[ME parameter estimation method (lbfgs or gis)]>" << endl 
      << " <-g[Gaussian parameter]>" << endl 
      << " <-sc[Selection criterion(0: no selection;" << endl
      << "                          1: agreement-based;" << endl
      << "                          2: confidence-based)]>" << endl 
      << " <-cl[Common label selection method (0: no common label selection;" << endl
      << "                                     1: agreement-only (NULL for non-agreed);" << endl
      << "                                     2: agreement-confidence;" << endl
      << "                                     3: agreement-prefered-view;" << endl
      << "                                     4: confidenc-only)]>" << endl 
      << " <-pv[Pereferred view when -cl is set to agreement-prefered-view]>" << endl 
      << " <-p[Pool size (0 if not used)]>" << endl
      << " <-pu[Pool usage when selection is used (0: no pool used;" << endl
      << "						 				   1: iterate to select whole;" << endl
      << "                                         2: iterate once and remove unselected;" << endl
      << "                                         3: iterate once and return unselected to the begining of unlabeled data;" << endl
      << "                                         4: iterate once and return unselected to the end of unlabeled data)>" << endl
      << " <-pq[Quality selection for pool (0: not used; 1: shorters first; 2: mediate lengths first; 3: simplers first)]>" << endl 
      << " <-pt[Selection probability threshold (0 if no threshold)]>" << endl 
      << " <-at[Agreement level threshold (1 for full agreement)]>" << endl 
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
 
 unsigned short vCoTrainingMethod = 1;
 unsigned int vSeedSize = 0;
 unsigned int vUnlabeledSize = 0;
 unsigned int vMaxPositive = MAX_TRAIN_POSITIVE;
 unsigned int vMaxNegative = MAX_TRAIN_NEGATIVE;
 unsigned short vFeatureSets[FEATURE_VIEW_COUNT];
 bool vGlobalOpt = false;
 unsigned int vMEIterations = 350;
 string vMEPEMethod = "lbfgs";
 double vGaussian = 1;
 unsigned int vPoolSize = 0;
 unsigned short vSelection = 0;
 unsigned short vCLSelMethod= AGREEMENT_CONFIDENCE;
 unsigned short vPreferedView = 1;
 unsigned short vPoolUsage = 3;
 double vAgreeThreshold = 1;
 unsigned short vNumberThreshold = 0;
 double vProbThreshold = 0;
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
 
 string tmpLogFile = LOG_COTRAINING_OUTPUT_FILE_PREFIX;
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

  if (vcParam[0] == "-c")
  { 
   vCoTrainingMethod = strToInt(vcParam[1]);
   continue;
   }
   
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
   string tmpFSs = vcParam[1];
   vector<string> tmpcFSs;
   tokenize(tmpFSs, tmpcFSs, " ,");
   for (unsigned short i = 0; i < tmpcFSs.size(); i++)
    vFeatureSets[i] = strToInt(tmpcFSs[i]);
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
   
  if (vcParam[0] == "-cl")
  { 
   vCLSelMethod = strToInt(vcParam[1]);
   continue;
   }
   
  if (vcParam[0] == "-pv")
  { 
   vPreferedView = strToInt(vcParam[1]);
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
   
  if (vcParam[0] == "-at")
  { 
   vAgreeThreshold = strToFloat(vcParam[1]);
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

 // overrriding vSelection to confidence-based when the separate-training-set
 // method is chosen as co-training method but agreement-based selection
 // criterion is selected
 if (vCoTrainingMethod == 2)
  if (vSelection  == 1)
   vSelection = 2;

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


/**
 *   loading CoNLL labeled training data
 * 
 *   From entire the corpus, only vSeedSize sentences will be loaded, 
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
 *  generating training samples from labeled sentences for base classifiers
 */

 if (generateLabeledSamples(vcLabeledSntncs, 
                            vMaxPositive, vMaxNegative, vFeatureSets, 
                            vLabeledWordsFile, true, false, log) != 0)
  return -1;

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


/**
 *  generating test samples to test base and co-trained classifiers
 */

 if (vTesting >= 1)
 {
  mout("\nGenerating WSJ development samples ...\n", cout, log);
  if (generateTestSamples(vcDevWSJSntncs, vFeatureSets, vDevWSJWordsFile, true, false, log) != 0)
   return -1;
  }
  
 if (vTesting >= 2)
 {
  mout("\nGenerating WSJ test samples ...\n", cout, log);
  if (generateTestSamples(vcTestWSJSntncs, vFeatureSets, vTestWSJWordsFile, true, false, log) != 0)
   return -1;
  }
  
 if (vTesting >= 3)
 {
  mout("\nGenerating Brown test samples ...\n", cout, log);
  if (generateTestSamples(vcTestBrownSntncs, vFeatureSets, vTestBrownWordsFile, true, false, log) != 0)
   return -1;
  }
  
 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


/**
 *  training the base classifiers
 */
 
 vector<ZMEClassifier> zmeClassifiers (2);
 
/**
 *  Here a vector of sentence for which the samples have been generated
 *  above is passed to trainer (instead of directly passing positive and
 *  negative samples themselves)
 * 
 *  The 4rd parameter sets the use of gold labels in training, since this 
 *  is the base classifier
 */
 for (unsigned short cntView = 1; cntView <= FEATURE_VIEW_COUNT; cntView++)
 {
  mout("\nTraining base classifier " + intToStr(cntView) + " (" + getLocalTimeStr() + ") ...\n", cout, log);
  zmeClassifiers[cntView - 1].train(vcLabeledSntncs, cntView, cntView, true,
                             1, vMEIterations, vMEPEMethod, vGaussian, true, 
                             ZME_TRAINING_MODEL_FILE, log);
  mout("\nTraining the base classifier " + intToStr(cntView) + " is done! (" +
       floatToStr(zmeClassifiers[cntView - 1].getElapsedTime()) + " sec)\n",
       cout, log);

  ///////////////
  mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
       cout, log);
  }


/**
 *  testing the base classifiers & writing the labeled samples into a CoNLL
 *  props file format to be evaluated afterward
 * 
 *  After labeling with the base classifiers of all views, it selects and 
 *  sets the common label between all view based on the selection method
 *  specified. 
 *  
 *  A ".x.0" suffix is added to the output props file name to indicate that
 *  this is the labeled data by the base classifier for view x (iteration 0).
 *  .0.0 is the props file created based on selected common labeling.
 * 
 *  Here a vector of sentence for which the samples have been generated
 *  above is passed to test (instead of passing samples themselves)
 */
 
 for (unsigned short cntView = 1; cntView <= FEATURE_VIEW_COUNT; cntView++)
 {
  if (vTesting >= 1)
  {
   mout("\nLabeling development data with base classifier " + intToStr(cntView) + " ...\n", cout, log);
   zmeClassifiers[cntView - 1].classify(vcDevWSJSntncs, cntView, vGlobalOpt, log);
   mout("Labeling development data is done! (" +
        floatToStr(zmeClassifiers[cntView - 1].getElapsedTime()) + " sec)\n",
        cout, log);  
   
   mout("\nSaving labeled development props into CoNLL file ...\n", cout, log);
   if (saveCoNLLProps(vcDevWSJSntncs, cntView, vDevWSJOutput + "." + intToStr(cntView) + ".0", log) != 0)
    return -1;
   }

  if (vTesting >= 2)
  {
   mout("\nLabeling WSJ test data with base classifier " + intToStr(cntView) + " ...\n", cout, log);
   zmeClassifiers[cntView - 1].classify(vcTestWSJSntncs, cntView, vGlobalOpt, log);
   mout("Labeling WSJ test data is done! (" +
        floatToStr(zmeClassifiers[cntView - 1].getElapsedTime()) + " sec)\n",
        cout, log);

   mout("\nSaving labeled WSJ test props into CoNLL file ...\n", cout, log);
   if (saveCoNLLProps(vcTestWSJSntncs, cntView, vTestWSJOutput + "." + intToStr(cntView) +  ".0", log) != 0)
    return -1;
   }     

  if (vTesting >= 3)
  {
   mout("\nLabeling Brown test data with base classifier " + intToStr(cntView) + " ...\n", cout, log);
   zmeClassifiers[cntView - 1].classify(vcTestBrownSntncs, cntView, vGlobalOpt, log);
   mout("Labeling Brown test data is done! (" +
        floatToStr(zmeClassifiers[cntView - 1].getElapsedTime()) + " sec)\n",
        cout, log);

   mout("\nSaving labeled Brown test props into CoNLL file ...\n", cout, log);
   if (saveCoNLLProps(vcTestBrownSntncs, cntView, vTestBrownOutput + "." + intToStr(cntView) + ".0", log) != 0)
    return -1;
   }

  }

 if (vTesting >= 1)
 {
  mout("\nLabeling development data with selected common labels ...\n", cout, log);
  setSelectedPrdLabeling(vcDevWSJSntncs, CONFIDENECE_ONLY, vPreferedView, log);
  mout("\nLabeling development data is done!\n", cout, log);  
  
  mout("\nSaving labeled development props into CoNLL file ...\n", cout, log);
  if (saveCoNLLProps(vcDevWSJSntncs, 0, vDevWSJOutput + ".0.0", log) != 0)
   return -1;
  }

 if (vTesting >= 2)
 {
  mout("\nLabeling WSJ test data with selected common labels ...\n", cout, log);
  setSelectedPrdLabeling(vcTestWSJSntncs, CONFIDENECE_ONLY, vPreferedView, log);
  mout("\nLabeling WSJ test data is done!\n", cout, log);  

  mout("\nSaving labeled WSJ test props into CoNLL file ...\n", cout, log);
  if (saveCoNLLProps(vcTestWSJSntncs, 0, vTestWSJOutput + ".0.0", log) != 0)
   return -1;
  }     

 if (vTesting >= 3)
 {
  mout("\nLabeling Brown test data with selected common labels ...\n", cout, log);
  setSelectedPrdLabeling(vcTestBrownSntncs, CONFIDENECE_ONLY, vPreferedView, log);
  mout("\nLabeling Brown test data is done!\n", cout, log);  

  mout("\nSaving labeled Brown test props into CoNLL file ...\n", cout, log);
  if (saveCoNLLProps(vcTestBrownSntncs, 0, vTestBrownOutput + ".0.0", log) != 0)
   return -1;
  }

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


/**
 *  Co-training
 */

 // if the specified size for unlabeled data is 0, ignore co-training
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
  *  generating unlabeled training samples for co-training
  */

 if (generateUnlabeledSamples(vcUnlabeledSntncs, vFeatureSets, vUnlabeledDataFile, false, false, log) != 0)
  return -1;

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);


 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
  
 mout("\nCo-training started at " + getLocalTimeStr() + " ...\n", cout, log);
 if (vCoTrainingMethod == 1)
  coTrainCommon(zmeClassifiers, vcLabeledSntncs, vcUnlabeledSntncs, 
                vcDevWSJSntncs, vcTestWSJSntncs, vcTestBrownSntncs,
                vMEIterations, vMEPEMethod, vGaussian, vGlobalOpt, vPoolSize,
                vSelection, vCLSelMethod, vPreferedView, vPoolUsage, vProbThreshold, 
                vAgreeThreshold, vNumberThreshold, vRemoveLabeled, vSTIterations, 
                vPoolQuality, vLogSelection, vTesting, vDevWSJOutput, vTestWSJOutput, 
                vTestBrownOutput, log);
 else if (vCoTrainingMethod == 2)
  coTrainSeparate(zmeClassifiers, vcLabeledSntncs, vcUnlabeledSntncs, 
                  vcDevWSJSntncs, vcTestWSJSntncs, vcTestBrownSntncs,
                  vMEIterations, vMEPEMethod, vGaussian, vGlobalOpt, vPoolSize,
                  vSelection, vCLSelMethod, vPreferedView, vPoolUsage, vProbThreshold, 
                  vNumberThreshold, vRemoveLabeled, vSTIterations, vPoolQuality, 
                  vLogSelection, vTesting, vDevWSJOutput, vTestWSJOutput, 
                  vTestBrownOutput, log);
 
 mout("\nCo-training is done at " + getLocalTimeStr() + "! ", cout, log);
 vEndClk = clock();
 mout("(" + floatToStr(((double) (vEndClk - vStartClk)) / (CLOCKS_PER_SEC * 60)), 
      cout, log);
 time(&vEndTime);
 mout(" min/" + floatToStr(difftime(vEndTime, vStartTime) / 60) + " min)\n",
      cout, log);

 log.close();

 ///////////////
 mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
      cout, log);

 cout << endl;

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

///---------------------------------------------------------------------

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

///---------------------------------------------------------------------
/**
 *  generate training samples from labeled sentences for base classifier
 *  (Samples can be written into a file in ZME format optionally by setting
 *  the pWriteToFile to true)
 */

short generateLabeledSamples(vector<Sentence *> &pSentences,
                             unsigned int pMaxPositive,
                             unsigned int pMaxNegative,
                             unsigned short pFeatureSets[],
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
 if (oSampleGenerator.generateSamples(pSentences, 
                                      vcPositiveSamples,
                                      vcNegativeSamples,
                                      FEATURE_VIEW_COUNT, pFeatureSets,
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
 *  generate unlabeled samples
 */
short generateUnlabeledSamples(vector<Sentence *> &pSentences,
                               unsigned short pFeatureSets[],
                               string &pDatasetID,
                               bool pWriteToFile,
                               bool pLogSamples,
                               ofstream &pLog)
{                                 
 mout("\nGenerating unlabeled samples ...\n", cout, pLog);

 UnlabeledSampleGenerator oSampleGenerator;
 vector<Sample *> vcSamples;
 if (oSampleGenerator.generateSamples(pSentences, vcSamples, FEATURE_VIEW_COUNT,
                                      pFeatureSets, pWriteToFile) == 0)
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

///---------------------------------------------------------------------
/**
 *  generate test samples
 *  (Samples can be written into a file in ZME format optionally by setting
 *  the pWriteToFile to true)
 */
short generateTestSamples(vector<Sentence *> &pSentences,
                          unsigned short pFeatureSets[],  
                          string &pDatasetID,
                          bool pWriteToFile,
                          bool pLogSamples,
                          ofstream &pLog)
{                                 
 TestSampleGenerator oSampleGenerator;
 vector<Sample *> vcSamples;
 if (oSampleGenerator.generateSamples(pSentences, vcSamples, FEATURE_VIEW_COUNT,
                                      pFeatureSets, pWriteToFile) == 0)
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
                     unsigned short pFeatureView,
                     string pOutputPropsFile, ofstream &pLog)
{
 CoNLLSaver oCoNLLSaver(pOutputPropsFile);
 
 if (oCoNLLSaver.saveProps(pLabeledSntncs, pFeatureView) == 0)
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

///---------------------------------------------------------------------
/**
 *  selects and sets a labeling common between views based on the specified 
 *  selection method
 *  
 *  This labeling is used for 2 purposes:
 *  1. labeling test data with common labels between views (in addition 
 *     to with each view)
 *  2. labeling training data to train views on common labels (with 
 *     common-training-set co-training method)
 * 
 *  Note that:
 *  1- Unlike confidence-based selection (-sc), agreement-based selection
 *     cannot look into common label to decide, but also it reexamines the
 *     views' labeling itself. So, do not use confidence-based common label
 *     selection methods (-cl) for agreement-based selection, since training 
 *     and selection will be different.
 *  2- For labeling test data, only CONFIDENCE_ONLY option is used without
 *     involing user option of -cl, because:
 *     - AGREEMENT_CONFIDENCE and CONFIDENCE_ONLY makes exaclty the same
 *       output.
 *     - AGREEMENT_PREFERED_VIEW makes the output label exactly like the
 *       output of preferred view. So, it is almost not useful and may be
 *       removed in the future.
 *  3- For training with common labels, AGREEMENT_ONLY common labeling 
 *     usually degrades the results because it assign NULL to non-agreed
 *     samples. So, it should not be used with other than agreement-based
 *     selection with agreement threshold 1. (It may be improved by replacing
 *     NULL assignment with other method.)     
 *  4- For training with common labels, AGREEMENT_CONFIDENCE is neutral 
 *     with agreement-based co-training with agreement threshold 1.
 *  5- For training with common labels, AGREEMENT_PREFERED_VIEW makes
 *     the output label exactly like the output of preferred view. So, it
 *     is almost not useful and may be removed in the future.
 */
void setSelectedPrdLabeling(vector<Sentence *> &pcLabeledSntncs, 
                            unsigned short pSelectionMethod,
                            unsigned short pPreferedView,
                            ofstream &pLog)
{
 for (vector<Sentence *>::iterator itSentence = pcLabeledSntncs.begin(); 
      itSentence < pcLabeledSntncs.end(); 
      itSentence++)
 {     
  (*itSentence)->setSelectedPrdLabels(pSelectionMethod, pPreferedView);
  cout << "\r" << (itSentence - pcLabeledSntncs.begin() + 1) << " sentences processed" << flush;
  }
 pLog << pcLabeledSntncs.size() << " sentences processed" << flush;

}

///---------------------------------------------------------------------
/**
 *  co-training with common training set for all views
 * 
 *  For explanation of parameters, refer to the option list of the main
 *  function above
 * 
 */
void coTrainCommon(vector<ZMEClassifier> &pZMEClassifiers,
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
                   unsigned short pCLSelMethod,
                   unsigned short pPreferedView,
                   unsigned short pPoolUsage,
                   double pProbThreshold,
                   double pAgreeThreshold,
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
 mout("Common Label Selection Method: " + intToStr(pCLSelMethod) + '\n', cout, pLog);
 mout("Pool Usage: " + intToStr(pPoolUsage) + '\n', cout, pLog);
 mout("Agreement Threshold for Selection: " + floatToStr(pAgreeThreshold) + '\n', cout, pLog);
 mout("Probability Threshold for Selection: " + floatToStr(pProbThreshold) + '\n', cout, pLog);
 mout("Number of Labeled Data for Selection: " + intToStr(pNumberThreshold) + '\n', cout, pLog);
 mout("Remove Once Labeled: " + intToStr(pRemoveLabeled) + '\n', cout, pLog);
 mout("Co-training Explicit Iteration number: " + intToStr(pSTIterations) + '\n', cout, pLog);
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
  // criterion controlling rules inside the loop fires (at the end of the 
  // loop)
  while (1)
  {
   mout("\nIteration " + intToStr(cntIteration) +
        " (Pool " + intToStr(cntPool) + "):\n", cout, pLog);

   /**
    *  labeling unlabeled samples (sentence-based)
    */
 
   for (unsigned short cntView = 1; cntView <= FEATURE_VIEW_COUNT; cntView++)
   {
    mout("\nLabeling unlabeled samples with classifier of view " + intToStr(cntView) + " ...\n", cout, pLog);
    pZMEClassifiers[cntView - 1].classify(vPool, cntView, pGlobalOpt, pLog);
    mout("Labeling unlabeled samples is done! (" +
         floatToStr(pZMEClassifiers[cntView - 1].getElapsedTime()) + " sec)\n",
         cout, pLog);
    }

   mout("\nLabeling unlabeled samples with selected common labels ...\n", cout, pLog);
   setSelectedPrdLabeling(vPool, pCLSelMethod, pPreferedView, pLog);
   mout("\nLabeling unlabeled samples is done!\n", cout, pLog);  
   
   /**
    *  selecting and adding newly labeled data to training set
    */
 
   mout("\nAdding newly labeled data to training set ...\n", cout, pLog);
   
   unsigned int vAddedCount = 0;
   
   if (pRemoveLabeled)
    vAddedCount = moveToTraining(pTrainingSntncs, pUnlabeledSntncs, vPool,
                                 1, pSelection, 0, pPoolUsage, 
                                 pProbThreshold, pAgreeThreshold, pNumberThreshold,
                                 DEFAULT_CO_TRAINING_CONVERGENCE_SENTENCE_COUNT,
                                 pLogSelection, LOG_POOL_SELECTION_FILE_NAME +
                                 intToStr(cntIteration) + ".log",
                                 pLog);
   else 
    // Note:
    // 1. When the labeled data should not be removed, selection may not 
    //    be useful, since in all iterations almost the same data may be
    //    selected.
    // 2. When common training set for all views are used, it is not meaningful
    //    to add all the newly labeled data without selection, and thus 
    //    without removing them. So, this option should not be set as the
    //    parameter for common training set setups. It is kept here only 
    //    for consistency with other setups and also preventing exceptions
    //    on inapproperiate parameters.
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
     *  The 3rd parameter sets the view on whose labels the training is done.
     *  0 is for common selected labeling, which has been set above after 
     *  labeling unlabeled samples by all views.
     * 
     *  The 4th parameter sets the use of predicted labels in training, since
     *  this is the co-trained classifier
     */
  
    for (unsigned short cntView = 1; cntView <= FEATURE_VIEW_COUNT; cntView++)
    {
     mout("\nTraining " + intToStr(cntIteration) + "th classifier of view " + 
          intToStr(cntView) + " (" + getLocalTimeStr() + ") ...\n", cout, pLog);
     pZMEClassifiers[cntView - 1].train(pTrainingSntncs, cntView, COMMON_VIEW, false, 0, 
                                       pMEIterations, pMEPEMethod, pGaussian, 
                                       false, "", pLog);
     mout("\nTraining the " + intToStr(cntIteration) + "th classifier of view " + 
          intToStr(cntView) + " is done! (" + 
          floatToStr(pZMEClassifiers[cntView - 1].getElapsedTime()) + " sec)\n",
          cout, pLog);

     }


    /**
     *  testing the new classifiers & writing the labeled samples into a 
     *  CoNLL props file format to be evaluated afterward
     *  
     *  A ".#.#" suffix is added to the output props file name to indicate
     *  the classifier view and iteration number respectively
     */
    
    for (unsigned short cntView = 1; cntView <= FEATURE_VIEW_COUNT; cntView++)
    {
     if (pTesting >= 1)
     {
      mout("\nLabeling development data with " + intToStr(cntIteration) +
           "th classifier of view " + intToStr(cntView) + " ...\n", cout, pLog);
      pZMEClassifiers[cntView - 1].classify(pDevWSJSntncs, cntView, pGlobalOpt, pLog);
      mout("Labeling development data is done! (" + 
           floatToStr(pZMEClassifiers[cntView - 1].getElapsedTime()) + " sec)\n",
           cout, pLog); 

      mout("\nSaving labeled development props into CoNLL file ...\n", cout, pLog);
      saveCoNLLProps(pDevWSJSntncs, cntView, pDevWSJOutput + 
                     "." + intToStr(cntView) + "." + intToStr(cntIteration), pLog);
      }
     
     if (pTesting >= 2)
     {
      mout("\nLabeling WSJ test data with " + intToStr(cntIteration) +
           "th classifier of view " + intToStr(cntView) + " ...\n", cout, pLog);
      pZMEClassifiers[cntView - 1].classify(pTestWSJSntncs, cntView, pGlobalOpt, pLog);
      mout("Labeling WSJ test data is done! (" + 
           floatToStr(pZMEClassifiers[cntView - 1].getElapsedTime()) + " sec)\n", 
           cout, pLog); 

      mout("\nSaving labeled WSJ test props into CoNLL file ...\n", cout, pLog);
      saveCoNLLProps(pTestWSJSntncs, cntView, pTestWSJOutput + 
                     "." + intToStr(cntView) + "." + intToStr(cntIteration), pLog);
      }
     
     if (pTesting >= 3)
     {
      mout("\nLabeling Brown test data with " + intToStr(cntIteration) +
           "th classifier of view " + intToStr(cntView) + " ...\n", cout, pLog);
      pZMEClassifiers[cntView - 1].classify(pTestBrownSntncs, cntView, pGlobalOpt, pLog);
      mout("Labeling Brown test data is done! (" + 
           floatToStr(pZMEClassifiers[cntView - 1].getElapsedTime()) + " sec)\n", 
           cout, pLog); 

      mout("\nSaving labeled Brown test props into CoNLL file ...\n", cout, pLog);
      saveCoNLLProps(pTestBrownSntncs, cntView, pTestBrownOutput + 
                     "." + intToStr(cntView) + "." + intToStr(cntIteration), pLog);
      }
  
     }
     
    /**
     *  labeling test sets with common selected label between classifier
     *  & writing the labeled samples into a CoNLL props file format to 
     *  be evaluated afterward
     *  
     *  A ".0.#" suffix is added to the output props file name to indicate
     *  the common labeling (0) and iteration number (#)
     */
    
    if (pTesting >= 1)
    {
     mout("\nLabeling development data with selected common labels ...\n", cout, pLog);
     setSelectedPrdLabeling(pDevWSJSntncs, CONFIDENECE_ONLY, pPreferedView, pLog);
     mout("\nLabeling development data is done!\n", cout, pLog);  
  
     mout("\nSaving labeled development props into CoNLL file ...\n", cout, pLog);
     saveCoNLLProps(pDevWSJSntncs, 0, pDevWSJOutput + ".0." + intToStr(cntIteration), pLog);
     }

    if (pTesting >= 2)
    {
     mout("\nLabeling WSJ test data with selected common labels ...\n", cout, pLog);
     setSelectedPrdLabeling(pTestWSJSntncs, CONFIDENECE_ONLY, pPreferedView, pLog);
     mout("\nLabeling WSJ test data is done!\n", cout, pLog);  

     mout("\nSaving labeled WSJ test props into CoNLL file ...\n", cout, pLog);
     saveCoNLLProps(pTestWSJSntncs, 0, pTestWSJOutput + ".0." + intToStr(cntIteration), pLog);
     }     

    if (pTesting >= 3)
    {
     mout("\nLabeling Brown test data with selected common labels ...\n", cout, pLog);
     setSelectedPrdLabeling(pTestBrownSntncs, CONFIDENECE_ONLY, pPreferedView, pLog);
     mout("\nLabeling Brown test data is done!\n", cout, pLog);  

     mout("\nSaving labeled Brown test props into CoNLL file ...\n", cout, pLog);
     saveCoNLLProps(pTestBrownSntncs, 0, pTestBrownOutput + ".0." + intToStr(cntIteration), pLog);
     }
    }
   
   cntIteration++; 


   /**
    *  logging the amount of data has been used so far
    * 
    *  The amount includes seed data plus the portion of unlabeled data
    *  has been removed when pRemoveLabeled is set or the size of current 
    *  pool when it is not set (and pool is used).
    * 
    *  Although with common cotraining, these amount are the same for all
    *  views, to retain log consistency with separate cotraining, we log 
    *  it for all views.
    * 
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

   for (unsigned short cntView = 1; cntView <= FEATURE_VIEW_COUNT; cntView++)
    mout("\nData used so far: " + intToStr(vDataUsedSofar) +
         " sentences for training view " + intToStr(cntView),
         cout, pLog);
   mout("\n", cout, pLog);
  
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

  ///////////////
  mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
       cout, pLog);

  }
}

///---------------------------------------------------------------------
/**
 *  co-training with separate training set for each view (each view labels
 *  the data for training the other view)
 * 
 *  Only two view is supported for this method of co-training
 * 
 *  Note that, pCLSelMethod and pPreferedView are only used for labeling
 *  test sets not unlabeled data, because this method is not based on common
 *  training set.
 * 
 *  For explanation of parameters, refer to the option list of the main
 *  function above
 * 
 */
void coTrainSeparate(vector<ZMEClassifier> &pZMEClassifiers,
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
                     unsigned short pCLSelMethod,
                     unsigned short pPreferedView,
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
 unsigned short vSeedSize = pTrainingSntncs.size();
 unsigned vAllUSize = pUnlabeledSntncs.size();
 
 // overriding pool usage of no pool is used
 if (pPoolSize == 0)
  pPoolUsage = 0;
 
 mout("\n", cout, pLog);
 mout("Seed Size: " + intToStr(vSeedSize) + '\n', cout, pLog);
 mout("Unlabeled Size: " + intToStr(pUnlabeledSntncs.size()) + '\n', cout, pLog);
 mout("Pool Size: " + intToStr(pPoolSize) + '\n', cout, pLog);
 mout("Selection Criterion: " + intToStr(pSelection) + '\n', cout, pLog);
 mout("Common Label Selection Method: " + intToStr(pCLSelMethod) + '\n', cout, pLog);
 mout("Pool Usage: " + intToStr(pPoolUsage) + '\n', cout, pLog);
 mout("Probability Threshold for Selection: " + floatToStr(pProbThreshold) + '\n', cout, pLog);
 mout("Number of Labeled Data for Selection: " + intToStr(pNumberThreshold) + '\n', cout, pLog);
 mout("Remove Once Labeled: " + intToStr(pRemoveLabeled) + '\n', cout, pLog);
 mout("Co-training Explicit Iteration number: " + intToStr(pSTIterations) + '\n', cout, pLog);
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
  
 // The global iteration counter
 unsigned short cntIteration = 1;
 
 //  Pool counters for each view
 unsigned short cntPool1 = 1;
 unsigned short cntPool2 = 1;
 
 // training sets and unlabeled sentences for each view
 vector<Sentence *> vTrainingSntncs1 = pTrainingSntncs;
 vector<Sentence *> vTrainingSntncs2 = pTrainingSntncs;
 vector<Sentence *> vUnlabeledSntncs1 = pUnlabeledSntncs;
 vector<Sentence *> vUnlabeledSntncs2 = pUnlabeledSntncs;
 
 // freeing memory for original training and unlabeled set
 pTrainingSntncs.clear();
 pUnlabeledSntncs.clear();
 
 // pools for each view
 vector<Sentence *> vPool1;
 vector<Sentence *> vPool2;
 
 unsigned int vPoolSize1 = pPoolSize;
 unsigned int vPoolSize2 = pPoolSize;
  
 // Iterating for loading pools until all unlabeled data for at least one
 // view are used. (When unlabeled data for one view is finished, it's not 
 // meaningful to continue with other view, since it's trained on unlabeled
 // data of former view). This also applies to pools.
 while ((vUnlabeledSntncs1.size() > 0) && (vUnlabeledSntncs2.size() > 0))
 {
  
  /**
   * loading pools:
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
  
  if (vUnlabeledSntncs1.size() < pPoolSize)
   vPoolSize1 = vUnlabeledSntncs1.size();
  vPool1.insert(vPool1.end(),
                vUnlabeledSntncs1.begin(), 
                vUnlabeledSntncs1.begin() + vPoolSize1);
  vUnlabeledSntncs1.erase(vUnlabeledSntncs1.begin(), 
                          vUnlabeledSntncs1.begin() + vPoolSize1);
   
  if (vUnlabeledSntncs2.size() < pPoolSize)
   vPoolSize2 = vUnlabeledSntncs2.size();
  vPool2.insert(vPool2.end(),
                vUnlabeledSntncs2.begin(), 
                vUnlabeledSntncs2.begin() + vPoolSize2);
  vUnlabeledSntncs2.erase(vUnlabeledSntncs2.begin(), 
                          vUnlabeledSntncs2.begin() + vPoolSize2);
   
  // iterating to train classifiers using current pools until one of stop
  // criterion controlling rules inside the loop fires for at least one
  // view (at the end of the loop)
  while (1)
  {
   mout("\nIteration " + intToStr(cntIteration) +
        " (Pool " + intToStr(cntPool1) + " of view 1 and Pool " +
        intToStr(cntPool2) + " of view 2):\n", cout, pLog);

   /**
    *  labeling unlabeled samples (sentence-based)
    */
 
   mout("\nLabeling unlabeled samples with classifier of view 1 ...\n", cout, pLog);
   pZMEClassifiers[0].classify(vPool1, 1, pGlobalOpt, pLog);
   mout("Labeling unlabeled samples is done! (" +
        floatToStr(pZMEClassifiers[0].getElapsedTime()) + " sec)\n",
        cout, pLog);

   mout("\nLabeling unlabeled samples with classifier of view 2 ...\n", cout, pLog);
   pZMEClassifiers[1].classify(vPool2, 2, pGlobalOpt, pLog);
   mout("Labeling unlabeled samples is done! (" +
        floatToStr(pZMEClassifiers[1].getElapsedTime()) + " sec)\n",
        cout, pLog);

   /**
    *  selecting and adding newly labeled data to training set
    */
 
   mout("\nAdding newly labeled data by view 1 to training set of view 2 ...\n", cout, pLog);
   
   unsigned int vAddedCount2 = 0;
   
   if (pRemoveLabeled)
    vAddedCount2 = moveToTraining(vTrainingSntncs2, vUnlabeledSntncs1, vPool1,
                                  2, pSelection, 1, pPoolUsage, 
                                  pProbThreshold, 0, pNumberThreshold,
                                  DEFAULT_CO_TRAINING_CONVERGENCE_SENTENCE_COUNT,
                                  pLogSelection, string(LOG_POOL_SELECTION_FILE_NAME) +
                                  ".1." + intToStr(cntIteration) + ".log",
                                  pLog);
   else 
    // Note:
    // 1. When the labeled data should not be removed, selection may not 
    //    be useful, since in all iterations almost the same data may be
    //    selected.
    // 2. When common training set for all views are used, it is not meaningful
    //    to add all the newly labeled data without selection, and thus 
    //    without removing them. So, this option should not be set as the
    //    parameter for common training set setups. It is kept here only 
    //    for consistency with other setups and also preventing exceptions
    //    on inapproperiate parameters.
    vAddedCount2 = addToTraining(vTrainingSntncs2, vPool1, vSeedSize);
    
   mout("Adding labeled data is done! (" + intToStr(vAddedCount2) +
        " sentences)\n", cout, pLog);
 
   mout("\nAdding newly labeled data by view 2 to training set of view 1 ...\n", cout, pLog);
   
   unsigned int vAddedCount1= 0;
   
   if (pRemoveLabeled)
    vAddedCount1 = moveToTraining(vTrainingSntncs1, vUnlabeledSntncs2, vPool2,
                                  2, pSelection, 2, pPoolUsage, 
                                  pProbThreshold, 0, pNumberThreshold,
                                  DEFAULT_CO_TRAINING_CONVERGENCE_SENTENCE_COUNT,
                                  pLogSelection, string(LOG_POOL_SELECTION_FILE_NAME) +
                                  ".2." + intToStr(cntIteration) + ".log",
                                  pLog);
   else 
    //  Refer to the comments for 1st view above.
    vAddedCount1 = addToTraining(vTrainingSntncs1, vPool2, vSeedSize);
    
   mout("Adding labeled data is done! (" + intToStr(vAddedCount1) +
        " sentences)\n", cout, pLog);
 
   
   // Only if any new labeled data is added to both training set, train
   // a new classifier (Refer to the comments on the outer while loop)
   if ((vAddedCount1 != 0) && (vAddedCount2 != 0))
   {
   
    /**
     *  training a new classifier with new labeled data
     * 
     *  The 3rd parameter sets the view on whose labels the training is done.
     *  For training each view, this is the other view.
     * 
     *  The 4th parameter sets the use of predicted labels in training, since
     *  this is the co-trained classifier
     */
  
    mout("\nTraining " + intToStr(cntIteration) + "th classifier of view 1 (" + 
         getLocalTimeStr() + ") ...\n", cout, pLog);
    pZMEClassifiers[0].train(vTrainingSntncs1, VIEW_1, VIEW_2, false, 0, pMEIterations, 
                             pMEPEMethod, pGaussian, false, "", pLog);
    mout("\nTraining the " + intToStr(cntIteration) + "th classifier of view 1 is done! (" + 
         floatToStr(pZMEClassifiers[0].getElapsedTime()) + " sec)\n",
         cout, pLog);

    mout("\nTraining " + intToStr(cntIteration) + "th classifier of view 2 (" + 
         getLocalTimeStr() + ") ...\n", cout, pLog);
    pZMEClassifiers[1].train(vTrainingSntncs2, VIEW_2, VIEW_1, false, 0, pMEIterations, 
                             pMEPEMethod, pGaussian, false, "", pLog);
    mout("\nTraining the " + intToStr(cntIteration) + "th classifier of view 2 is done! (" + 
         floatToStr(pZMEClassifiers[1].getElapsedTime()) + " sec)\n",
         cout, pLog);

    /**
     *  testing the new classifiers & writing the labeled samples into a 
     *  CoNLL props file format to be evaluated afterward
     *  
     *  A ".#" suffix is added to the output props file name to indicate the
     *  iteration
     */
    
    for (unsigned short cntView = 1; cntView <= FEATURE_VIEW_COUNT; cntView++)
    {
     if (pTesting >= 1)
     {
      mout("\nLabeling development data with " + intToStr(cntIteration) +
           "th classifier of view " + intToStr(cntView) + " ...\n", cout, pLog);
      pZMEClassifiers[cntView - 1].classify(pDevWSJSntncs, cntView, pGlobalOpt, pLog);
      mout("Labeling development data is done! (" + 
           floatToStr(pZMEClassifiers[cntView - 1].getElapsedTime()) + " sec)\n",
           cout, pLog); 

      mout("\nSaving labeled development props into CoNLL file ...\n", cout, pLog);
      saveCoNLLProps(pDevWSJSntncs, cntView, pDevWSJOutput + 
                     "." + intToStr(cntView) + "." + intToStr(cntIteration), pLog);
      }
     
     if (pTesting >= 2)
     {
      mout("\nLabeling WSJ test data with " + intToStr(cntIteration) +
           "th classifier of view " + intToStr(cntView) + " ...\n", cout, pLog);
      pZMEClassifiers[cntView - 1].classify(pTestWSJSntncs, cntView, pGlobalOpt, pLog);
      mout("Labeling WSJ test data is done! (" + 
           floatToStr(pZMEClassifiers[cntView - 1].getElapsedTime()) + " sec)\n", 
           cout, pLog); 

      mout("\nSaving labeled WSJ test props into CoNLL file ...\n", cout, pLog);
      saveCoNLLProps(pTestWSJSntncs, cntView, pTestWSJOutput + 
                     "." + intToStr(cntView) + "." + intToStr(cntIteration), pLog);
      }
     
     if (pTesting >= 3)
     {
      mout("\nLabeling Brown test data with " + intToStr(cntIteration) +
           "th classifier of view " + intToStr(cntView) + " ...\n", cout, pLog);
      pZMEClassifiers[cntView - 1].classify(pTestBrownSntncs, cntView, pGlobalOpt, pLog);
      mout("Labeling Brown test data is done! (" + 
           floatToStr(pZMEClassifiers[cntView - 1].getElapsedTime()) + " sec)\n", 
           cout, pLog); 

      mout("\nSaving labeled Brown test props into CoNLL file ...\n", cout, pLog);
      saveCoNLLProps(pTestBrownSntncs, cntView, pTestBrownOutput + 
                     "." + intToStr(cntView) + "." + intToStr(cntIteration), pLog);
      }
  
     }

    /**
     *  labeling test sets with common selected label between classifier
     *  & writing the labeled samples into a CoNLL props file format to 
     *  be evaluated afterward
     *  
     *  A ".0.#" suffix is added to the output props file name to indicate
     *  the common labeling (0) and iteration number (#)
     */
    
    if (pTesting >= 1)
    {
     mout("\nLabeling development data with selected common labels ...\n", cout, pLog);
     setSelectedPrdLabeling(pDevWSJSntncs, CONFIDENECE_ONLY, pPreferedView, pLog);
     mout("\nLabeling development data is done!\n", cout, pLog);  
   
     mout("\nSaving labeled development props into CoNLL file ...\n", cout, pLog);
     saveCoNLLProps(pDevWSJSntncs, 0, pDevWSJOutput + ".0." + intToStr(cntIteration), pLog);
     } 

    if (pTesting >= 2)
    {
     mout("\nLabeling WSJ test data with selected common labels ...\n", cout, pLog);
     setSelectedPrdLabeling(pTestWSJSntncs, CONFIDENECE_ONLY, pPreferedView, pLog);
     mout("\nLabeling WSJ test data is done!\n", cout, pLog);   

     mout("\nSaving labeled WSJ test props into CoNLL file ...\n", cout, pLog);
     saveCoNLLProps(pTestWSJSntncs, 0, pTestWSJOutput + ".0." + intToStr(cntIteration), pLog);
     }     

    if (pTesting >= 3)
    {
     mout("\nLabeling Brown test data with selected common labels ...\n", cout, pLog);
     setSelectedPrdLabeling(pTestBrownSntncs, CONFIDENECE_ONLY, pPreferedView, pLog);
     mout("\nLabeling Brown test data is done!\n", cout, pLog);  

     mout("\nSaving labeled Brown test props into CoNLL file ...\n", cout, pLog);
     saveCoNLLProps(pTestBrownSntncs, 0, pTestBrownOutput + ".0." + intToStr(cntIteration), pLog);
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
    vDataUsedSofar += vAllUSize - pUnlabeledSntncs.size() - vPool2.size();
   else
   {
    if (pPoolUsage > 0)
     vDataUsedSofar += vPool2.size();
    else 
     vDataUsedSofar += pUnlabeledSntncs.size();
    }

   mout("\nData used so far: " + intToStr(vDataUsedSofar) + " sentences for training view 1\n", cout, pLog);

   vDataUsedSofar = vSeedSize;
   
   if (pRemoveLabeled != 0)
    vDataUsedSofar += vAllUSize - pUnlabeledSntncs.size() - vPool1.size();
   else
   {
    if (pPoolUsage > 0)
     vDataUsedSofar += vPool1.size();
    else 
     vDataUsedSofar += pUnlabeledSntncs.size();
    }

   mout("Data used so far: " + intToStr(vDataUsedSofar) + " sentences for training view 2\n", cout, pLog);
   
   /**
    *  Checking stop criterion for looping inside the pool
    */

   // This is the stop criterion for the situation that pool items must be
   // removed after labeling (pRemove is set to 1)
   // If either one of pools is empty, the loop breaks(see comments for 
   // outer loop)
   if ((vPool1.size() == 0) || (vPool2.size() == 0))
   {
    vPool1.clear();
    vPool2.clear();
    break;
    }
   
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
   
  cntPool1++;
  cntPool2++;

  ///////////////
  mout("\nMemory Usage: " + setThousandSeparator(ulongToStr(getMemUsage())) + " Bytes\n",
       cout, pLog);

  }
}

///---------------------------------------------------------------------
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

///---------------------------------------------------------------------
/**
 *  moves labeled data from pool to training set (remove from pool after
 *  adding to training set)
 * 
 *  If selection has been set to be used it uses selectAndMove...() to do it,
 *  and then post-process the pool and unlabeled set based on other parameters
 *  (see switch statement). Otherwise, simply move whole pool to the end of
 *  training set.
 */ 
unsigned int moveToTraining(vector<Sentence *> &pTrainingSntncs,
                            vector<Sentence *> &pUnlabeledSntncs,
                            vector<Sentence *> &pPool,
                            unsigned short pCoTrainingMethod,
                            unsigned short pSelection,
                            unsigned short pSelectionView,
                            unsigned short pPoolUsage,
                            double pProbThreshold, 
                            double pAgreeThreshold, 
                            unsigned short pNumberThreshold,
                            unsigned short pConvergenceThreshold,
                            unsigned short pLogSelection,
                            string pLogFileName,
                            ofstream &pLog)
{                            
 unsigned int vSelected = 0;
 
 switch (pSelection)
 {
  // if no selection is used, add whole labeled pool to the end of previous
  // training set, and clear pool
  case (0):
   pTrainingSntncs.insert(pTrainingSntncs.end(), pPool.begin(), pPool.end());
   vSelected = pPool.size();
   pPool.clear();
   break;

  // select the agreement-based labeling and move selected from pPool to
  // training set beside returning the number of selected
  case (1): 
   vSelected = selectAndMoveAgreed(pPool, pTrainingSntncs, pAgreeThreshold, 
                                   pNumberThreshold, pConvergenceThreshold,
                                   pLogSelection, pLogFileName, pLog);
   break;

  // select according to criteria and move selected from pPool to training set
  // beside returning the number of selected
  case (2): 
   if (pCoTrainingMethod == 1)
    vSelected = selectAndMoveConfident(pPool, pTrainingSntncs,
                                       pProbThreshold, pNumberThreshold, 
                                       pConvergenceThreshold, pLogSelection,
                                       pLogFileName, pLog);
   else if (pCoTrainingMethod == 2)
    vSelected = selectAndMove(pPool, pTrainingSntncs, pSelectionView, 
                              pProbThreshold, pNumberThreshold, pConvergenceThreshold, 
                              pLogSelection, pLogFileName, pLog);
   break;
  } 
  
 if (pSelection != 0) 
  switch (pPoolUsage)
  {
   // If no pool is used and no labeled sentenced was selected, co-training
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

   // Return (add) unselected data to the begining of unlabeled sentences
   // (useful when unlabeled data is sorted) if some sentences was selected
   // above, UNLESS there was no sentence selected (vSelected == 0);
   // in this case see if the size of remaining unlabeled data is more than
   // half of pool size, then add to the end (instead of begining to prevent
   // wandering around some difficult sentences), otherwise the remaining 
   // data may not be useful and will cause infinit loop, so do nothing.
   case (3):
    if (vSelected == 0)
    {
     if (pUnlabeledSntncs.size() >= (pPool.size() / 2))
      pUnlabeledSntncs.insert(pUnlabeledSntncs.end(), pPool.begin(), pPool.end());
     }
    else
     pUnlabeledSntncs.insert(pUnlabeledSntncs.begin(), pPool.begin(), pPool.end());
     
    pPool.clear();
    break;

   // return (add) unselected data to the end of unlabeled sentences
   // (useful when unlabeled data is sorted)
   // If there was no sentence selected (vSelected == 0), see if the size
   // of remaining unlabeled data is more than half of pool size, then add
   // to the end, otherwise the remaining data may not be useful and will
   // cause infinit loop, so do nothing.
   case (4):
    if (vSelected == 0)
    {
     if (pUnlabeledSntncs.size() >= (pPool.size() / 2))
      pUnlabeledSntncs.insert(pUnlabeledSntncs.end(), pPool.begin(), pPool.end());
     }
    else 
     pUnlabeledSntncs.insert(pUnlabeledSntncs.end(), pPool.begin(), pPool.end());
     
    pPool.clear();
    break;
   }
 
 return vSelected;
} 
 
///---------------------------------------------------------------------
/**
 *  selects pNumberThreshold top most agreed-upon labeling of sentences if 
 *  their agreement level is not less than pAgreeThreshold and return the
 *  number of selected sentences
 * 
 *  The agreement level of labeling of a sentences with several views is
 *  calculated by the ratio of its sample whose labels are identical with
 *  all views to all of its samples.
 * 
 *  The probability of labeling a sentences is the average probability 
 *  of the prediction of labels of all its samples.
 * 
 *  Selection can be logged into a file (including the sentences surface
 *  form, its samples with their predicted label, surface form and 
 *  probability, and the average probability of sentence labeling, based
 *  on the value of pLogSelection:
 *  0: do not log
 *  1: log only selected data
 *  2: log selected and filtered data
 */ 
unsigned int selectAndMoveAgreed(vector<Sentence *> &pPool, 
                                 vector<Sentence *> &pTrainingSntncs, 
                                 double pAgreeThreshold,
                                 unsigned short pNumberThreshold,
                                 unsigned short pConvergenceThreshold,
                                 unsigned short pLogSelection,
                                 string &pLogFileName,
                                 ofstream &pLog)
{
 ofstream strmSelection;
 if (pLogSelection > 0)
  strmSelection.open(pLogFileName.c_str());

 sort(pPool.begin(), pPool.end(), isMoreAgreedLabeling);
 
 unsigned int cntSortedPool;
 if ((pNumberThreshold == 0) || (pPool.size() < pNumberThreshold))
  cntSortedPool = pPool.size();
 else
  cntSortedPool = pNumberThreshold;
 
 // recognizing the least-agreed-upon labeling meeting the threshold to 
 // identify the selection boundary
 for (; cntSortedPool > 0; cntSortedPool--)
  if (pPool[cntSortedPool - 1]->getLabelingAgreement() >= pAgreeThreshold)
   break;
  
 unsigned int vSelectedCount = 0;
 double vMinAgreement = 0;
 double vMaxAgreement = 0;
 
 // If the number of selected sentences in this iteration is less than
 // a certaian amount, it means that co-training has converged and there
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
  vMinAgreement = pPool[vSelectedCount - 1]->getLabelingAgreement();
  vMaxAgreement = pPool[0]->getLabelingAgreement();
  
  if (pLogSelection > 0)
  {
   strmSelection << "Selected:" << endl;
    
   for (unsigned int cntr = 0; cntr < vSelectedCount; cntr++)
   {
    strmSelection << cntr + 1 << "- ";
    pPool[cntr]->logPrdLabelings(FEATURE_VIEW_COUNT, strmSelection);
    }
   }

  pPool.erase(pPool.begin(), pPool.begin() + vSelectedCount);

  mout("From " + floatToStr(vMinAgreement) +
       " to " + floatToStr(vMaxAgreement) + '\n',
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
   pPool[cntr]->logPrdLabelings(FEATURE_VIEW_COUNT, strmSelection);
   }
  }
  
 if (strmSelection.is_open()) 
  strmSelection.close(); 
  
 return vSelectedCount;
}

///---------------------------------------------------------------------
/**
 *  selects pNumberThreshold top most probable labeling of sentences if 
 *  their probability is not less than pProbThreshold and return the number
 *  of selected sentences
 * 
 *  see selectAndMoveAgreed() for more comments
 * 
 */ 
unsigned int selectAndMoveConfident(vector<Sentence *> &pPool, 
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
 // the feature view for which you want to get the labeling probability.
 // For 1-view applications like self-training its value is 1 which is the
 // default, and the value of 0 is used when co-training with common training
 // sets. Also, when co-training with separate training sets, the value is
 // set to alternative views. 
 gvActiveFeatureView = 0;
 sort(pPool.begin(), pPool.end(), isMoreLikelyLabeling);
 
 unsigned int cntSortedPool;
 if ((pNumberThreshold == 0) || (pPool.size() < pNumberThreshold))
  cntSortedPool = pPool.size();
 else
  cntSortedPool = pNumberThreshold;
 
 // recognizing the least-probable labeling which is not under threshold to 
 // identify the selection boundary
 for (; cntSortedPool > 0; cntSortedPool--)
  if ((pPool[cntSortedPool - 1]->getLabelingProbability(0) >= pProbThreshold) &&
      (pPool[cntSortedPool - 1]->getLabelingProbability(0) != 0))
   break;
  
 unsigned int vSelectedCount = 0;
 double vMinProbability = 0;
 double vMaxProbability = 0;
 
 // If the number of selected sentences in this iteration is less than
 // a certaian amount, it means that co-training has converged and there
 // is no room to further improvment. So, no sentences is selected.
 //
 // Note that this will also discard all labeling with probability of 0 
 // (usually when there is no predicate and thus no sample for sentence)
 if (cntSortedPool <= pConvergenceThreshold)
  vSelectedCount = 0;
 else 
  vSelectedCount = cntSortedPool;
  
 if (vSelectedCount != 0)
 {
  pTrainingSntncs.insert(pTrainingSntncs.end(), 
                         pPool.begin(), pPool.begin() + vSelectedCount); 
  vMinProbability = pPool[vSelectedCount - 1]->getLabelingProbability(0);
  vMaxProbability = pPool[0]->getLabelingProbability(0);
  
  if (pLogSelection > 0)
  {
   strmSelection << "Selected:" << endl;
    
   for (unsigned int cntr = 0; cntr < vSelectedCount; cntr++)
   {
    strmSelection << cntr + 1 << "- ";
    pPool[cntr]->logPrdLabelings(FEATURE_VIEW_COUNT, strmSelection);
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
   pPool[cntr]->logPrdLabelings(FEATURE_VIEW_COUNT, strmSelection);
   }
  }
  
 if (strmSelection.is_open()) 
  strmSelection.close(); 
  
 return vSelectedCount;
}

///---------------------------------------------------------------------
/**
 *  selects pNumberThreshold top most probable labeling of sentences if 
 *  their probability is not less than pProbThreshold and return the number
 *  of selected sentences
 * 
 *  This version of selectAndMove... is used for co-training with separate
 *  training sets, thus it's almost similar to self-training's selectAndMove()
 * 
 *  see selectAndMoveAgreed() for more comments
 */ 
unsigned int selectAndMove(vector<Sentence *> &pPool, 
                           vector<Sentence *> &pTrainingSntncs, 
                           unsigned short pSelectionView,
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
 // default, and when co-training with separate training sets, the value is
 // set to alternative views. 
 gvActiveFeatureView = pSelectionView;
 sort(pPool.begin(), pPool.end(), isMoreLikelyLabeling);
 
 unsigned int cntSortedPool;
 if ((pNumberThreshold == 0) || (pPool.size() < pNumberThreshold))
  cntSortedPool = pPool.size();
 else
  cntSortedPool = pNumberThreshold;
 
 // recognizing the least-probable labeling which is not under threshold to 
 // identify the selection boundary
 for (; cntSortedPool > 0; cntSortedPool--)
  if ((pPool[cntSortedPool - 1]->getLabelingProbability(pSelectionView) >= pProbThreshold) &&
      (pPool[cntSortedPool - 1]->getLabelingProbability(pSelectionView) != 0))
   break;
  
 unsigned int vSelectedCount = 0;
 double vMinProbability = 0;
 double vMaxProbability = 0;
 
 // If the number of selected sentences in this iteration is less than
 // a certaian amount, it means that co-training has converged and there
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
  vMinProbability = pPool[vSelectedCount - 1]->getLabelingProbability(pSelectionView);
  vMaxProbability = pPool[0]->getLabelingProbability(pSelectionView);
  
  if (pLogSelection > 0)
  {
   strmSelection << "Selected:" << endl;
    
   for (unsigned int cntr = 0; cntr < vSelectedCount; cntr++)
   {
    strmSelection << cntr + 1 << "- ";
    pPool[cntr]->logPrdLabeling(pSelectionView, strmSelection);
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
   pPool[cntr]->logPrdLabeling(pSelectionView, strmSelection);
   }
  }
  
 if (strmSelection.is_open()) 
  strmSelection.close(); 
  
 return vSelectedCount;
}
