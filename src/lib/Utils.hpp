/**
 *  @ RSK
 *  @ Version 0.1        17.11.2009
 * 
 *  Utility functions declarations
 *   
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>

#include "CoNLLLoader.hpp"

using namespace std;


void tokenize(const string &, vector<string> &, const string &);
string extractLemma(const string &, const string &);
string guessVerbLemma(const string &, const string &);
bool isVerbPredicate(const string &, const string &);
bool isPassiveAux(string);

string trim(const string &);
string lowerCase(const string &);
bool isCapital(const char);
string intToStr(const int &);
string ulongToStr(const unsigned long int &);
string floatToStr(const double &);
long strToInt(const string &);
double strToFloat(const string &);
bool isNumeric(const string &);
string setThousandSeparator(const string &);
bool isEmptyLine(const string &);
void trimLine(string &);
short indexIn(const unsigned short, const unsigned short [], const unsigned);

bool isDir(const string &);
string extractFileName(const string &, bool);
string extractFileExt(const string &, unsigned short);

void mout(const string, ostream &, ostream &);

string getLocalTimeStr();
string getLocalTimeStr(string);
string convertToHMS(double);

void generateRandomNumbers(vector<unsigned int> &, const unsigned int &,
                           const unsigned int, const unsigned int,
                           const unsigned short);
double uniformDeviate(const int &);

bool isShorter(Sentence*, Sentence*);
bool isLonger(Sentence*, Sentence*);
bool isSimpler(Sentence*, Sentence*);
bool isMediate(Sentence*, Sentence*);
bool isMoreLikelyLabeling(Sentence*, Sentence*);
bool isMoreAgreedLabeling(Sentence*, Sentence*);
bool isMoreLikelyLabel(Sample*, Sample*);
bool isAgreedLabel(Sample*, Sample*);

int getSpansRelation(const WordSpan &, const WordSpan &);

string getFeatureName(const short &);

unsigned long int getMemUsage();

#endif /* UTILS_HPP */
