/**
 *  @ RSK
 *  @ Version 0.1        14.01.2010
 * 
 *  excutable file to convert Charniak parses to CoNLL column-based format,
 *  while applying some filters on parsed sentences
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
 unsigned int processedSntncCount;
 unsigned int filteredSntncCount;
 unsigned int convertedSntncCount;
 };

bool convertDir(const string &, const string &, bool, const string &, 
                const string &, const string &, bool, corpusAnalysis &, ostream &);
bool convertFile(const string &, const string &, const string &,
                 bool, corpusAnalysis &, ostream &);
bool processFile(const string &, string &, bool, unsigned int &, unsigned int &);
void initAnalysisStruct(corpusAnalysis &);
void logCorpusAnalysis(corpusAnalysis &, ostream &);
void logDirAnalysis(corpusAnalysis &, ostream &, const string &);
void logFileAnalysis(corpusAnalysis &, ostream &);
void updateGlobalAnalysis(corpusAnalysis &, corpusAnalysis &, bool);

static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][..]" << endl
      << "<-p[Converter path]>" << endl
      << "<-i[Source directory]>" << endl
      << "<-o[Output directory]>" << endl
      << "<-s(Include subdirectories)>" << endl
      << "<-x[Output files extention]>" << endl
      << "<-m[Merge all into one file; the name of the merged file]>" << endl
      << "<-f(Do filtering)>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 string vConvertorPath = "../../packages/CoNLL";
 string vSourceDir;
 string vOutputDir;
 string vOutExt = "conll";
 string vMergedFile;
 bool vSubDirs = false;
 bool vMerg = false;
 bool vFilter = false;
 
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
   
  if (vParam == "x")
  { 
   vOutExt = argv[cntArg + 1];
   continue;
   }

  if (vParam == "m")
  { 
   vMerg = true;
   vMergedFile = argv[cntArg + 1];
   continue;
   }

  if (vParam == "f")
  { 
   vFilter = true;
   continue;
   }

  }   
 
 /**
  *  processing source directory, and all subdirectories if the vSubDirs
  *  option is set, to convert all included files
  */
 
 string vLogFile = vOutputDir + "/" + "conversion.log";
 ofstream strmLog (vLogFile.c_str());
 if(!strmLog)
 {
  cerr << "\nCan't create log file: " << vLogFile << endl;
  return -1;
  }
 
 corpusAnalysis vCorpusAnalysis;
 initAnalysisStruct(vCorpusAnalysis);
 
 string vOutputFile;
 if (vMerg)
  vOutputFile = (vOutputDir + "/" + vMergedFile);
 
 cout << "\nConversion started ... " << endl;
 if (convertDir(vConvertorPath, vSourceDir, vSubDirs, vOutputDir, 
                vOutputFile, vOutExt, vFilter, vCorpusAnalysis, strmLog))
 {
  cout << endl << "Conversion is done!";
  logCorpusAnalysis(vCorpusAnalysis, strmLog);
  logCorpusAnalysis(vCorpusAnalysis, cout);
  cout << endl;
  }
 else 
  cerr << "\nConversion was unsuccessful!"
       << endl << endl;
 
}

/**
 * 
 *  Function definitions
 * 
 */ 

/**
 *  a recurseive function to navigate directory structure to convert content
 * 
 *  It verifies all directories and files inside the passed pSourceDir
 *  and recursively calls itself for directories and calls convertFile()
 *  for files.
 */
bool convertDir(const string &pConvertorPath, const string &pSourceDir, 
                bool pSubDirs, const string &pOutputDir, const string &pOutputFile,
                const string &pOutExt, bool pFilter, corpusAnalysis &pCAnalysis,
                ostream &pLogStream)
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
     pLogStream << "\nEntering directory: /" << vDirEntry->d_name << endl;

     corpusAnalysis vDirAnalysis;
     initAnalysisStruct(vDirAnalysis);
     
     string vOutputDir;
     // If all output files for this directory should not be merged (no
     // merged output file is specified), create the proper output directory
     if (pOutputFile.empty())
     {
      vOutputDir = pOutputDir + "/" + vDirEntry->d_name;
      if (mkdir(vOutputDir.c_str(), 0777) != 0)
      {
       cerr << "Can't create directory: " 
            << pOutputDir + "/" + vDirEntry->d_name << endl;
       return false;
       }
      }

     convertDir(pConvertorPath, pSourceDir + "/" + vDirEntry->d_name, pSubDirs, 
                vOutputDir, pOutputFile, pOutExt, pFilter, vDirAnalysis, pLogStream);
                
     logDirAnalysis(vDirAnalysis, pLogStream, vDirEntry->d_name);
     logDirAnalysis(vDirAnalysis, cout, vDirEntry->d_name);
     updateGlobalAnalysis(pCAnalysis, vDirAnalysis, true);
     }
    else if ((!vIsDir) && (extractFileExt(vDirEntry->d_name, 1) == "chare"))
    {
     corpusAnalysis vFileAnalysis;
     initAnalysisStruct(vFileAnalysis);
     
     // if each output file for this directory should be separate, then
     // set output file name here
     if (pOutputFile.empty())
     {
      string vOutputFile = (pOutputDir + "/" + 
                            extractFileName(pSourceDir + "/" + vDirEntry->d_name, true) +
                            "." + pOutExt);

      if (!convertFile(pConvertorPath, pSourceDir + "/" + vDirEntry->d_name, 
                       vOutputFile, pFilter, vFileAnalysis, pLogStream))
       return false;
      }
     else 
     {
      if (!convertFile(pConvertorPath, pSourceDir + "/" + vDirEntry->d_name, 
                       pOutputFile, pFilter, vFileAnalysis, pLogStream))
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
 *  opens parse file, processes sentences to filter non-full sentences 
 *  (those without VBx POS tag), and convert remaining
 */
bool convertFile(const string &pConvertorPath, const string &pParseFile, 
                 const string &pOutputFile, bool pFilter, 
                 corpusAnalysis &pCAnalysis, ostream &pLogStream)
{
 unsigned int vProcessedSntncCount = 0;
 unsigned int vFilteredSntncCount = 0;
 
 string vProcessedFile = pParseFile + ".filtered";
 
 if (processFile(pParseFile, vProcessedFile, pFilter, vProcessedSntncCount, vFilteredSntncCount))
 {
  pCAnalysis.processedSntncCount = vProcessedSntncCount;
  pCAnalysis.filteredSntncCount = vFilteredSntncCount;
  
  string vConvertCmd = "cd " + pConvertorPath + "; " +
                       "perl ./wsj-to-se.pl < " + vProcessedFile +
                       " >> " + pOutputFile;
 
  if (system(vConvertCmd.c_str()) == 0)
   pCAnalysis.convertedSntncCount = (vProcessedSntncCount - vFilteredSntncCount);
  else 
   return false;
  
//  remove(vProcessedFile.c_str());
  return true;
  }
 else
  return false;
}

bool processFile(const string &pParseFile, string &pProcessedFile, 
                 bool pFilter,
                 unsigned int &pProcessedSntncCount, 
                 unsigned int &pFilteredSntncCount)
{
 string vParse;
 
 ifstream strmParseFile (pParseFile.c_str());
     
 if(!strmParseFile)
 {
  cerr << "Can't open file: " << pParseFile << endl;
  return false;
  }
 else 
  cout << endl << pParseFile << " ..." << endl;
 
 ofstream strmProcessed (pProcessedFile.c_str());
 if(!strmProcessed)
 {
  cerr << "\nCan't create filtered parse file " << pProcessedFile << endl;
  return -1;
  }
 
 // looping to read lines
 while (!strmParseFile.eof())
 {
  getline(strmParseFile, vParse);
  if (vParse.empty())
   continue;
  
  // filter sentences without any VBx tag
  if ((pFilter) && (vParse.find("VB") == string::npos))
    pFilteredSntncCount++;
   else
    strmProcessed << vParse <<endl;
  
  pProcessedSntncCount++;
  } 

 return true;
}	

void initAnalysisStruct(corpusAnalysis &pCA)
{
 pCA.fileCount = 0;
 pCA.dirCount = 0;
 pCA.processedSntncCount = 0;
 pCA.convertedSntncCount = 0;
 pCA.filteredSntncCount = 0;
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
 pGlobalAnalysis.processedSntncCount += pLocalAnalysis.processedSntncCount;
 pGlobalAnalysis.convertedSntncCount += pLocalAnalysis.convertedSntncCount;
 pGlobalAnalysis.filteredSntncCount += pLocalAnalysis.filteredSntncCount;
}

void logCorpusAnalysis(corpusAnalysis &pCA, ostream &pStream)
{
 pStream << endl
         << pCA.fileCount << " files in " << pCA.dirCount << " directories" << endl
         << pCA.processedSntncCount << " processed sentences" << endl 
         << pCA.convertedSntncCount << " converted sentences" << endl 
         << pCA.filteredSntncCount << " filtered sentences" << endl;
}

void logDirAnalysis(corpusAnalysis &pDA, ostream &pStream, const string &pDir)
{
 pStream << endl
         << pDA.fileCount << " files in /" << pDir << endl
         << pDA.processedSntncCount << " processed sentences" << endl 
         << pDA.convertedSntncCount << " converted sentences" << endl 
         << pDA.filteredSntncCount << " filtered sentences" << endl;
}

void logFileAnalysis(corpusAnalysis &pFA, ostream &pStream)
{
 pStream << endl
         << pFA.processedSntncCount << " processed sentences" << endl 
         << pFA.convertedSntncCount << " converted sentences" << endl 
         << pFA.filteredSntncCount << " filtered sentences" << endl;
}
