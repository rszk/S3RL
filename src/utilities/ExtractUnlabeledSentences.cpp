/**
 *  @ RSK
 *  @ Version 0.1        08.02.2010
 * 
 *  excutable file for selecting unlabeled sentences from MERGED CoNLL file
 *  format based on user-requested criteria and saving in a separate file
 * 
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#include "Constants.hpp"
#include "Utils.hpp"

using namespace std;


size_t LoadSentences(const string &, vector<vector<string> > &, unsigned int, unsigned int);
short SaveSentences(vector<vector<string> > &, vector<unsigned int> &, const string &);
void fillWithNumbers(vector<unsigned int> &, const int, const int);

static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][...]" << endl
      << " <-n[Number of sentences]>" << endl 
      << " <-o[Selection order (1: original; 2: reverse; 3: random]>" << endl 
      << " <-f[From sentence]>" << endl
      << " <-t[To sentence]>" << endl
      << " <-min[Minimum sentence length]>" << endl
      << " <-max[Maximum sentence length]>" << endl
      << " <-sf[CoNLL source file name]>" << endl
      << " <-ext[Destination file extension>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 unsigned int vSntncNum = 0;         
 unsigned int vFromSntnc = 1;        // default is 1st sentence
 unsigned int vToSntnc = 0;
 unsigned int vMinLength = 0;        // default is 0 for no check
 unsigned int vMaxLength = 0;        // default is 0 for no check
 unsigned short vSelOrder = 1;       // default is original order
 
 string vDefaultPath = DEFAULT_CONLL_UNLABELED_PATH;
 string vSrcFile = vDefaultPath + "/" + DEFAULT_CONLL_UNLABELED_DATA_NAME;
 string vDestExtension;
 
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
  
  if (vParam == "n")
  { 
   vSntncNum = atoi(argv[cntArg + 1]);
   continue;
   }
   
  if (vParam == "o")
  { 
   vSelOrder = atoi(argv[cntArg + 1]);
   continue;
   }
   
  if (vParam == "f")
  { 
   vFromSntnc = atoi(argv[cntArg + 1]);
   continue;
   }
   
  if (vParam == "t")
  { 
   vToSntnc = atoi(argv[cntArg + 1]);
   continue;
   }

  if (vParam == "min")
  { 
   vMinLength = atoi(argv[cntArg + 1]);
   continue;
   }
   
  if (vParam == "max")
  { 
   vMaxLength = atoi(argv[cntArg + 1]);
   continue;
   }

  if (vParam == "ext")
  { 
   vDestExtension = argv[cntArg + 1];
   continue;
   }

  if (vParam == "sf")
  { 
   vSrcFile = vDefaultPath + "/" + argv[cntArg + 1];
   continue;
   }
  }

 vector<vector<string> > vcData;
 
 cout << "\nLoading the source ..." << endl;
 size_t vLoadedSntnc = LoadSentences(vSrcFile, vcData, vMinLength, vMaxLength);
 if (vLoadedSntnc > 0)
  cout << "\nLoading the source is done!" << endl;
 else 
 {
  cerr << "\n\nLoading the source was not sucessful!" << endl << endl;
  exit(-1);
  }

 // if the last sentence number to be extracted is not specified, the last 
 // of all sentences will be considered
 if (vToSntnc == 0)
  vToSntnc = vLoadedSntnc;

 if (vSntncNum == 0)
  vSntncNum = vToSntnc - vFromSntnc + 1;

 if ((vToSntnc - vFromSntnc + 1) > vLoadedSntnc)
  vToSntnc = vFromSntnc + vLoadedSntnc - 1;
 
 vector<unsigned int> vNumbers;
 if (vSelOrder == 1)
  fillWithNumbers(vNumbers, vFromSntnc - 1, vFromSntnc - 2 + vSntncNum);
 else if (vSelOrder == 2)
  fillWithNumbers(vNumbers, vFromSntnc - 2 + vSntncNum, vFromSntnc - 1);
 else if (vSelOrder == 3)
 {
  cout << "\nGenerating random numbers ..." << endl;
  generateRandomNumbers(vNumbers, vSntncNum,
                        vFromSntnc - 1, vToSntnc - 1);
  cout << "Generating random numbers is done!" << endl;
  }
 
///////////////////////////////////////////////////////
// for(unsigned int i=0; i<vNumbers.size(); i++)
//  cout << i << ": " << vNumbers[i] << " ";
 
 if (vSelOrder == 2)
  vDestExtension += "reverse.";
 else if (vSelOrder == 3)
  vDestExtension += "random.";

 if (vSntncNum != vLoadedSntnc)
  vDestExtension = intToStr(vSntncNum);
  
 cout << "\nSaving extracted sentences ..." << endl;
 if (SaveSentences(vcData, vNumbers, vSrcFile + '.' + vDestExtension) == 0)
  cout << "\nSaving extracted sentences is done!" << endl;
 else 
 {
  cerr << "\n\nSaving extracted sentences was not sucessful!" << endl << endl;
  exit(-1);
  }

 cout << endl;
}

/**
 * 
 *  Function definitions
 * 
 */ 

size_t LoadSentences(const string &pSrcFile,
                     vector<vector<string> > &pData,
                     unsigned int pMinLength,
                     unsigned int pMaxLength)
{
 ifstream strmData (pSrcFile.c_str());
 if(!strmData)
 {
  cerr << "Can't open file: " << pSrcFile << endl;
  return 0;
  }

 string vDataLine;
 size_t cntLine = 0;
 size_t cntSentence = 0;
 size_t cntShortSntncs = 0;
 size_t cntLongSntncs = 0;
 
 // looping to read sentences
 while (!strmData.eof())
 {
  // reading the 1st line (in CoNLL format) of a sentence from file
  getline(strmData, vDataLine);
  
  // If the 1st line of a sentence is empty, there is a blank line at the
  // begining of the sentence, which is not allowed, or it is the last line
  // which is left empty by CoNLL format and signals the end of file.
  // In both cases the procedure ends.
  if (isEmptyLine(vDataLine))
   break;
  
  cntLine++;

  // prepare vector to store lines of the sentence
  pData.resize(pData.size() + 1);

  // looping to read lines
  while (!isEmptyLine(vDataLine))
  { 
   pData[pData.size() - 1].push_back(vDataLine);

   // reading the next lines (in CoNLL format) of a sentence from file
   getline(strmData, vDataLine);
  
   cntLine++;
   }
  
  // checking for sentence length criteria and deleting loaded stuff
  // if not meet
  
  if (pMinLength > 0)
  {
   if (pData[pData.size() - 1].size() < pMinLength)
   {
    pData.pop_back();
    cntShortSntncs++;
    cntSentence++;
    cout << "\r" << cntSentence << " sentences loaded" << flush;
    continue;
    }
   } 
  
  if (pMaxLength > 0)
  {
   if (pData[pData.size() - 1].size() > pMaxLength)
   {
    pData.pop_back();
    cntLongSntncs++;
    cntSentence++;
    cout << "\r" << cntSentence << " sentences loaded" << flush;
    continue;
    }
   }	
  
  cntSentence++;
  cout << "\r" << cntSentence << " sentences loaded" << flush;
  }
 
 cout << endl << cntShortSntncs << " were shorter than " << pMinLength
      << endl << cntLongSntncs << " were longer than " << pMaxLength;

 return cntSentence - cntShortSntncs - cntLongSntncs;
}


short SaveSentences(vector<vector<string> > &pData,
                    vector<unsigned int> &pNumbers,
                    const string &pDestFile)
{
 ofstream strmDest (pDestFile.c_str());
 if(!strmDest)
 {
  cerr << "\nCan't create output file: " << pDestFile << endl;
  return -1;
  }
 
 for (unsigned int cntSentence = 0; cntSentence < pNumbers.size(); cntSentence++)
 {
  for (unsigned int cntLine = 0; cntLine < pData[pNumbers[cntSentence]].size(); cntLine++)
  {
   strmDest << pData[pNumbers[cntSentence]][cntLine] << endl;  
   }
  
  // an empty line after each sentence
  strmDest << endl;  
  
  cout << "\r" << cntSentence + 1 << " sentences (No. " 
       << pNumbers[cntSentence] << ") saved" << flush;
  }
 
 cout << "\r" << pNumbers.size() << " sentences saved" 
      << "               " << flush;

 return 0;
}


void fillWithNumbers(vector<unsigned int> &pNumbers,
                     const int pStart,
                     const int pEnd)
{
 int cntNumbers;
 
 if (pStart <= pEnd)
  for (cntNumbers = pStart; cntNumbers <= pEnd; cntNumbers++)
   pNumbers.push_back(cntNumbers);
 else
  for (cntNumbers = pStart; cntNumbers >= pEnd; cntNumbers--)
   pNumbers.push_back(cntNumbers);
}
