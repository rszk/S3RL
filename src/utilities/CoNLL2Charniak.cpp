/**
 *  @ RSK
 *  @ Version 0.1        20.02.2010
 * 
 *  excutable file to convert sentences extracted form row-based CoNLL 2005
 *  or any other CoNLL files to the input format of Charniak Parser.
 *  
 *  The program treats CoNLL quotation (``, ''), which make problems with
 *  parsing, by changing them to original quotation marks (")
 *  
 *  The output of the program can be separate Charniak input files for each
 *  text file or a merged Charniak input files for each directory based on
 *  the executable argument
 * 
 */

#include <iostream>
#include <fstream>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>

#include "Utils.hpp"

using namespace std;


struct corpusAnalysis
{
 unsigned int fileCount;
 unsigned int dirCount;
 unsigned int correctedSntncs;
 };

bool processDir(const string &, bool, bool, const string &, ostream &, 
                bool, corpusAnalysis &, ostream &);
bool processFile(const string &, bool, ostream &, corpusAnalysis &);
bool processSentence(string &, bool);
void initAnalysisStruct(corpusAnalysis &);
void logCorpusAnalysis(corpusAnalysis &, ostream &);
void logDirAnalysis(corpusAnalysis &, ostream &, const string &);
void logFileAnalysis(corpusAnalysis &, ostream &);
void updateGlobalAnalysis(corpusAnalysis &, corpusAnalysis &, bool);


static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][..]" << endl
      << "<-i[Source directory]>" << endl
      << "<-o[Output directory]>" << endl
      << "<-s[Include subdirectories]>" << endl
      << "<-m[Merged file per directory]>" << endl
      << "<-c[Correct problematic tokens]>"
      <<  endl;
}

int main(int argc, char ** argv)
{
 string vSourceDir = "../../corpus/CoNLL/original";
 string vOutputDir = "../../corpus/CoNLL/Charniak/input";
 bool vSubDirs = false;
 bool vMerg = false;
 bool vCorrect = false;
 
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
  
  if (vParam == "i")
  { 
   vSourceDir = argv[cntArg + 1];
   continue;
   }
  
  if (vParam == "o")
  { 
   vOutputDir = argv[cntArg + 1];
   continue;
   }

  if (vParam == "s")
  { 
   vSubDirs = true;
   continue;
   }
   
  if (vParam == "m")
  { 
   vMerg = true;
   continue;
   }

  if (vParam == "c")
  { 
   vCorrect = true;
   continue;
   }

  }   
 
 /**
  *  processing source directory, and all subdirectories if the vSubDirs
  *  option is set, to convert all included files
  */
 
 string vLogFile = vOutputDir + "/" + "CorpusStat.log";
 ofstream strmLog (vLogFile.c_str());
 if(!strmLog)
 {
  cerr << "\nCan't create log file: " << vLogFile << endl;
  return -1;
  }
 
 corpusAnalysis vCorpusAnalysis;
 initAnalysisStruct(vCorpusAnalysis);
 
 ofstream strmCharniak;
 if (vMerg)
 {
  string pCharniakFile = (vOutputDir + "/" + extractFileName(vSourceDir, false) + ".cha.input");
  strmCharniak.open(pCharniakFile.c_str());
  if(!strmCharniak)
  {
   cerr << "\nCan't create output file: " << pCharniakFile << endl;
   return -1;
   }
  }
 
 cout << "\nProcess started ..." << endl;
 if (processDir(vSourceDir, vSubDirs, vMerg, vOutputDir, strmCharniak,
                vCorrect, vCorpusAnalysis, strmLog))
  cout << "Process is done!" << endl;
 else 
  cerr << "\nProcess was unsuccessful! Please check directories and files"
       << endl << endl;
 
 logCorpusAnalysis(vCorpusAnalysis, strmLog);
 logCorpusAnalysis(vCorpusAnalysis, cout);
}

/**
 * 
 *  Function definitions
 * 
 */ 

/**
 *  a recurseive funstion to process directory structure
 * 
 *  It verifies all directories and files inside the passed pSourceDir
 *  and recursively calls itself for directories and calls processFile()
 *  for files. It also logs and collects analysis statistics of every 
 *  folder and file into the global corpus analysis
 * 
 */
bool processDir(const string &pSourceDir, bool pSubDirs, bool pMerg,
                const string &pOutputDir, ostream &pCharniakStream,
                bool pCorrect, corpusAnalysis &pCAnalysis, ostream &pLogStream)
{
 DIR *vDir;
 struct dirent *vDirEntry; 
 vDir = opendir(pSourceDir.c_str());
 
 if (vDir != NULL)
 {
  while ((vDirEntry = readdir(vDir)))
  {
   if ((strcmp(vDirEntry->d_name, ".") == 0) ||
       (strcmp(vDirEntry->d_name, "..") == 0))
    continue;
   else
   {
   	bool vIsDir = isDir(pSourceDir + "/" + vDirEntry->d_name); 
    if ((vIsDir) && (pSubDirs))
    {
     cout << "\nEntering directory: /" << vDirEntry->d_name << endl;
     pLogStream << "\nDirectory: /" << vDirEntry->d_name << endl;
  
     corpusAnalysis vDirAnalysis;
     initAnalysisStruct(vDirAnalysis);
     
     string vOutputDir = pOutputDir + "/" + vDirEntry->d_name;
     if (mkdir(vOutputDir.c_str(), 0777) != 0)
     {
      cerr << "Can't create directory: " 
           << pOutputDir + "/" + vDirEntry->d_name << endl;
      return false;
      }
     
     ofstream strmCharniak;
     // if all output files for this directory should be merged, then
     // create stream here and send to processDir() which will process
     // the files of this directory
     if (pMerg)
     {
      string pCharniakFile = (vOutputDir + "/" + vDirEntry->d_name + ".cha.input");
      strmCharniak.open(pCharniakFile.c_str());
      if(!strmCharniak)
      {
       cerr << "\nCan't create output file: " << pCharniakFile << endl;
       return -1;
       }
      }

     processDir(pSourceDir + "/" + vDirEntry->d_name, pSubDirs, pMerg, 
                vOutputDir, strmCharniak, pCorrect, vDirAnalysis, pLogStream);
                
     logDirAnalysis(vDirAnalysis, pLogStream, vDirEntry->d_name);
     logDirAnalysis(vDirAnalysis, cout, vDirEntry->d_name);
     updateGlobalAnalysis(pCAnalysis, vDirAnalysis, true);
     }
    else if (!vIsDir)
    {
     corpusAnalysis vFileAnalysis;
     initAnalysisStruct(vFileAnalysis);
     
     // if each output file for this directory should be separate, then
     // create stream here
     if (pMerg)
     {
      if (!processFile(pSourceDir + "/" + vDirEntry->d_name,
                       pCorrect, pCharniakStream, vFileAnalysis))
       return false;
      }
     else 
     {
      string pCharniakFile = (pOutputDir + "/" + 
                              extractFileName(pSourceDir + "/" + vDirEntry->d_name, true) +
                              ".cha.input");
      ofstream strmCharniak (pCharniakFile.c_str());
      if(!strmCharniak)
      {
       cerr << "\nCan't create output file: " << pCharniakFile << endl;
       return -1;
       }

      if (!processFile(pSourceDir + "/" + vDirEntry->d_name,
                       pCorrect, strmCharniak, vFileAnalysis))
       return false;
      }

     pLogStream << endl << pSourceDir << "/" << vDirEntry->d_name;
     logFileAnalysis(vFileAnalysis, pLogStream);
     logFileAnalysis(vFileAnalysis, cout);
     updateGlobalAnalysis(pCAnalysis, vFileAnalysis, false);
     }
    }
   } 
  closedir(vDir);
  }
 else 
  return false;
     
 return true;
}

/**
 *  extracts sentences from a file, processes them by processSentence(),
 *  and add to existing Charniak input file or a new file with the same 
 *  name based on the value of pAppend parameter
 */
bool processFile(const string &pFile, bool pCorrect, ostream &pStream, 
                 corpusAnalysis &pCAnalysis)
{
 string vLine;
 unsigned int cntSentence = 0;
 
 ifstream strmFile (pFile.c_str());
     
 if(!strmFile)
 {
  cerr << "Can't open file: " << pFile << endl;
  return false;
  }
 else 
  cout << "\nProcessing file: " << pFile << endl;
 
 // looping to read sentences
 while (!strmFile.eof())
 {
  string vLine;
  getline(strmFile, vLine);
  if (vLine.empty())
   return true; 
  
  if (processSentence(vLine, pCorrect))
   pCAnalysis.correctedSntncs++;
  pStream << vLine << endl;
  
  cntSentence++;
  cout << "\r" << cntSentence << " sentences processed" << flush;
  }
 
 strmFile.close();
 return true;
}

/**
 *  processes the extracted sentence and corrects the known problems, then
 *  returns true if any correction was made
 */

bool processSentence(string &pSentence, bool pCorrect)
{
 size_t tmpPosition = 0;
 bool vResult = false;
 
 if (pCorrect)
 {
  // replacing any ``  with "
  while (true)
  {
   tmpPosition = pSentence.find("``");
   if (tmpPosition != string::npos)
   {
    pSentence.replace(tmpPosition, 2, "\"");
    vResult = true;
    }
   else
    break; 
   }
 
  // replacing any ''  with "
  while (true)
  {
   tmpPosition = pSentence.find("''");
   if (tmpPosition != string::npos)
   {
    pSentence.replace(tmpPosition, 2, "\"");
    vResult = true;
    }
   else
    break; 
   }
  }
 
 pSentence = "<s> " + pSentence + " </s>";
 return vResult; 
}

void initAnalysisStruct(corpusAnalysis &pCA)
{
 pCA.fileCount = 0;
 pCA.dirCount = 0;
 pCA.correctedSntncs = 0;
}

void updateGlobalAnalysis(corpusAnalysis &pGlobalAnalysis,
                          corpusAnalysis &pLocalAnalysis,
                          bool pDir)
{
 if (pDir)
 {
  pGlobalAnalysis.dirCount += (pLocalAnalysis.dirCount + 1);
  pGlobalAnalysis.fileCount += pLocalAnalysis.fileCount;
  }
 else 
  pGlobalAnalysis.fileCount++;
 pGlobalAnalysis.correctedSntncs += pLocalAnalysis.correctedSntncs;
}

void logCorpusAnalysis(corpusAnalysis &pCA, ostream &pStream)
{
 pStream << endl
         << pCA.fileCount << " files in " << pCA.dirCount << " directories" << endl
         << pCA.correctedSntncs << " sentences corrected" << endl; 
}

void logDirAnalysis(corpusAnalysis &pDA, ostream &pStream, const string &pDir)
{
 pStream << endl
         << pDA.fileCount << " files in /" << pDir << endl
         << pDA.correctedSntncs << " sentences corrected" << endl;
}

void logFileAnalysis(corpusAnalysis &pFA, ostream &pStream)
{
 pStream << endl
         << pFA.correctedSntncs << " sentences corrected" << endl; 
}
