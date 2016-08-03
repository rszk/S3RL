/**
 *  @ RSK
 *  @ Version 0.1        18.02.2010
 * 
 *  excutable file converting CoNLL 2005 merged file to MaltParser's input
 *  which is CoNLL x (and CoNLL 2008) format
 * 
 */

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Constants.hpp"
#include "Utils.hpp"

using namespace std;


short Convert(const string &, const string &);

static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][...]" << endl
      << " <-i[CoNLL 2005 merged input file]>" << endl
      << " <-o[CoNLL x output file (MaltParser input)]>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 string vSourceFile;
 string vDestFile;
 
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
  }

 if (vSourceFile.empty())
 {
  cerr << "\n\nPlease specify an input file!" << endl << endl;
  exit(-1);
  }

 if (vDestFile.empty())
  vDestFile = vSourceFile + ".malt";

 cout << "\nConverting ..." << endl;
 if (Convert(vSourceFile, vDestFile) == 0)
  cout << "\nConversion is done!" << endl;
 else 
 {
  cerr << "\n\nConversion was not sucessful!" << endl << endl;
  exit(-1);
  }

 cout << endl;
}

/**
 * 
 *  Function definitions
 * 
 */ 

short Convert(const string &pSourceFile,
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
 size_t cntWords = 0;
 
 // looping to read lines
 while (!strmSrc.eof())
 {
  getline(strmSrc, vSrcLine);

  vcTokens.clear();
  tokenize(vSrcLine, vcTokens, " \t\n\r");
  
  // if this is a sentence separator line, reset sentence word count and
  // put an empty line in the output file and skip next steps
  if (vcTokens.empty())
  {
   cntWords = 0;
   strmDest << endl;
   cntSntncs++;
   cout << "\r" << cntSntncs << " sentences converted" << flush;
   continue;
   }

  cntWords++;
  
  strmDest << cntWords << '\t'
           << vcTokens[0] << '\t'
           << "_\t"
           << vcTokens[1] << '\t'
           << vcTokens[1] << '\t'
           << "_"
           << endl;
 }
 
 return 0;
}
