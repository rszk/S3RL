/**
 *  @ RSK
 *  @ Version 1.1        03.03.2010
 * 
 *  Classes to load CoNLL data (sentences + annotations) from CoNLL files
 *  into Sentence class
 * 
 *  Both 2005 and 2008 data are loaded here, 2005 being the pivot for
 *  propositions and arguments, since dependecy-based arguments will be
 *  derived from constituent-based arguments (as done by 2008 shared task
 *  itself) to keep the consistency and ease of unification in co-training
 * 
 */

#ifndef CONLLLOADER_HPP
#define CONLLLOADER_HPP

#include <vector>
#include <string>
#include "Sentence.hpp"

using namespace std;


class CoNLLLoader
{
 private:
  
  string words;
  string syntCha;
  string syntDep;
  string props;
  int maxSentences;

  int loadedSentencesCount;
  int loadedWordsCount;
  int loadedCnstntCount;
  int loadedPropsCount;
  int loadedArgsCount;

  // to keep the last argument read for each proposition, so that a 
  // predicate anntotation (V*) can be distinguished from actual argument
  // annotation when it comes to settting the span end of the last argument
  // (i.e. facing the closing bracket)
  vector<string> lastArg;
  
  // to keep number of predicate lemmas read so far for the sentence 
  // (the first column of CoNLL props file) to identify the proposition
  // for which the newly read lemma belongs (the best way of setting lemma
  // which is also compatible with the design of all CoNLL's data)
  short predRead;
  
  // to gradually stack tree nodes read from CoNLL data, which is mainly 
  // needed for remembering parents of new nodes
  vector<TBTree *> tmpTBStack; 
  
  // temporarily keeping the dependency head word index of the sentence's
  // words (1st column of dependency syntax file), since those are set after
  // reading whole the sentence
  vector<WordIdx> tmpHeadIdxs;
  
  double cpuUsage;
  double elapsedTime;
  
  DataAnalysis *dataAnalysis;

  void appendToTBTree(Sentence &, const string &, 
                      const string &, const WordIdx, const bool &);
  void appendToDTree(Sentence &, WordPtr &, const string &);
  void appendToProps(Sentence &, const vector<string> &, const WordIdx &);

 public:

  CoNLLLoader(string &, string &, string &, string &, int);
  ~CoNLLLoader();
  
  double getElapsedTime();

  int loadData(vector<Sentence *> &);  
  void logLoadedData();
  void logDataAnalysis();
};

#endif /* CONLLLOADER_HPP */
