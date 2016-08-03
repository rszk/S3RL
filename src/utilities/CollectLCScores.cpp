/**
 *  @ RSK
 *  @ Version 0.1        03.05.2010
 * 
 *  excutable file to read CoNLL evaluation score files for learning curve 
 *  and store precision, recall, and F1 of each file in a CVS style file
 *  (1 row per file) to be imported by graph plotters (e.g. Excel) or other uses
 * 
 *  All evaluation score file names must end with ".score.#" pattern where # 
 *  is the amount of data used for training
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
 string sizeSeri;
 string featureSet;
 string globalOpt;
 string meIterations;
 string mePEMethod;
 string meGaussian;
 string ltw;
 string lts;
 string ltd;
 string ltp;
 string twsjw;
 string twsjs;
 string twsjd;
 string twsjp;
 string dwsjw;
 string dwsjs;
 string dwsjd;
 string dwsjp;
 string tbrww;
 string tbrws;
 string tbrwd;
 string tbrwp;
};
  
struct Scores
{
 unsigned size;
 string precision;
 string recall;
 string f1;
};
  
void collectFiles(vector<string> &, string &, string &);
void collectScores(string &, vector<string> &, vector<Scores *> &);
void collectParameters(string &, Parameter &);
void writeScores(Parameter &, vector<Scores *> &, ofstream &);
bool isOlderScoreFile(string, string);

static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][..]" << endl
      << "<-si [Score source directory]>" << endl
      << "<-o  [Output directory]>" << endl
      << "<-sp [Source file name prefix (e.g. test.wsj)]>" << endl
      << "<-lf [Log file name (containing learning curve parameters)]>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 string vScoreSrcDir = "../../results";
 string vOutputDir = "../../analysis";
 string vScoreFilePrefix = "devel.24";
 string vLogFile;

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

  }   
 
 if (vLogFile.empty())
 {
  cerr << "\nLog file not specified!" << endl;
  return -1;
  }
   
 Parameter vParams;
 
 // collecting parameters
 collectParameters(vLogFile, vParams);


 vector<string> vScoreFiles;

 // collecting and sorting score files
 vScoreFilePrefix += ".score.";
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
 writeScores(vParams, vcScores, strmOutput);
 
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
 ifstream strmFile (pLogFile.c_str());
     
 if(!strmFile)
 {
  cerr << "Can't open file: " << pLogFile << endl;
  return;
  }
 else 
  cout << "\nProcessing parameters..." << endl;
  
 while (!strmFile.eof())
 {
  string vParam;
  getline(strmFile, vParam);
  vector<string> vcParam;
  vcParam.clear();
  tokenize(vParam, vcParam, " \t");
  
  if (vcParam.size() == 0)
   break;
  
  if (vcParam[0] == "-ss")
  { 
   pParams.sizeSeri = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-fs")
  { 
   pParams.featureSet = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-go")
  { 
   pParams.globalOpt = vcParam[1];
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

  if (vcParam[0] == "-ltw")
  { 
   pParams.ltw = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-lts")
  { 
   pParams.lts = vcParam[1];
   continue; 
   }
   
  if (vcParam[0] == "-ltd")
  { 
   pParams.ltd = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-ltp")
  { 
   pParams.ltp = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-twsjw")
  { 
   pParams.twsjw = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-twsjs")
  { 
   pParams.twsjs = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-twsjd")
  { 
   pParams.twsjd = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-twsjp")
  { 
   pParams.twsjp = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-dwsjw")
  { 
   pParams.dwsjw = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-dwsjs")
  { 
   pParams.dwsjs = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-dwsjd")
  { 
   pParams.dwsjd = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-dwsjp")
  { 
   pParams.dwsjp = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-tbrww")
  { 
   pParams.tbrww = vcParam[1];
   continue;
   }
   
  if (vcParam[0] == "-tbrws")
  { 
   pParams.tbrws = vcParam[1];
   continue;
   }

  if (vcParam[0] == "-tbrwd")
  { 
   pParams.tbrwd = vcParam[1];
   continue; 
   }

  if (vcParam[0] == "-tbrwp")
  { 
   pParams.tbrwp = vcParam[1];
   continue; 
   }

  }
 
 cout << "Collecting parameters is done!" << endl;
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
    tmpScores->size = strToInt((*itFile).substr((*itFile).find_last_of('.') + 1));
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


void writeScores(Parameter &pParams, vector<Scores *> &pScores, ofstream &pOutputStrm)
{
 pOutputStrm << "ss" << "\t"
             << "fs" << "\t" 
             << "go" << "\t" 
             << "mi" << "\t" 
             << "mp" << "\t" 
             << "g" << "\t"
             << "ltw" << "\t" 
             << "lts" << "\t" 
             << "ltd" << "\t" 
             << "ltp" << "\t" 
             << "twsjw" << "\t" 
             << "twsjs" << "\t" 
             << "twsjd" << "\t" 
             << "twsjp" << "\t" 
             << "dwsjw" << "\t" 
             << "dwsjs" << "\t" 
             << "dwsjd" << "\t" 
             << "dwsjp" << "\t" 
             << "tbrww" << "\t" 
             << "tbrws" << "\t" 
             << "tbrwd" << "\t" 
             << "tbrwp" << endl;
             
 pOutputStrm << pParams.sizeSeri << "\t" 
             << pParams.featureSet << "\t" 
             << pParams.globalOpt << "\t" 
             << pParams.meIterations << "\t" 
             << pParams.mePEMethod << "\t" 
             << pParams.meGaussian << "\t"
             << pParams.ltw << "\t" 
             << pParams.lts << "\t" 
             << pParams.ltd << "\t" 
             << pParams.ltp << "\t" 
             << pParams.twsjw << "\t" 
             << pParams.twsjs << "\t" 
             << pParams.twsjd << "\t" 
             << pParams.twsjp << "\t" 
             << pParams.dwsjw << "\t" 
             << pParams.dwsjs << "\t" 
             << pParams.dwsjd << "\t" 
             << pParams.dwsjp << "\t" 
             << pParams.tbrww << "\t" 
             << pParams.tbrws << "\t" 
             << pParams.tbrwd << "\t" 
             << pParams.tbrwp << endl << endl;

 for (unsigned i = 0; i < pScores.size(); i++)
 {
  pOutputStrm << i + 1 << "\t"
              << pScores[i]->size << "\t"
              << pScores[i]->precision << "\t"
              << pScores[i]->recall << "\t" 
              << pScores[i]->f1 
              << endl;
  
  cout << i + 1 << "\t"
       << pScores[i]->size << "\t"
       << pScores[i]->precision << "\t"
       << pScores[i]->recall << "\t" 
       << pScores[i]->f1 
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
