/**
 *  @ RSK
 *  @ Version 1.0        05.05.2010
 * 
 *  exceutable file for selecting sentences from CoNLL annotation files
 *  based on user-requested criteria and saving in a separate file
 * 
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#include "Constants.hpp"
#include "Utils.hpp"

using namespace std;


size_t LoadSentences(vector<string> &, 
                     vector<vector<vector<string> > > &, 
                     unsigned int, unsigned int);
short SaveSentences(vector<vector<vector<string> > > &,
                    vector<unsigned int> &,
                    vector<string> &,
                    const string &);
void fillWithNumbers(vector<unsigned int> &, const int, const int);

static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][...]" << endl
      << " <-n[Number of sentences]>" << endl 
      << " <-o[Selection order (1: original; 2: reverse; 3: random]>" << endl 
      << " <-shf[Number of time to shuffle the loaded sentences to randomize the selction (when -o is 3)]>" << endl
      << " <-f[From sentence]>" << endl
      << " <-t[To sentence]>" << endl
      << " <-min[Minimum sentence length]>" << endl
      << " <-max[Maximum sentence length]>" << endl
      << " <-fl[List of CoNLL annotation files separated by comma to extract the sentence annotations]>" << endl
      << " <-ext[Destination file extension>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 unsigned int vSntncNum = 0;         
 unsigned short vShflCount = 1;
 unsigned int vFromSntnc = 1;        // default is 1st sentence
 unsigned int vToSntnc = 0;
 unsigned int vMinLength = 0;        // default is 0 for no check
 unsigned int vMaxLength = 0;        // default is 0 for no check
 unsigned short vSelOrder = 1;       // default is original order
 
 string vDefaultPath = DEFAULT_CONLL_LABELED_PATH;
 vector<string> vcSrcFiles;
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
   
  if (vParam == "shf")
  { 
   vShflCount = atoi(argv[cntArg + 1]);
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

  if (vParam == "fl")
  { 
   string vSrcFiles = argv[cntArg + 1];
   tokenize(vSrcFiles, vcSrcFiles, ",;");
   continue;
   }
   
  if (vParam == "ext")
  { 
   vDestExtension = argv[cntArg + 1];
   continue;
   }
  }

 vector<vector<vector<string> > > vcAnnotations;
 
 for (unsigned short i = 0; i < vcSrcFiles.size(); i++)
  vcSrcFiles[i] = vDefaultPath + "/" + vcSrcFiles[i];
 
 cout << "\nLoading the source ..." << endl;
 size_t vLoadedSntnc  = LoadSentences(vcSrcFiles, vcAnnotations, vMinLength, vMaxLength);
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
                        vFromSntnc - 1, vToSntnc - 1, vShflCount);
  cout << "Generating random numbers is done!" << endl;
  }
 
///////////////////////////////////////////////////////
// for(unsigned int i=0; i<vNumbers.size(); i++)
//  cout << i << ": " << vNumbers[i] << " ";
 
 if (vSelOrder == 2)
  vDestExtension = "reverse." + vDestExtension;
 else if (vSelOrder == 3)
  vDestExtension = "random." + vDestExtension;

 if (vSntncNum != vLoadedSntnc)
  vDestExtension = intToStr(vSntncNum);
  
 cout << "\nSaving extracted sentences ..." << endl;
 if (SaveSentences(vcAnnotations, vNumbers, vcSrcFiles, vDestExtension) == 0)
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

size_t LoadSentences(vector<string> &pSrcFiles,
                     vector<vector<vector<string> > > &pAnnotations,
                     unsigned int pMinLength,
                     unsigned int pMaxLength)
{
 vector<ifstream *> strmSources;
 
 for (unsigned short i = 0; i < pSrcFiles.size(); i++)
 {
  ifstream *tmpStrm = new ifstream;
  tmpStrm->open(pSrcFiles[i].c_str());
  strmSources.push_back(tmpStrm);

  // one annotation vector per file
  pAnnotations.resize(pAnnotations.size() + 1);
  
  if(!strmSources[i])
  {
   cerr << "Can't open file: " << pSrcFiles[i] << endl;
   return 0;
   }
  }

 vector<string> vcSrcLines(strmSources.size(), "");

 size_t cntLine = 0;
 size_t cntSentence = 0;
 size_t cntShortSntncs = 0;
 size_t cntLongSntncs = 0;
 
 // looping to read sentences; only one of the streams is checked for eof
 while (!strmSources[0]->eof())
 {
  // reading the 1st line (in CoNLL format) of a sentence from files
  for (unsigned short i = 0; i < strmSources.size(); i++)
  {
   string tmpLine;
   getline(*strmSources[i], tmpLine);
   vcSrcLines[i] = tmpLine;
   }
  
  // If the 1st line of a sentence is empty, there is a blank line at the
  // begining of the sentence, which is not allowed, or it is the last line
  // which is left empty by CoNLL format and signals the end of file.
  // In both cases the procedure ends.
  if (isEmptyLine(vcSrcLines[0]))
   break;
  
  cntLine++;

  // prepare vectors to store lines of the sentence
  for (unsigned short i = 0; i < pAnnotations.size(); i++)
   pAnnotations[i].resize(pAnnotations[i].size() + 1);

  // looping to read lines
  while (!isEmptyLine(vcSrcLines[0]))
  { 
   for (unsigned short i = 0; i < pAnnotations.size(); i++)
    pAnnotations[i][pAnnotations[i].size() - 1].push_back(vcSrcLines[i]);

   // reading the next lines (in CoNLL format) of a sentence from files
   for (unsigned short i = 0; i < strmSources.size(); i++)
   {
    string tmpLine;
    getline(*strmSources[i], tmpLine);
    vcSrcLines[i] = tmpLine;
    }
  

   cntLine++;
   }

  // controlling CoNLL files consistency; base is the 1st file
  
  if (isEmptyLine(vcSrcLines[0]))
   for (unsigned short i = 1; i < strmSources.size(); i++)
    if ((!isEmptyLine(vcSrcLines[i])))
    {
     cerr << "There is a mismatch between first and " << i << "th file"
          << cntLine << endl;
     return 0;
     }
  
  // checking for sentence length criteria and deleting loaded stuff
  // if not meet
  
  if (pMinLength > 0)
  {
   if (pAnnotations[0][pAnnotations[0].size() - 1].size() < pMinLength)
   {
    for (unsigned short i = 0; i < pAnnotations.size(); i++)
     pAnnotations[i].pop_back();
    cntShortSntncs++;
    cntSentence++;
    cout << "\r" << cntSentence << " sentences loaded" << flush;
    continue;
    }
   } 
  
  if (pMaxLength > 0)
  {
   if (pAnnotations[0][pAnnotations[0].size() - 1].size() > pMaxLength)
   {
    for (unsigned short i = 0; i < pAnnotations.size(); i++)
     pAnnotations[i].pop_back();
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


short SaveSentences(vector<vector<vector<string> > > &pAnnotations,
                    vector<unsigned int> &pNumbers,
                    vector<string> &pSrcFiles,
                    const string &pDestExt)
{
 vector<ofstream *> strmDests;
 
 for (unsigned short i = 0; i < pSrcFiles.size(); i++)
 {
  ofstream *tmpStrm = new ofstream;
  string tmpDestFileName = pSrcFiles[i] + "." + pDestExt;
  tmpStrm->open(tmpDestFileName.c_str());
  strmDests.push_back(tmpStrm);
  
  if(!strmDests[i])
  {
   cerr << "Can't create output file: " << tmpDestFileName << endl;
   return -1;
   }
  }

 for (unsigned int cntSentence = 0; cntSentence < pNumbers.size(); cntSentence++)
 {
  for (unsigned int cntLine = 0; cntLine < pAnnotations[0][pNumbers[cntSentence]].size(); cntLine++)
   for (unsigned short i = 0; i < strmDests.size(); i++)
    *strmDests[i] << pAnnotations[i][pNumbers[cntSentence]][cntLine] << endl;
  
  // an empty line after each sentence
  for (unsigned short i = 0; i < pAnnotations.size(); i++)
   *strmDests[i] << endl;
  
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
