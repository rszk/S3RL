/**
 *  @ RSK
 *  @ Version 0.1        02.12.2009
 * 
 *  classes to generate labeled and unlabeled training samples and test 
 *  samples and analysis and statistics of the generation
 * 
 *  A LabeledSampleGenerator has a LabeledSGAnalysis to gather statistics
 *  and analysis of the generation
 * 
 *  A UnlabeledSampleGenerator has a UnlabeledSGAnalysis to gather statistics
 *  and analysis of the generation
 * 
 *  A TestSampleGenerator has a TestSGAnalysis to gather statistics
 *  and analysis of the generation
 * 
 *  We have not used inheritence for the above classes because they
 *  have a few things in common and also for the sake of time!
 *    
 */

#ifndef SAMPLEGENERATOR_HPP
#define SAMPLEGENERATOR_HPP

#include <vector>

#include "Types.hpp"

using namespace std;


class Sentence;
class Argument;
class Sample;
class LabeledSGAnalysis;
class UnlabeledSGAnalysis;
class TestSGAnalysis;

/**
 *   sample generator classes
 */

class LabeledSampleGenerator
{
 private:
 
  int maxPositive;
  int maxNegative;
  
  LabeledSGAnalysis *sgAnalysis;
  
  double cpuUsage;
  double elapsedTime;
  
 public:

  LabeledSampleGenerator();
  LabeledSampleGenerator(const int &, const int &);
  ~LabeledSampleGenerator();
  
  int generateSamples(vector<Sentence *> &, vector<Sample *> &, vector<Sample *> &, 
                      unsigned short, unsigned short [], bool);  
  
  void incPropCount();
  void incPositiveCount();
  void incNegativeCount();
  bool reachedMaxPositive();
  bool reachedMaxNegative();
  void addArgXPMissed(Argument * const &);
  
  double getElapsedTime();
  
  void logSGAnalysis(string &, bool);
};


class UnlabeledSampleGenerator
{
 private:
 
  UnlabeledSGAnalysis *sgAnalysis;
  
  double cpuUsage;
  double elapsedTime;
  
 public:

  UnlabeledSampleGenerator();
  ~UnlabeledSampleGenerator();
  
  int generateSamples(vector<Sentence *> &, vector<Sample *> &, 
                      unsigned short, unsigned short [], bool);  
  
  void incPropCount();
  void incSampleCount();
  
  double getElapsedTime();
  
  void logSGAnalysis(string &);
};


class TestSampleGenerator
{
 private:
 
  TestSGAnalysis *sgAnalysis;
  
  double cpuUsage;
  double elapsedTime;
  
 public:

  TestSampleGenerator();
  ~TestSampleGenerator();
  
  int generateSamples(vector<Sentence *> &, vector<Sample *> &, 
                      unsigned short, unsigned short [], bool);  
  
  void incPropCount();
  void incSampleCount();
  void addArgXPMissed(Argument * const &);
  
  double getElapsedTime();
  
  void logSGAnalysis(string &, bool);
};



/**
 *   sample generation analysis classes
 */

class LabeledSGAnalysis
{
 private:
  
  int sentenceCount;                // number of processed sentences
  int propCount;                    // number of processed propositions
  int positiveCount;
  int negativeCount;
  
  // positive arguments that are potentially missed by X&P heuristic and
  // parse errors when using auto parses
  vector<Argument *> argsXPMissed;
   
 public:
  
  LabeledSGAnalysis ();
  
  void incSentenceCount();
  int getSentenceCount();
  void incPropCount();
  void incPositiveCount();
  int getPositiveCount();
  void incNegativeCount();
  int getNegativeCount();
  
  void addArgXPMissed(Argument * const &);
  void logSGAnalysis(const double &, const double &, string &, bool);
  void logArgsXPMissed(ostream &);
};


class UnlabeledSGAnalysis
{
 private:
  
  int sentenceCount;                // number of processed sentences
  int propCount;                    // number of processed propositions
  int sampleCount;
  
 public:
  
  UnlabeledSGAnalysis ();
  
  void incSentenceCount();
  int getSentenceCount();
  void incPropCount();
  void incSampleCount();
  int getSampleCount();
  
  void logSGAnalysis(const double &, const double &, string &);
};


class TestSGAnalysis
{
 private:
  
  int sentenceCount;                // number of processed sentences
  int propCount;                    // number of processed propositions
  int sampleCount;
  
  // positive arguments that are potentially missed by X&P heuristic and
  // parse errors when using auto parses
  vector<Argument *> argsXPMissed;
   
 public:
  
  TestSGAnalysis ();
  
  void incSentenceCount();
  int getSentenceCount();
  void incPropCount();
  void incSampleCount();
  int getSampleCount();
  
  void addArgXPMissed(Argument * const &);
  void logSGAnalysis(const double &, const double &, string &, bool);
  void logArgsXPMissed(ostream &);
};

#endif /* SAMPLEGENERATOR_HPP */
