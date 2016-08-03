/**
 *  @ RSK
 *  @ Version 0.1        14.01.2010
 * 
 *  excutable file to call Charniak parser to parse all files in a given
 *  directory, which are in Charniak input format
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


bool parseDir(const string &, const string, const string, const string &, bool, const string &, ostream &);
bool parseFile(const string &, const string, const string, const string &, const string &);

static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][..]" << endl
      << "<-p[Parser path]>" << endl
      << "<-i[Source directory]>" << endl
      << "<-o[Output directory]>" << endl
      << "<-s(Include subdirectories)>" << endl
      << "<-k(Parse with existing tokenization)>" << endl
      << "<-s(Continue with the next sentence when failed (-S option for first-stage of Charniak))>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 string vParserPath = "../../packages/Charniak";
 string vSourceDir;
 string vOutputDir;
 bool vSubDirs = false;
 string vTokenize;
 string vContOnFail;
 
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
   
  if ((vParam == "k") || (vParam == "K"))
  { 
   vTokenize = "-K";
   continue;
   }

  if (vParam == "c")
  { 
   vContOnFail = "-S";
   continue;
   }

  }   


 /**
  *  processing source directory, and all subdirectories if the vSubDirs
  *  option is set, to parse all included files
  */
 
 string vLogFile = vOutputDir + "/" + "parsing.log";
 ofstream strmLog (vLogFile.c_str());
 if(!strmLog)
 {
  cerr << "\nCan't create log file: " << vLogFile << endl;
  return -1;
  }
 
 time_t vStartTime, vEndTime;
 time(&vStartTime);

 cout << "\nParsing started at " << getLocalTimeStr() << " ..." << endl;
 if (parseDir(vParserPath, vTokenize, vContOnFail, vSourceDir, vSubDirs, vOutputDir, strmLog))
 {
  time(&vEndTime);
  double vElapsedTime = difftime(vEndTime, vStartTime);
  
  cout << "Parsing is done at " << getLocalTimeStr() 
       << "! (" << convertToHMS(vElapsedTime) << ")" << endl;
  }
 else 
  cerr << "\nParsing was unsuccessful! (" << getLocalTimeStr() << ")"
       << endl << endl;
 
}

/**
 * 
 *  Function definitions
 * 
 */ 

/**
 *  a recurseive function to navigate directory structure to parse content
 * 
 *  It verifies all directories and files inside the passed pSourceDir
 *  and recursively calls itself for directories and calls parseFile()
 *  for files.
 */
bool parseDir(const string &pParserPath, const string pTokenize, const string pContOnFail, 
              const string &pSourceDir, bool pSubDirs, const string &pOutputDir,
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
     time_t vStartTime, vEndTime;
     time(&vStartTime);

     cout << "\nEntering directory: /" << vDirEntry->d_name << " at "
          << getLocalTimeStr() << endl;
     pLogStream << "\nEntering directory: /" << vDirEntry->d_name << " at "
                << getLocalTimeStr() << endl;

     string vOutputDir = pOutputDir + "/" + vDirEntry->d_name;
     if (mkdir(vOutputDir.c_str(), 0777) != 0)
     {
      cerr << "Can't create directory: " 
           << pOutputDir + "/" + vDirEntry->d_name << endl;
      return false;
      }
      
     parseDir(pParserPath, pTokenize, pContOnFail, pSourceDir + "/" + vDirEntry->d_name,
              pSubDirs, vOutputDir, pLogStream);
                
     time(&vEndTime);
     double vElapsedTime = difftime(vEndTime, vStartTime);
     
     cout << "\nParsing directory /" << vDirEntry->d_name << " is done at " 
          << getLocalTimeStr() << "! (" << convertToHMS(vElapsedTime) << ")" << endl;
     pLogStream << "\nParsing directory /" << vDirEntry->d_name << " is done at " 
                << getLocalTimeStr() << "! (" << convertToHMS(vElapsedTime) << ")" << endl;
     }
    else if ((!vIsDir) && (extractFileExt(vDirEntry->d_name, 2) == "cha.input"))
    {
     time_t vStartTime, vEndTime;
     time(&vStartTime);

     if (!parseFile(pParserPath, pTokenize, pContOnFail, pSourceDir + "/" + vDirEntry->d_name, pOutputDir))
      return false;
                
     time(&vEndTime);
     double vElapsedTime = difftime(vEndTime, vStartTime);
     
     cout << "\nParsing "<< vDirEntry->d_name << " is done in "
          << convertToHMS(vElapsedTime) << endl;
     pLogStream << "\nParsing "<< vDirEntry->d_name << " is done in "
                << convertToHMS(vElapsedTime) << endl;
     }
    }
   } 
  closedir(vDir);
  }
 else 
  return false;
     
 return true;
}

bool parseFile(const string &pParserPath, const string pTokenize, const string pContOnFail, 
               const string &pFile, const string &pOutputDir)
{
 string vParseFile = (pOutputDir + "/" + 
                      extractFileName(pFile, true) + ".parse.chare");
 
 string vParserCmd = pParserPath + "/first-stage/PARSE/parseIt -l399 -N50 " +
                     pParserPath + "/first-stage/DATA/EN/ " + pFile + " " +
                     pTokenize + " " + pContOnFail + " | " +
                     pParserPath + "/second-stage/programs/features/best-parses -l " +
                     pParserPath + "/second-stage/models/ec50spfinal/features.gz " +
                     pParserPath + "/second-stage/models/ec50spfinal/cvlm-l1c10P1-weights.gz" +
                     " >> " + vParseFile;
 
 if (system(vParserCmd.c_str()) == 0)
  return true;
 else
  return false;
}
