/**
 *  @ RSK
 *  @ Version 1.1        06.04.2010
 * 
 *  excutable file to read CoNLL evaluation score files and store precision,
 *  recall, and F1 of ech file in a CVS style file (1 row per file) to be
 *  imported by graph plotters (e.g. Excel) or other uses
 * 
 *  All evaluation score file names must end with ".score.#" or ".score.#.#" pattern
 * 
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <algorithm>

#include "Utils.hpp"

using namespace std;

struct Parameter
{
 string app;
 string coMethod;
 string seedSize;
 string uSize;
 string maxPositive;
 string maxNegative;
 string featureSet;
 string globalOpt;
 string meIterations;
 string mePEMethod;
 string meGaussian;
 string poolSize;
 string selection;
 string clSelection;
 string preferedView;
 string poolUsage;
 string probThreshold;
 string agreeThreshold;
 string growthSize;
 string removeLabeled;
 string tIterations;
 string poolQuality;
 string lTW;
 string lTS;
 string lTD;
 string lTP;
 string uTD;
 string uTDS;
};
  
struct DataUsage
{
 unsigned trainUsage;
 unsigned totalUsage;
};
  
struct Scores
{
 unsigned short it;
 string precision;
 string recall;
 string f1;
};
  
void collectFiles(vector<string> &, string &, string &);
void collectScores(string &, vector<string> &, vector<Scores *> &);
void collectParameters(string &, Parameter &);
// collecting self-training data usage
void collectSTDataUsage(string &, vector<DataUsage *> &);
// collecting a co-training view's data usage
void collectCTDataUsage(string &, vector<DataUsage *> &, short);
// collecting all co-training views' data usage
void collectAllCTDataUsage(string &, vector<vector<DataUsage *> >&, short);
void writeScores(Parameter &, vector<vector<DataUsage *> >&, vector<Scores *> &, ofstream &);
bool isOlderScoreFile(string, string);

static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][..]" << endl
      << "<-si [Score source directory]>" << endl
      << "<-li [Log directory]>" << endl
      << "<-o  [Output directory]>" << endl
      << "<-sp [Source file name prefix (e.g. test.wsj for test.wsj.score.0)]>" << endl
      << "<-lf [Log file name (containing data usage)]>" << endl
      << "<-v  [Feature View (the 1st number after .score. when two numbers appear (second is for iteration))]>" << endl
//      << "<-vc [Number of feature views to extract the data usage when -v is 0]>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 string vScoreSrcDir = "../../results";
 string vLogDir = "../../log";
 string vOutputDir = "../../analysis";
 string vScoreFilePrefix = "devel.24";
 string vLogFile;
 short vFeatureView = -1;                   // default for self-training
 short vViewCount = 2;                  

 const string ctmpStr  = "help";
 if(argv[1] == ctmpStr.c_str())
 {
  usage(argv[0]);
  exit(-1);
  }
 
 for (unsigned short cntArg = 1; cntArg < argc; cntArg++)
 {
  if (argv[cntArg][0] != '-')
   continue;
   	 
  string vParam = argv[cntArg] + 1;
  
  if (vParam == "si")
  { 
   vScoreSrcDir = argv[cntArg + 1];
   continue;
   }
  
  if (vParam == "li")
  { 
   vLogDir = argv[cntArg + 1];
   continue;
   }

  if (vParam == "o")
  { 
   vOutputDir = argv[cntArg + 1];
   continue;
   }
   
  if (vParam == "sp")
  { 
   vScoreFilePrefix = argv[cntArg + 1];
   continue;
   }

  if (vParam == "lf")
  { 
   vLogFile = argv[cntArg + 1];
   continue;
   }

  if (vParam == "v")
  { 
   vFeatureView = atoi(argv[cntArg + 1]);
   continue;
   }

  if (vParam == "vc")
  { 
   vViewCount = atoi(argv[cntArg + 1]);
   continue;
   }

  }   
 
 if (vLogFile.empty())
 {
  cerr << "\nLog file not specified!" << endl;
  return -1;
  }
   
 vLogFile = vLogDir + "/" + vLogFile;

 Parameter vParams;
 
 // collecting parameters
 collectParameters(vLogFile, vParams);


 // This is mylti-dimensional to support data collection for all views
 // (when -v is 0). Only one dimension is used with other -v's
 vector<vector<DataUsage *> > vcDataUsages;

 vector<DataUsage *> vcDataUsage;
 
 // collecting data usage
 switch (vFeatureView)
 {
  case (-1):
   collectSTDataUsage(vLogFile, vcDataUsage);
   vcDataUsages.push_back(vcDataUsage);
   break;
  case (0):
   collectAllCTDataUsage(vLogFile, vcDataUsages, vViewCount);
   break;
  default:
   collectCTDataUsage(vLogFile, vcDataUsage, vFeatureView); 
   vcDataUsages.push_back(vcDataUsage);
  }

 vector<string> vScoreFiles;

 // collecting and sorting score files
 if (vFeatureView == -1)
  vScoreFilePrefix += ".score.";
 else
  vScoreFilePrefix += ".score." + intToStr(vFeatureView) + "."; 
 
 ////// collecting scores for all co-trainin views (-v = 0) is not supported 
 ////// here, thus -vc cannot be used yet (needs to complete the code here)
  
 collectFiles(vScoreFiles, vScoreSrcDir, vScoreFilePrefix);
 sort(vScoreFiles.begin(), vScoreFiles.end(), isOlderScoreFile);
 
 if (vScoreFiles.empty())
 {
  cout << "\nNo score file to process!"<< endl;
  return 0;
  }
 
 vector<Scores *> vcScores;

 // collecting scores from files
 collectScores(vScoreSrcDir, vScoreFiles, vcScores);
 

 // creating output file
 string vOutputFile = vOutputDir + "/" + 
                      vScoreFiles[0].substr(0, vScoreFiles[0].find_last_of('.')) +
                      ".all";
 ofstream strmOutput (vOutputFile.c_str());
 if(!strmOutput)
 {
  cerr << "\nCan't create output file: " << vOutputFile << endl;
  return -1;
  }
 
 //  writing collected scores to final output
 writeScores(vParams, vcDataUsages, vcScores, strmOutput);
 
 strmOutput.close();
 cout << endl;
}


/**
 *  collect score file names into a vector
 */ 
void collectFiles(vector<string> &pFiles, string &pSourceDir, string &pPrefix)
{
 DIR *vDir;
 struct dirent *vDirEntry; 
 vDir = opendir(pSourceDir.c_str());
 
 if (vDir != NULL)
 {
  while ((vDirEntry = readdir(vDir)))
  {
   string tmpFileName = vDirEntry->d_name;
   
   size_t tmpPOS = tmpFileName.find(pPrefix);
   
   if (tmpPOS != string::npos)
   {
    string vExt = tmpFileName.substr(tmpPOS + pPrefix.size());
    
    size_t tmpPoint = vExt.find('.');
    
    if (tmpPoint == string::npos)
     if (isNumeric(vExt))
      pFiles.push_back(tmpFileName);
    }
   } 
  }
}


void collectParameters(string &pLogFile, Parameter &pParams)
{
 size_t tmp = pLogFile.find_last_of('/') + 1;
 if (pLogFile.substr(tmp, 14) == "SelfTraining.1")
  pParams.app = "st.1";
 else if (pLogFile.substr(tmp, 14) == "SelfTraining.2")
  pParams.app = "st.2";
 else if (pLogFile.substr(tmp, 12) == "Coraining.1")
  pParams.app = "ct.1";
 else if (pLogFile.substr(tmp, 12) == "CoTraining.2")
  pParams.app = "ct.2";
 
 cout << "\nProcessing parameters..." << endl;
 
 ifstream strmParams (pLogFile.c_str());
     
 if(!strmParams)
 {
  cerr << "Can't open file: " << pLogFile << endl;
  return;
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
  
  if (vcParam.size() == 0)
   break;

  if (vcParam[0] == "-c")
  { 
   pParams.coMethod = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-s")
  { 
   pParams.seedSize = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-u")
  { 
   pParams.uSize = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-mxp")
  { 
   pParams.maxPositive = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-mxn")
  { 
   pParams.maxNegative = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-go")
  { 
   pParams.globalOpt = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-fs")
  { 
   pParams.featureSet = vcParam[1];
   continue;
   }

  if (vcParam[0] == "-mi")
  { 
   pParams.meIterations = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-mp")
  { 
   pParams.mePEMethod = vcParam[1];
   continue; 
   }
   
  if (vcParam[0] == "-g")
  { 
   pParams.meGaussian = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-p")
  { 
   pParams.poolSize = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-sc")
  { 
   pParams.selection = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-cl")
  { 
   pParams.clSelection = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-pv")
  { 
   pParams.preferedView = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-pu")
  { 
   pParams.poolUsage = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-pt")
  { 
   pParams.probThreshold = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-at")
  { 
   pParams.agreeThreshold = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-nt")
  { 
   pParams.growthSize = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-r")
  { 
   pParams.removeLabeled = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-si")
  { 
   pParams.tIterations = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-pq")
  { 
   pParams.poolQuality = vcParam[1];
   continue;
   }

  if (vcParam[0] == "-ltw")
  { 
   pParams.lTW = vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-lts")
  { 
   pParams.lTS = vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-ltd")
  { 
   pParams.lTD = vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-ltp")
  { 
   pParams.lTP = vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-utd")
  { 
   pParams.uTD = vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-utds")
  { 
   pParams.uTDS = vcParam[1];
   continue; 
   }
  }
 
 cout << "Collecting parameters is done!" << endl;
}


/**
 *  collecting self-training data usage
 */
void collectSTDataUsage(string &pLogFile, vector<DataUsage *> &pcDataUsage)
{
 ifstream strmFile (pLogFile.c_str());

 if(!strmFile)
 {
  cerr << "\nCan't open log file: " << pLogFile << endl;
  return;
  }
 else 
  cout << "\nProcessing log file... " << endl;
 
 while (!strmFile.eof())
 {
  string vLine;
  vector<string> vcTokens;
   
  getline(strmFile, vLine);
  
  if (vLine.find("Training base") != string::npos)
  {
   DataUsage *tmpDataUsage = new DataUsage;
 
   getline(strmFile, vLine);
   vcTokens.clear();
   tokenize(vLine, vcTokens, " ");
 
   tmpDataUsage->trainUsage = strToInt(vcTokens[0]);
   tmpDataUsage->totalUsage = tmpDataUsage->trainUsage;
   pcDataUsage.push_back(tmpDataUsage);
   
   getline(strmFile, vLine);
   continue;
   }
   
  if ((vLine.substr(0, 9) == "Iteration") && (vLine.find("Pool") != string::npos))
  {
   // escaping 10 lines and reading 11th line
   for (short i = 1; (i <= 11) && (!strmFile.eof()); i++)
    getline(strmFile, vLine);

   vcTokens.clear();
   tokenize(vLine, vcTokens, " ");
   
   DataUsage *tmpDataUsage = new DataUsage;

   // if the 11th line after iteration start begins with "Training", there
   // has been training in this step
   if (vcTokens[0] == "Training") 
   {
   	getline(strmFile, vLine);
    vcTokens.clear();
    tokenize(vLine, vcTokens, " ");
  
    tmpDataUsage->trainUsage = strToInt(vcTokens[0]);
    
    do 
     getline(strmFile, vLine);
    while (vLine.substr(0, 4) != "Data"); 
    vcTokens.clear();
    tokenize(vLine, vcTokens, " ");
    	
    tmpDataUsage->totalUsage = strToInt(vcTokens[4]);
    pcDataUsage.push_back(tmpDataUsage);
    vcTokens.clear();
    tokenize(vLine, vcTokens, " ");
    }
   // if the 11th line after iteration start begins with "Data", there
   // was no training in this step, so use the previous trainData for
   // trainData but fetch totalData
   else if (vcTokens[0] == "Data") 
   {
    tmpDataUsage->trainUsage = pcDataUsage.back()->trainUsage;
    tmpDataUsage->totalUsage = strToInt(vcTokens[4]);
    pcDataUsage.push_back(tmpDataUsage);
    }
 
   getline(strmFile, vLine);
   continue;
   }
  }
    
 strmFile.close();

 cout << "Collecting data usage is done!" << endl;
}


/**
 *  collecting a co-training view's data usage
 */
void collectCTDataUsage(string &pLogFile, vector<DataUsage *> &pcDataUsage,
                        short pFeatureView)
{
 ifstream strmFile (pLogFile.c_str());

 if(!strmFile)
 {
  cerr << "\nCan't open log file: " << pLogFile << endl;
  return;
  }
 else 
  cout << "\nProcessing log file..." << endl;
 
 while (!strmFile.eof())
 {
  string vLine;
  vector<string> vcTokens;
   
  getline(strmFile, vLine);
  
  if (vLine.find("Training base classifier " + intToStr(pFeatureView)) != string::npos)
  {
   DataUsage *tmpDataUsage = new DataUsage;
 
   getline(strmFile, vLine);
   // this for in progress log files that are not complete yet
   if (strmFile.eof())
    continue;

   vcTokens.clear();
   tokenize(vLine, vcTokens, " ");
 
   tmpDataUsage->trainUsage = strToInt(vcTokens[0]);
   tmpDataUsage->totalUsage = tmpDataUsage->trainUsage;
   pcDataUsage.push_back(tmpDataUsage);
   getline(strmFile, vLine);
   continue;
   }
   
  if ((vLine.substr(0, 9) == "Iteration") && (vLine.find("Pool") != string::npos))
  {
   // looking for the line starting with "Training ..." or "Data used ..." (which 
   // one reaches earlier)
   do 
    getline(strmFile, vLine);
   while ((vLine.substr(0, 8) != "Training") && (vLine.substr(0, 4) != "Data") &&
          !strmFile.eof());
   
   // this for in progress log files that are not complete yet
   if (strmFile.eof())
    continue;

   // if current line starts with "Training ...", look for "... view #" 
   if (vLine.substr(0, 8) == "Training")
    while (((vLine.find("view " + intToStr(pFeatureView)) == string::npos) ||
            (vLine.substr(0, 8) != "Training")) && 
           !strmFile.eof())
     getline(strmFile, vLine);
   // if current line starts with "Data ...", look for "... view #" 
   else if (vLine.substr(0, 4) == "Data")
    while (((vLine.find("view " + intToStr(pFeatureView)) == string::npos) ||
            (vLine.substr(0, 4) != "Data")) &&
           !strmFile.eof())
     getline(strmFile, vLine);
   
   // this for in progress log files that are not complete yet
   if (strmFile.eof())
    continue;

   vcTokens.clear();
   tokenize(vLine, vcTokens, " ");
    	
   DataUsage *tmpDataUsage = new DataUsage;

   // if current line starts with "Training", there has been training in this step
   if (vcTokens[0] == "Training") 
   {
    getline(strmFile, vLine);

    // this for in progress log files that are not complete yet
    if (strmFile.eof())
     continue;

    vcTokens.clear();
    tokenize(vLine, vcTokens, " ");
  
    tmpDataUsage->trainUsage = strToInt(vcTokens[0]);
    
    do 
     getline(strmFile, vLine);
    while (((vLine.substr(0, 4) != "Data") ||
            (vLine.find("view " + intToStr(pFeatureView)) == string::npos)) &&
           !strmFile.eof()); 

    // this for in progress log files that are not complete yet
    if (strmFile.eof())
     continue;

    vcTokens.clear();
    tokenize(vLine, vcTokens, " ");
    	
    tmpDataUsage->totalUsage = strToInt(vcTokens[4]);
    pcDataUsage.push_back(tmpDataUsage);
    vcTokens.clear();
    tokenize(vLine, vcTokens, " ");
    }
   // if current line starts with "Training", there was no training in 
   // this step, so use the previous trainData for trainData but fetch totalData
   else if (vcTokens[0] == "Data") 
   {
    tmpDataUsage->trainUsage = pcDataUsage.back()->trainUsage;
    tmpDataUsage->totalUsage = strToInt(vcTokens[4]);
    pcDataUsage.push_back(tmpDataUsage);
    }
 
   getline(strmFile, vLine);
   continue;
   }
  }
    
 strmFile.close();

 cout << "Collecting data usage is done!" << endl;
}


/**
 *  collecting all co-training all views' data usage
 *  useful with separate-training-set co-training
 */ 
void collectAllCTDataUsage(string &pLogFile, vector<vector<DataUsage *> > &pcDataUsages,
                           short pViewCount)
{
 for (short i = 1; i <= pViewCount; i++)
 {
  vector<DataUsage *> vcDataUsage;
  cout << "\nCollecting data usage for view " << i;
  collectCTDataUsage(pLogFile, vcDataUsage, i);
  pcDataUsages.push_back(vcDataUsage);
  }
}


void collectScores(string &pSourceDir,vector<string> &pScoreFiles, 
                   vector<Scores *> &pScores)
{
 for (vector<string>::iterator itFile= pScoreFiles.begin();
      itFile < pScoreFiles.end();
      itFile++)
 {
  string tmpScoreFile = pSourceDir + "/" + (*itFile).c_str();
  ifstream strmFile (tmpScoreFile.c_str());
     
  if(!strmFile)
  {
   cerr << "Can't open file: " << tmpScoreFile << endl;
   return;
   }
  else 
   cout << "\nProcessing file: " << tmpScoreFile;
  
  while (!strmFile.eof())
  {
   string vLine;
   vector<string> tmpTokens;
   
   getline(strmFile, vLine);
   //cout << vLine << endl;
   if (vLine.find("Overall") != string::npos)
   {
    tokenize(vLine, tmpTokens, " \t\n\r");
    
    Scores *tmpScores = new Scores;
    tmpScores->it = strToInt((*itFile).substr((*itFile).find_last_of('.') + 1));
    tmpScores->precision = tmpTokens[4];
    tmpScores->recall = tmpTokens[5];
    tmpScores->f1 = tmpTokens[6];
    
    pScores.push_back(tmpScores);
    break;
    }
   }
    
  strmFile.close();
  } 

 cout << "\nProcessing score files is done!" << endl << endl;
}


void writeScores(Parameter &pParams, vector<vector<DataUsage *> > &pcDataUsages, 
                 vector<Scores *> &pScores, ofstream &pOutputStrm)
{
 pOutputStrm << "app" << "\t"
             << "c" << "\t" 
             << "s" << "\t" 
             << "u" << "\t" 
             << "mxp" << "\t" 
             << "mxn" << "\t"
             << "go" << "\t" 
             << "fs" << "\t" 
             << "mi" << "\t" 
             << "mp" << "\t" 
             << "g" << "\t" 
             << "p" << "\t" 
             << "sc" << "\t" 
             << "cl" << "\t" 
             << "pv" << "\t" 
             << "pu" << "\t" 
             << "pt" << "\t" 
             << "at" << "\t" 
             << "nt" << "\t" 
             << "r" << "\t" 
             << "si" << "\t" 
             << "pq" << "\t" 
             << "ltw" << "\t" 
             << "lts" << "\t" 
             << "ltd" << "\t" 
             << "ltp" << "\t" 
             << "utd" << "\t" 
             << "utds" << endl;
             
 pOutputStrm << pParams.app << "\t" 
             << pParams.coMethod << "\t" 
             << pParams.seedSize << "\t" 
             << pParams.uSize << "\t" 
             << pParams.maxPositive << "\t" 
             << pParams.maxNegative << "\t"
             << pParams.globalOpt << "\t" 
             << pParams.featureSet << "\t" 
             << pParams.meIterations << "\t" 
             << pParams.mePEMethod << "\t" 
             << pParams.meGaussian << "\t" 
             << pParams.poolSize << "\t" 
             << pParams.selection << "\t" 
             << pParams.clSelection << "\t" 
             << pParams.preferedView << "\t" 
             << pParams.poolUsage << "\t" 
             << pParams.probThreshold << "\t" 
             << pParams.agreeThreshold << "\t" 
             << pParams.growthSize << "\t" 
             << pParams.removeLabeled << "\t" 
             << pParams.tIterations << "\t" 
             << pParams.poolQuality << "\t" 
             << pParams.lTW << "\t" 
             << pParams.lTS << "\t" 
             << pParams.lTD << "\t" 
             << pParams.lTP << "\t" 
             << pParams.uTD << "\t" 
             << pParams.uTDS << endl << endl;

 // for remembering the last scores to be used for iterations that there 
 // was no scores for
 unsigned short vLastScoreIdx;
 
 for (unsigned i = 0, j = 0; i < pcDataUsages[0].size(); i++)
 {
  if (pScores[j]->it == i)
  {
   vLastScoreIdx = j;
   if (j < pScores.size() - 1)
    j++;
   }
   
  pOutputStrm << i << "\t"; 
  for (unsigned k = 0; k < pcDataUsages.size(); k++)
   pOutputStrm << pcDataUsages[k][i]->trainUsage << "\t" 
               << pcDataUsages[k][i]->totalUsage << "\t";
  pOutputStrm << pScores[vLastScoreIdx]->precision << "\t"
              << pScores[vLastScoreIdx]->recall << "\t" 
              << pScores[vLastScoreIdx]->f1 
              << endl;
  
  cout << i << "\t"; 
  for (unsigned k = 0; k < pcDataUsages.size(); k++)
   cout << pcDataUsages[k][i]->trainUsage << "\t" 
        << pcDataUsages[k][i]->totalUsage << "\t";
  cout << pScores[vLastScoreIdx]->precision << "\t"
       << pScores[vLastScoreIdx]->recall << "\t" 
       << pScores[vLastScoreIdx]->f1 
       << endl;

  }  
}

bool isOlderScoreFile(string p1stFile, string p2ndFile)
{
 if (strToInt(extractFileExt(p1stFile, 1)) < strToInt(extractFileExt(p2ndFile, 1)))
  return true;
 else
  return false;
}
