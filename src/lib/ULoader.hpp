/**
 *  @ RSK
 *  @ Version 1.1        03.03.2010
 * 
 *  Classes to load unlabeled parsed data in CoNLL format into Sentence 
 *  class. Data includes word forms and syntactic parses and POS tags.
 *  Predicates (verbs) are identified on the fly based on POS tags.
 *  
 * 
 */

#ifndef ULOADER_HPP
#define ULOADER_HPP

#include <vector>
#include <string>
#include "Sentence.hpp"

using namespace std;


class ULoader
{
 private:
  
  string dataFile;
  string syntDepFile;
  int maxSentences;

  int loadedSentencesCount;
  int loadedWordsCount;
  int loadedCnstntCount;
  int loadedPredsCount;

  // to gradually stack tree nodes read from CoNLL data, which is mainly 
  // needed for remembering parents of new nodes
  vector<TBTree *> tmpTBStack; 
  
  // temporarily keeping the dependency head word index of the sentence's
  // words (1st column of dependency syntax file), since those are set after
  // reading whole the sentence
  vector<WordIdx> tmpHeadIdxs;
  
  double cpuUsage;
  double elapsedTime;
  
  void appendToTBTree(Sentence &, const string &, 
                      const string &, const WordIdx, const bool &);
  void appendToDTree(Sentence &, WordPtr &, const string &);
  void appendToProps(Sentence &, const string &, const WordIdx &);

 public:

  ULoader(string &, string &, int);
  
  double getElapsedTime();

  int loadData(vector<Sentence *> &);  
  void logLoadedData();
};

#endif /* ULOADER_HPP */
