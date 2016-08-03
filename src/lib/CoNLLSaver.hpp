/**
 *  @ RSK
 *  @ Version 0.1        11.12.2009
 * 
 *  Classes to save annotated propositions into CoNLL props format
 * 
 */

#ifndef CONLLSAVER_HPP
#define CONLLSAVER_HPP

#include <vector>
#include <string>
#include "Sentence.hpp"

using namespace std;


class CoNLLSaver
{
 private:
  
  string propsFile;

  int savedSentencesCount;

  double cpuUsage;
  double elapsedTime;
  
 public:

  CoNLLSaver(string &);
  
  double getElapsedTime();

  int saveProps(vector<Sentence *> &, unsigned short);  
};

#endif /* CONLLSAVER_HPP */
