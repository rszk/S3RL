/**
 *  @ RSK
 *  @ Version 0.1        17.11.2009
 * 
 *  Utility functions definitions for S3RL
 * 
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <dirent.h>
#include <algorithm>

#include "wn.h"

#include "Utils.hpp"
#include "Constants.hpp"
#include "Sample.hpp"

// This is declared (extern) in Constants.hpp. For commnets see there.
unsigned short gvActiveFeatureView = 1;


/**
 *  @Adapted from Swirl -> CharUtils -> simpleTokenize()
 * 
 *  Gets a string and tokenize it into a vector of strings (2nd arg)
 *  based on specified separators (3rd arg)
 */
void tokenize(const string &pInput,
		      vector<string> &pvcTokens,
		      const string &pSeparators)
{
 for(int start = pInput.find_first_not_of(pSeparators);
     start < (int) pInput.size() && start >= 0;
     start = pInput.find_first_not_of(pSeparators, start))
 {
  int end = pInput.find_first_of(pSeparators, start);
  if(end < 0) end = pInput.size();
  pvcTokens.push_back(pInput.substr(start, end - start));
  start = end;
  }
};

/**
 *  using WordNet 3 lemmatizer to extract the lemma of a word
 *  (see http://wordnet.princeton.edu/wordnet/man/morph.3WN.html)
 * 
 *  It was observed that for some verbs (e.g. "readied") WordNet lemmatizer
 *  returns NULL. To treat such cases, a non-complete heuristic is followed
 *  by calling guessVerbLemma() (only verbs).
 */
string extractLemma(const string &pWord, const string &pPOS)
{
 char *vWord, * vLemma;
 vWord = const_cast<char *> (pWord.c_str());
 
 // conveting to WordNet POS tags; if no matching POS, returns the word itself
 string vPOSPrefix = pPOS.substr(0, 2);
 int vWNPOS;
 if (vPOSPrefix == "NN")
  vWNPOS = 1;
 else if (vPOSPrefix == "VB")
  vWNPOS = 2;
 else if (vPOSPrefix == "JJ")
  vWNPOS = 3;
 else if (vPOSPrefix == "RB")
  vWNPOS = 4;
 else
  return pWord;
  
 if (wninit() == 0)
  vLemma = morphstr(vWord, vWNPOS);
 else 
  cerr << "\nError in initializing WordNet" << endl;
  
 if (!vLemma)
 {
  if (vWNPOS == 2)
   return guessVerbLemma(vWord, pPOS);
  else 
   return vWord;
  }
 
 return vLemma; 
}
 

/**
 *  a non-complete heuristic to guess the verb lemma to be used when
 *  WordNet fails (just trying the chance)
 */
string guessVerbLemma(const string &pVerb, const string &pPOS)
{
 unsigned short vLen = pVerb.size();

 // mostly to prevent some errors because of incorrect text or parsing
 if (vLen < 2)
  return pVerb;
 
 string vRawLemma = pVerb;
 
 if (pPOS == "VB")
  return pVerb; 
 
 if (pPOS == "VBZ")
 {
  // mostly to prevent some errors because of incorrect text or parsing
  if (vLen <= 2)
   return pVerb;
 
  if (pVerb.substr(vLen - 2, 2) == "es")
   vRawLemma == pVerb.substr(0, vLen - 3);
  else if (pVerb.substr(vLen - 1, 1) == "s")
   vRawLemma == pVerb.substr(0, vLen - 2);
  }
    
 if ((pPOS == "VBD") || (pPOS == "VBN"))
 {
  // mostly to prevent some errors because of incorrect text or parsing
  if (vLen <= 2)
   return pVerb;
 
  if (pVerb.substr(vLen - 2, 2) == "ed")
   vRawLemma == pVerb.substr(0, vLen - 3);
  else if (pVerb.substr(vLen - 1, 1) == "d")
   vRawLemma == pVerb.substr(0, vLen - 2);
  }

 if (pPOS == "VBG")
 {
  // mostly to prevent some errors because of incorrect text or parsing
  if (vLen <= 3)
   return pVerb;
 
  if (pVerb.substr(vLen - 3, 3) == "ing")
   vRawLemma == pVerb.substr(0, vLen - 4);
  }

 if (vRawLemma.substr(vLen - 1, 1) == "i") 
  return vRawLemma.substr(0, vLen - 2)+ 'y';
 
 return vRawLemma;
}	

/**
 *  returns true if the POS tag starts with "VB" and the verb is not a 
 *  passive auxiliary ("be" verbs) wrongly tagged with "VB"
 */
bool isVerbPredicate(const string &pWordForm, const string &pPOS)
{
 
 if (pPOS.substr(0, 2) != "VB")
  return false;
 
 // if the word is a passive auxiliary, it is not predicate
 for (int i = 0; !AUX_VERBS[i].empty(); i++)
  if (pWordForm == AUX_VERBS[i])
   return false;

 return true;
}

bool isPassiveAux(string pVerbForm)
{
 for (int i = 0; !AUX_VERBS[i].empty(); i++)
  if (pVerbForm == AUX_VERBS[i])
   return true;
 
 return false;
}

/**
 *  Examines an input string and returns true if it is a sequence of 
 *  spaces or empty at all
 */
bool isEmptyLine(const string &pInput)
{
 return ((pInput.empty()) || trim(pInput).empty());
};

/**
 *  Deletes \n\r from the end of line if any exists
 */
void trimLine(string &pLine)
{
 size_t tmpPosition = pLine.find("\n");
 if (tmpPosition != string::npos)
  pLine.erase(tmpPosition, 2);

 tmpPosition = pLine.find("\r");
 if (tmpPosition != string::npos)
  pLine.erase(tmpPosition, 2);
}

/**
 *  returns the index of first parameter in pArray or -1 if it does not
 *  exist (currently, only unsigned int arrays)
 */ 
short indexIn(const unsigned short pItem, const unsigned short pArray[],
              const unsigned pArrayLength)
{
 for (unsigned int cntArray = 0; 
      cntArray < pArrayLength;
      cntArray++)
 {
  if (pArray[cntArray] == pItem)
   return cntArray;
  }
 
 return -1; 
}

/**
 *  Determines whether the parameter is a valid directory or not.
 *  Invalid directory may means file or a non-exsisting or problematic
 *  directory.
 */
bool isDir(const string &pFileDir)
{
 DIR *vDir;

 vDir = opendir(pFileDir.c_str());
 
 if (vDir != NULL)
  return true;
 else   
  return false;  
}

/**
 *  Returns file name extracted from a full-path file name either with 
 *  or without extension based on 2nd parameter
 */
string extractFileName(const string &pFull, bool pWithExt)
{
 string vFullName = pFull.substr(pFull.find_last_of('/') + 1);
 if (pWithExt)
  return vFullName;
 else
  return vFullName.substr(0, vFullName.find_first_of('.'));
}

/**
 *  Returns file extension according to the level that is specified in 
 *  the last parameter; e.g. level of 2 for "example.ext1.ext2" returns
 *  "ext1.ext2" and for "example.ext1" returns itself.
 */
string extractFileExt(const string &pFull, unsigned short pLevel)
{
 size_t tmpPosition = pFull.size();
 unsigned short vLevel = pLevel;
 
 do
 {
  tmpPosition = pFull.find_last_of('.', tmpPosition - 1);
  vLevel--;
  } while ((vLevel > 0) && (tmpPosition != string::npos));
 
 if (tmpPosition != string::npos)
  return pFull.substr(tmpPosition + 1);
 else
  return pFull; 
}


/**
 *  outputs pStr to both pStrm1 and Strm2
 */
void mout(const string pStr, ostream &pStream1, ostream &pStream2)
{
 pStream1 << pStr << flush;
 pStream2 << pStr << flush;
}

/**
 *  Returns current local time string formatted by asctime()
 */
string getLocalTimeStr()
{
 time_t vRawTime;

 time(&vRawTime);
 string vTimeStr = asctime(localtime(&vRawTime));
 trimLine(vTimeStr);
 return vTimeStr;
}

/**
 *  Returns current local time string in the specified format
 * 
 *  Note: this is currently incomplete
 */
string getLocalTimeStr(string pFormat)
{
 if (pFormat == "asc")
  return getLocalTimeStr();

 time_t vRawTime;
 struct tm * vTimeInfo;

 time(&vRawTime);
 vTimeInfo = localtime(&vRawTime);
 string vTimeStr;

 if (pFormat == "YYYY-MM-DD-HH-MM") 
  vTimeStr = intToStr(vTimeInfo->tm_year + 1900) + "-" + 
             intToStr(vTimeInfo->tm_mon + 1) + "-" +
             intToStr(vTimeInfo->tm_mday) + "-" + 
             intToStr(vTimeInfo->tm_hour) + "-" +
             intToStr(vTimeInfo->tm_min);
 
 return vTimeStr;
}

string convertToHMS(double elapsedTime)
{
 int vHours, vMins, vSecs;
 long int vRndElapsedTime = elapsedTime;
 
 vHours = vRndElapsedTime / 3600;
 vMins = (vHours == 0 ? (vRndElapsedTime / 60) : (vRndElapsedTime % vHours) / 60);
 vSecs = (vMins == 0 ? (vRndElapsedTime) : (vRndElapsedTime % ((vHours * 3600) + (vMins * 60))));
 
 return intToStr(vHours) + "h, " + 
        intToStr(vMins) + "m, " + 
        intToStr(vSecs) + "s";
}

/**
 *  Trims space characters from left and right of the input string
 */
string trim(const string &pInput)
{
 string vResult;
 int cntPos = 0;
 int vStart, vEnd, vInputSize;
 
 if (pInput.empty())
  return "";
 
 // iterating spaces at the left
 vInputSize = pInput.size();
 while ((cntPos < vInputSize) && (pInput[cntPos] == ' '))
  cntPos++;
 vStart = cntPos;
 
 // iterating spaces at the right
 cntPos = vInputSize - 1;
 while ((cntPos >= 0) && (pInput[cntPos] == ' '))
  cntPos--;
 vEnd = cntPos; 
 
 if (vStart <= vEnd)
  vResult.assign(pInput, vStart, vEnd-vStart+1);
 else
  return ""; 
 return vResult; 
}

string lowerCase(const string &pStr)
{
 string vLowercase;
 
 for (size_t cntStr = 0; cntStr < pStr.size(); cntStr++)
  vLowercase += tolower(pStr[cntStr]);
  
 return vLowercase;
}

bool isCapital(const char pChar)
{
 return ((pChar > 'A') && (pChar < 'Z'));
}

string intToStr(const int &pInt)
{
 ostringstream sout;
 sout << pInt;
 return sout.str();
}

string ulongToStr(const unsigned long int &pULong)
{
 ostringstream sout;
 sout << pULong;
 return sout.str();
}

string floatToStr(const double &pFloat)
{
 ostringstream sout;
 sout << pFloat;
 return sout.str();
}

long strToInt(const string &pStr)
{
 long vInt;
 istringstream sin;
 sin.str(pStr);
 sin >> vInt;
 if (!sin.fail())
  return vInt;
 else
  return -1; 
}

double strToFloat(const string &pStr)
{
 double vFloat;
 istringstream sin;
 sin.str(pStr);
 sin >> vFloat;
 if (!sin.fail())
  return vFloat;
 else
  return -1; 
}

bool isNumeric(const string &pStr)
{
 double vInput;
 istringstream sin;
 sin.str(pStr);
 sin >> vInput;
 return !sin.fail();
}

string setThousandSeparator(const string &pStr)
{
 unsigned short vLen = pStr.size();
 string vOutput = pStr;
 
 for (unsigned short i = 1; i <= ((vLen-1) / 3); i++)
  vOutput.insert((vLen - (3 * i)), ",");

 return vOutput;
}



/**
 *  Generates pCount random number of type unsigned integer in the range
 *  of pStart to pEnd and puts into pNumbers
 *  
 *  The algorithm uses random_suffle() instead of rand(), because some bug
 *  or difficulty in using rand() were found that repeaded the randoms.
 *  This algorithm first generates sequential numbers from start to end,
 *  then randomly rearrange it using random_suffle. Then deletes all 
 *  number after element pCount in output vector.
 */
void generateRandomNumbers(vector<unsigned int> &pNumbers,
                           const unsigned int &pCount,
                           const unsigned int pStart,
                           const unsigned int pEnd,
                           const unsigned short pShflCnt)
{
 unsigned int cntNumbers;
 
 for (cntNumbers = pStart; cntNumbers <= pEnd; cntNumbers++)
  pNumbers.push_back(cntNumbers);
  
 for (unsigned short i = 1; i <= pShflCnt; i++)
  random_shuffle(pNumbers.begin(), pNumbers.end());
 
 for (cntNumbers = 1; cntNumbers <= (pEnd - pStart - pCount + 1); cntNumbers++)
  pNumbers.pop_back();
}

/**
 *  This is taken from http://eternallyconfuzzled.com/arts/jsw_art_rand.aspx
 *  as a more accurate way of generating random numbers
 */
double uniformDeviate(const int &pSeed)
{
 return pSeed * (1.0 / (RAND_MAX + 1.0));
}

bool isShorter(Sentence* p1st, Sentence* p2nd)
{
 return (p1st->getLength() < p2nd->getLength());
}
 
bool isLonger(Sentence* p1st, Sentence* p2nd)
{
 return (p1st->getLength() > p2nd->getLength());
}
 
bool isSimpler(Sentence* p1st, Sentence* p2nd)
{
 return (p1st->getSampleCount() < p2nd->getSampleCount());
}
 
bool isMediate(Sentence* p1st, Sentence* p2nd)
{
 if ((p1st->getLength() >= 15) && (p1st->getLength() <=30) &&
     (p2nd->getLength() >= 15) && (p2nd->getLength() <=30))
  return false;
 else if ((p1st->getLength() >= 15) && (p1st->getLength() <=30))
  return true;
 else if ((p2nd->getLength() >= 15) && (p2nd->getLength() <=30))
  return false;
 else
  return false;
}
 
bool isMoreLikelyLabeling(Sentence* p1st, Sentence* p2nd)
{
 return (p1st->getLabelingProbability(gvActiveFeatureView) > 
         p2nd->getLabelingProbability(gvActiveFeatureView));
}

bool isMoreAgreedLabeling(Sentence* p1st, Sentence* p2nd)
{
 return (p1st->getLabelingAgreement() > 
         p2nd->getLabelingAgreement());
}

bool isMoreLikelyLabel(Sample* p1st, Sample* p2nd)
{
 return (p1st->getPrdArgLabelProb(gvActiveFeatureView) > 
        p2nd->getPrdArgLabelProb(gvActiveFeatureView));
}

bool isAgreedLabel(Sample* p1st, Sample* p2nd)
{
 return (p1st->isPrdArgLabelsAgreed() > 
         p2nd->isPrdArgLabelsAgreed());
}


/**
 *  returns the relation of the first passed word span of elements (e.g. words)
 *  to another span in a sequence (e.g. sentence)
 *  
 *  (it is being completed based on needs)
 */
int getSpansRelation(const WordSpan &p1stSpan, const WordSpan &p2ndSpan)
{
 if (p1stSpan.end < p2ndSpan.start)
  return BEFORE;
  
 if (p2ndSpan.end < p1stSpan.start)
  return AFTER;
  
 if ((p1stSpan.start == p2ndSpan.start) && (p1stSpan.end == p2ndSpan.end))
  return ALIGNED;
  
 if (((p1stSpan.start >= p2ndSpan.start) && (p1stSpan.end < p2ndSpan.end)) ||
     ((p1stSpan.start > p2ndSpan.start) && (p1stSpan.end <= p2ndSpan.end)))
  return EMBEDDED;
  
 if (((p1stSpan.start <= p2ndSpan.start) && (p1stSpan.end > p2ndSpan.end)) ||
     ((p1stSpan.start < p2ndSpan.start) && (p1stSpan.end >= p2ndSpan.end)))
  return INCLUDE;

 if (((p1stSpan.start < p2ndSpan.start) && (p1stSpan.end < p2ndSpan.end)) ||
     ((p1stSpan.start > p2ndSpan.start) && (p1stSpan.end > p2ndSpan.end)))
  return OVERLAPED;
  
 return -1;
}

/** 
 *  converting the feature type numerical constant to string
 * 
 *  Note:
 *  This can convert the type to a string represenating the name or simply
 *  convert the feature no to string type. Currently, the latter is used,
 *  so there is no need to edit it when changing the feature set.
 */
string getFeatureName(const short &pFeatureType)
{
 return intToStr(pFeatureType + 1);

/*
 switch (pFeatureType)
 { 
  case PT_F:
   return "PT";
  case PL_F:
   return "PL";
  case PP_F:
   return "PP";
  case PVP_F:
   return "PVP";
  case PATH_F:
   return "PATH";
  case HW_F:
   return "HW";
  case GC_F:
   return "GC";
  case HWP_F:
   return "HWP";
  case PS_F:
   return "PS";
  case POSITION_F:
   return "Position";
  case PV_F:
   return "PV";
  case PF_F:
   return "PF";
  default:
   return ""; 
 }
*/  
}

/**
 *  @ Adapted from http://stackoverflow.com/questions/669438/
 *                      how-to-get-memory-usage-at-run-time-in-c
 *    by Don Wakefield
 * 
 *  attempts to read the system-dependent data for a process' virtual 
 *  memory size, and return the result in Bytes; on failure, returns 0
 */ 
unsigned long int getMemUsage()
{
 // 'file' stat seems to give the most reliable results
 ifstream strmStat("/proc/self/stat",ios_base::in);

 // dummy var for leading entries in stat that we don't care about
 string vDummy;

 // the field we want
 unsigned long vMemSize = 0;

 strmStat >> vDummy >> vDummy >> vDummy >> vDummy >> vDummy >> vDummy
          >> vDummy >> vDummy >> vDummy >> vDummy >> vDummy >> vDummy
          >> vDummy >> vDummy >> vDummy >> vDummy >> vDummy >> vDummy
          >> vDummy >> vDummy >> vDummy >> vDummy >> vMemSize; 
          // don't care about the rest

 return vMemSize;
}
