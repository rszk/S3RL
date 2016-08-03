/**
 *  @ RSK
 *  @ Version 0.1        19.02.2010
 * 
 *  excutable file to convert Charniak (and other Penn Treebank) parses 
 *  to dependency parses using pennconverter.jar (Johansson & Negues 2007)
 *  downloadable from http://nlp.cs.lth.se/software/treebank_converter/
 * 
 *  The output is CoNLL-X format
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
 };

bool convertDir(const string &, const string &, bool, const string &, 
                const string &, corpusAnalysis &, ostream &);
bool convertFile(const string &, const string &, const string &,
                 corpusAnalysis &, ostream &);
void initAnalysisStruct(corpusAnalysis &);
void logCorpusAnalysis(corpusAnalysis &, ostream &);
void logDirAnalysis(corpusAnalysis &, ostream &, const string &);
void updateGlobalAnalysis(corpusAnalysis &, corpusAnalysis &, bool);

static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][..]" << endl
      << "<-p[Convertor path]>" << endl
      << "<-i[Source directory]>" << endl
      << "<-o[Output directory]>" << endl
      << "<-s(Include subdirectories)>" << endl
      << "<-m(Merge all into one file)>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 string vConvertorPath = "../../packages/PennConverter";
 string vSourceDir;
 string vOutputDir;
 bool vSubDirs = false;
 bool vMerg = false;
 
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
  vOutputFile = (vOutputDir + "/" + "dependency.jn");
 
 cout << "\nConversion started ... " << endl;
 if (convertDir(vConvertorPath, vSourceDir, vSubDirs, 
                vOutputDir, vOutputFile, vCorpusAnalysis, strmLog))
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
                corpusAnalysis &pCAnalysis, ostream &pLogStream)
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
                vOutputDir, pOutputFile, vDirAnalysis, pLogStream);
                
     logDirAnalysis(vDirAnalysis, pLogStream, vDirEntry->d_name);
     logDirAnalysis(vDirAnalysis, cout, vDirEntry->d_name);
     updateGlobalAnalysis(pCAnalysis, vDirAnalysis, true);
     }
    else if ((!vIsDir) && (extractFileExt(vDirEntry->d_name, 2) == "chare.filtered"))
    {
     corpusAnalysis vFileAnalysis;
     initAnalysisStruct(vFileAnalysis);
     
     // if each output file for this directory should be separate, then
     // set output file name here
     if (pOutputFile.empty())
     {
      string vOutputFile = (pOutputDir + "/dependency." + 
                            extractFileName(pSourceDir + "/" + vDirEntry->d_name, false) +
                            ".jn");

      if (!convertFile(pConvertorPath, pSourceDir + "/" + vDirEntry->d_name, 
                       vOutputFile, vFileAnalysis, pLogStream))
       return false;
      }
     else 
     {
      if (!convertFile(pConvertorPath, pSourceDir + "/" + vDirEntry->d_name, 
                       pOutputFile, vFileAnalysis, pLogStream))
       return false;
      }
                
     pLogStream << endl << pSourceDir << "/" << vDirEntry->d_name;
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
                 const string &pOutputFile, corpusAnalysis &pCAnalysis,
                 ostream &pLogStream)
{
 string vConvertCmd = "cd " + pConvertorPath + "; " +
                       "java -jar pennconverter.jar -raw -splitSlash=false <" +
                       pParseFile +
                       " >> " + pOutputFile;
 
 if (system(vConvertCmd.c_str()) != 0)
  return false;
  
 return true;
}

void initAnalysisStruct(corpusAnalysis &pCA)
{
 pCA.fileCount = 0;
 pCA.dirCount = 0;
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
}

void logCorpusAnalysis(corpusAnalysis &pCA, ostream &pStream)
{
 pStream << endl
         << pCA.fileCount << " files in " << pCA.dirCount << " directories" << endl;
}

void logDirAnalysis(corpusAnalysis &pDA, ostream &pStream, const string &pDir)
{
 pStream << endl
         << pDA.fileCount << " files in /" << pDir << endl;
}
