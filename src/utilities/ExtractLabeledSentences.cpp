/**
 *  @ RSK
 *  @ Version 0.2        29.12.2009
 * 
 *  excutable file for selecting labeled sentences from SPLIT CoNLL file
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


size_t LoadSentences(const string &, const string &, const string &,
                     vector<vector<string> > &, vector<vector<string> > &,
                     vector<vector<string> > &, unsigned int, unsigned int);
short SaveSentences(vector<vector<string> > &,
                    vector<vector<string> > &,
                    vector<vector<string> > &,
                    vector<unsigned int> &,
                    const string &, const string &, const string &);
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
      << " <-wf[CoNLL words source file name]>" << endl
      << " <-sf[CoNLL syntax source file name]>" << endl
      << " <-df[CoNLL converter dependency syntax source file name]>" << endl
      << " <-mf[CoNLL Malt dependency syntax source file name]>" << endl
      << " <-pf[CoNLL props source file name]>" << endl
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
 
 string vDefaultPath = DEFAULT_CONLL_LABELED_PATH;
 string vSrcWordsFile = vDefaultPath + "/" + DEFAULT_CONLL_TRAIN_WORDS_NAME;
 string vSrcSyntFile = vDefaultPath + "/" + DEFAULT_CONLL_TRAIN_SYNT_CHA_NAME;
 string vSrcSyntDepFile = vDefaultPath + "/" + ;
 string vSrcSyntMaltFile = vDefaultPath + "/" + ;
 string vSrcPropsFile = vDefaultPath + "/" + DEFAULT_CONLL_TRAIN_PROPS_NAME;
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

  if (vParam == "wf")
  { 
   vSrcWordsFile = vDefaultPath + "/" + argv[cntArg + 1];
   continue;
   }
   
  if (vParam == "sf")
  { 
   vSrcSyntFile = vDefaultPath + "/" + argv[cntArg + 1];
   continue;
   }
   
  if (vParam == "pf")
  { 
   vSrcPropsFile = vDefaultPath + "/" + argv[cntArg + 1];
   continue;
   }
  
  if (vParam == "ext")
  { 
   vDestExtension = argv[cntArg + 1];
   continue;
   }
  }

 vector<vector<string> > vcWords, vcSynt, vcProps;
 
 cout << "\nLoading the source ..." << endl;
 size_t vLoadedSntnc  = LoadSentences(vSrcWordsFile, vSrcSyntFile, vSrcPropsFile,
                                      vcWords, vcSynt, vcProps,
                                      vMinLength, vMaxLength);
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
 if (SaveSentences(vcWords, vcSynt, vcProps, vNumbers,
                   vSrcWordsFile + '.' + vDestExtension, 
                   vSrcSyntFile + '.' + vDestExtension,
                   vSrcPropsFile + '.' + vDestExtension) == 0)
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

size_t LoadSentences(const string &pWordsFile,
                     const string &pSyntFile,
                     const string &pPropsFile,
                     vector<vector<string> > &pWords,
                     vector<vector<string> > &pSynt,
                     vector<vector<string> > &pProps,
                     unsigned int pMinLength,
                     unsigned int pMaxLength)
{
 ifstream strmWords (pWordsFile.c_str());
 if(!strmWords)
 {
  cerr << "Can't open file: " << pWordsFile << endl;
  return 0;
  }

 ifstream strmSynt (pSyntFile.c_str());
 if(!strmSynt)
 {
  cerr << "Can't open file: " << pSyntFile << endl;
  return 0;
  }

 ifstream strmProps (pPropsFile.c_str());
 if(!strmProps)
 {
  cerr << "Can't open file: " << pPropsFile << endl;
  return 0;
  }

 string vWordsLine;
 string vSyntLine;
 string vPropsLine;
 size_t cntLine = 0;
 size_t cntSentence = 0;
 size_t cntShortSntncs = 0;
 size_t cntLongSntncs = 0;
 
 // looping to read sentences
 while (!(strmWords.eof() || strmSynt.eof()))
 {
  // reading the 1st line (in CoNLL format) of a sentence from files
  getline(strmWords, vWordsLine);
  getline(strmSynt, vSyntLine);
  getline(strmProps, vPropsLine);
  
  // If the 1st line of a sentence is empty, there is a blank line at the
  // begining of the sentence, which is not allowed, or it is the last line
  // which is left empty by CoNLL format and signals the end of file.
  // In both cases the procedure ends.
  if (isEmptyLine(vWordsLine))
   break;
  
  cntLine++;

  // prepare vectors to store lines of the sentence
  pWords.resize(pWords.size() + 1);
  pSynt.resize(pSynt.size() + 1);
  pProps.resize(pProps.size() + 1);

  // looping to read lines
  while (!isEmptyLine(vWordsLine))
  { 
   pWords[pWords.size() - 1].push_back(vWordsLine);
   pSynt[pSynt.size() - 1].push_back(vSyntLine);
   pProps[pProps.size() - 1].push_back(vPropsLine);

   // reading the next lines (in CoNLL format) of a sentence from files
   getline(strmWords, vWordsLine);
   getline(strmSynt, vSyntLine);
   getline(strmProps, vPropsLine);
  
   cntLine++;
   }
  
  // controlling CoNLL files consistency
  if ((isEmptyLine(vWordsLine)) && (!isEmptyLine(vSyntLine)))
  {
   cerr << "There is a mismatch between word file and syntax file in line!"
        << cntLine << endl;
   return 0;
   }
  
  if ((isEmptyLine(vWordsLine)) && (!isEmptyLine(vPropsLine)))
  {
   cerr << "There is a mismatch between word file and props file in line!"
        << cntLine << endl;
   return 0;
   }

  // checking for sentence length criteria and deleting loaded stuff
  // if not meet
  
  if (pMinLength > 0)
  {
   if (pWords[pWords.size() - 1].size() < pMinLength)
   {
    pWords.pop_back();
    pSynt.pop_back();
    pProps.pop_back();
    cntShortSntncs++;
    cntSentence++;
    cout << "\r" << cntSentence << " sentences loaded" << flush;
    continue;
    }
   } 
  
  if (pMaxLength > 0)
  {
   if (pWords[pWords.size() - 1].size() > pMaxLength)
   {
    pWords.pop_back();
    pSynt.pop_back();
    pProps.pop_back();
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


short SaveSentences(vector<vector<string> > &pWords,
                    vector<vector<string> > &pSynt,
                    vector<vector<string> > &pProps,
                    vector<unsigned int> &pNumbers,
                    const string &pWordsFile,
                    const string &pSyntFile,
                    const string &pPropsFile)
{
 ofstream strmWords (pWordsFile.c_str());
 if(!strmWords)
 {
  cerr << "\nCan't create output file: " << pWordsFile << endl;
  return -1;
  }
 
 ofstream strmSynt (pSyntFile.c_str());
 if(!strmSynt)
 {
  cerr << "\nCan't create output file: " << pSyntFile << endl;
  return -1;
  }
 
 ofstream strmProps (pPropsFile.c_str());
 if(!strmProps)
 {
  cerr << "\nCan't create output file: " << pPropsFile << endl;
  return -1;
  }
  
 for (unsigned int cntSentence = 0; cntSentence < pNumbers.size(); cntSentence++)
 {
  for (unsigned int cntLine = 0; cntLine < pWords[pNumbers[cntSentence]].size(); cntLine++)
  {
   strmWords << pWords[pNumbers[cntSentence]][cntLine] << endl;  
   strmSynt << pSynt[pNumbers[cntSentence]][cntLine] << endl;  
   strmProps << pProps[pNumbers[cntSentence]][cntLine] << endl;  
   }
  
  // an empty line after each sentence
  strmWords << endl;  
  strmSynt << endl;  
  strmProps << endl;  
  
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
