/**
 *  @ RSK
 *  @ Version 1.0        06.05.2010
 * 
 *  excutable file for merging CoNLL annotations files into a single file
 * 
 *  Note:
 *  After the merging is done, remove the 2nd last line of merged file.
 * 
 */

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Constants.hpp"
#include "Utils.hpp"

using namespace std;


short MergFiles(vector<string> &, const string &);

static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][...]" << endl
      << " <-fl[List of CoNLL annotation files separated by comma]>" << endl
      << " <-df[Destination file name (considered in labeled dir)]>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 string vDefaultPath = DEFAULT_CONLL_LABELED_PATH;
 vector<string> vcSrcFiles;
 string vDestFile = vDefaultPath + "/" + "merged";
 
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
  
  if (vParam == "fl")
  { 
   string vSrcFiles = argv[cntArg + 1];
   tokenize(vSrcFiles, vcSrcFiles, ",;");
   continue;
   }
   
  if (vParam == "df")
  { 
   vDestFile = argv[cntArg + 1];
   vDestFile = vDefaultPath + "/" + vDestFile;
   continue;
   }
  }

 for (unsigned short i = 0; i < vcSrcFiles.size(); i++)
  vcSrcFiles[i] = vDefaultPath + "/" + vcSrcFiles[i];
 
 cout << "\nMerging files ..." << endl;
 if (MergFiles(vcSrcFiles, vDestFile) == 0)
  cout << "\nMerging files is done!" << endl;
 else 
 {
  cerr << "\n\nMerging files was not sucessful!" << endl << endl;
  exit(-1);
  }

 cout << endl;
}

/**
 * 
 *  Function definitions
 * 
 */ 

short MergFiles(vector<string> &pSrcFiles,
                const string &pDestFile)
{
 vector<ifstream *> strmSources;
 
 for (unsigned short i = 0; i < pSrcFiles.size(); i++)
 {
  ifstream *tmpStrm = new ifstream;
  tmpStrm->open(pSrcFiles[i].c_str());
  strmSources.push_back(tmpStrm);

  if(!strmSources[i])
  {
   cerr << "Can't open file: " << pSrcFiles[i] << endl;
   return -1;
   }
  }

 ofstream strmDest (pDestFile.c_str());
 if(!strmDest)
 {
  cerr << "\nCan't create output file: " << pDestFile << endl;
  return -1;
  }
 
 vector<string> vcSrcLines(strmSources.size(), "");
 size_t cntLines = 0;
 
 // looping to read files; only one of the streams is checked for eof
 while (!strmSources[0]->eof())
 {
  // reading the 1st line (in CoNLL format) of a sentence from files
  for (unsigned short i = 0; i < strmSources.size(); i++)
  {
   string tmpLine;
   getline(*strmSources[i], tmpLine);
   vcSrcLines[i] = tmpLine;
   }
  
  // controlling CoNLL files consistency; base is the 1st file
  if (isEmptyLine(vcSrcLines[0]))
   for (unsigned short i = 1; i < strmSources.size(); i++)
    if ((!isEmptyLine(vcSrcLines[i])))
    {
     cerr << "There is a mismatch between first and " << i << "th file"
          << cntLines << endl;
     return 0;
     }
  
  // an empty line after each sentence
  for (unsigned short i = 0; i < vcSrcLines.size(); i++)
   trimLine(vcSrcLines[i]);
  
  strmDest << vcSrcLines[0];
  for (unsigned short i = 1; i < vcSrcLines.size(); i++)
   strmDest << "\t" << vcSrcLines[i];
  strmDest << endl;

  cntLines++;
  cout << "\r" << cntLines << " Lines merged" << flush;
  }
 
 return 0;
}
