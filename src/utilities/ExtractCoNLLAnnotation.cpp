/**
 *  @ RSK
 *  @ Version 0.1        19.02.2010
 * 
 *  excutable file extracting annotation columns or rows specified as 
 *  arguments (e.g. 2,3,4 or T) from a CoNLL-X file (e.g. MaltParser output) 
 *  or CoNLL 2005 row-based file into a separate file
 * 
 */

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Constants.hpp"
#include "Utils.hpp"

using namespace std;


short ExtractCols(const string &, vector<unsigned short> &, 
                  unsigned short, const string &);
short ExtractRows(const string &, char, bool,const string &);

static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][...]" << endl
      << " <-i[Input CoNLL x file]>" << endl
      << " <-o[Output file]>" << endl
      << " <-c[Input file is columnar and requested annotaion column numbers are ... (e.g. 2,5,6)]>" << endl
      << " <-s[Space characters number between columns in output]>" << endl
      << " <-r[Input file is row-based and requested annotaion row type is ... (W: words, P: POS, T: parse trees, R: semantic roles)]>" << endl
      << " <-b[Separate sentences with a blank line (valid when -r is used and useful when extracting R)>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 string vSourceFile;
 bool vColumn;
 string vDestFile;
 vector<unsigned short> vcCols;
 unsigned short vColSpace = 10;
 char vRow;
 bool vBlankLine = false;
 
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
   vSourceFile = argv[cntArg + 1];
   continue;
   }
  
  if (vParam == "o")
  { 
   vDestFile = argv[cntArg + 1];
   continue;
   }

  if (vParam == "c")
  { 
   vColumn = true;	
   string tmpCols = argv[cntArg + 1];
   vector<string> tmpcCols;
   tokenize(tmpCols, tmpcCols, " ,");
   for (unsigned short i = 0; i < tmpcCols.size(); i++)
    vcCols.push_back(strToInt(tmpcCols[i]));
   continue;
   }

  if (vParam == "s")
  { 
   vColSpace = strToInt(argv[cntArg + 1]);
   continue;
   }

  if (vParam == "r")
  { 
   vColumn = false;	
   vRow = argv[cntArg + 1][0];
   continue;
   }

  if (vParam == "b")
  { 
   vBlankLine = true;	
   continue;
   }
  }

 if (vSourceFile.empty())
 {
  cerr << "\n\nPlease specify an input file!" << endl << endl;
  exit(-1);
  }

 if (vColumn)
 {
  if (vDestFile.empty())
   vDestFile = vSourceFile + ".out";

  cout << "\nExtracting annotations columns ..." << endl;
  if (ExtractCols(vSourceFile, vcCols, vColSpace, vDestFile) == 0)
   cout << "\nExtracting annotations columns is done!" << endl;
  else 
  {
   cerr << "\n\nExtracting annotations column was not sucessful!" << endl << endl;
   exit(-1);
   }
  }
 else
 {
  if (vDestFile.empty())
   vDestFile = vSourceFile + "." + vRow;

  cout << "\nExtracting annotation rows..." << endl;
  if (ExtractRows(vSourceFile, vRow, vBlankLine, vDestFile) == 0)
   cout << "\nExtracting annotation rows is done!" << endl;
  else 
  {
   cerr << "\n\nExtracting annotation rows was not sucessful!" << endl << endl;
   exit(-1);
   }
  }
  
 cout << endl;
}

/**
 * 
 *  Function definitions
 * 
 */ 

short ExtractCols(const string &pSourceFile,
                  vector<unsigned short> &pCols,
                  unsigned short pColSpace,
                  const string &pDestFile)
{
 ifstream strmSrc;
 if (!pSourceFile.empty())
 {
  strmSrc.open(pSourceFile.c_str());
  if(!strmSrc)
  {
   cerr << "Can't open file: " << pSourceFile << endl;
   return -1;
   }
  }

 ofstream strmDest (pDestFile.c_str());
 if(!strmDest)
 {
  cerr << "\nCan't create output file: " << pDestFile << endl;
  return -1;
  }
 
 string vSrcLine;
 vector<string> vcTokens;
 size_t cntSntncs = 0;
 
 // looping to read lines
 while (!strmSrc.eof())
 {
  getline(strmSrc, vSrcLine);

  vcTokens.clear();
  tokenize(vSrcLine, vcTokens, " \t\n\r");
  
  // if this is a sentence separator line, put an empty line in the output
  // file and skip next steps
  if (vcTokens.empty())
  {
   strmDest << endl;
   cntSntncs++;
   cout << "\r" << cntSntncs << " sentences processed" << flush;
   continue;
   }

  // writing the 1st column left-aligned and the rest right-align
  strmDest.width(0);
  for (unsigned short i = 0; i < pCols.size(); i++)
  {
   strmDest << vcTokens[pCols[i] - 1] << '\t';
   strmDest.width(pColSpace);
   }

  
  strmDest << endl;
  }
 
 cout << "\r" << cntSntncs - 1 << " sentences processed" << flush;
 return 0;
}

short ExtractRows(const string &pSourceFile,
                  char pRow,
                  bool pBlankLine,
                  const string &pDestFile)
{
 ifstream strmSrc;
 if (!pSourceFile.empty())
 {
  strmSrc.open(pSourceFile.c_str());
  if(!strmSrc)
  {
   cerr << "Can't open file: " << pSourceFile << endl;
   return -1;
   }
  }

 ofstream strmDest (pDestFile.c_str());
 if(!strmDest)
 {
  cerr << "\nCan't create output file: " << pDestFile << endl;
  return -1;
  }
 
 string vSrcLine;
 size_t cntSntncs = 0;
 
 // looping to read lines
 while (!strmSrc.eof())
 {
  getline(strmSrc, vSrcLine);

  // if this is a sentence separator line, put an empty line in the output
  if (vSrcLine.empty())
  {
   if (pBlankLine)
    strmDest << endl;
   cntSntncs++;
   cout << "\r" << cntSntncs << " sentences processed" << flush;
   }
  else
   if (vSrcLine[0] == pRow) 
    strmDest << vSrcLine.substr(4)
             << endl;
  
  }
 
 cout << "\r" << cntSntncs - 1 << " sentences processed" << flush;
 return 0;
}
