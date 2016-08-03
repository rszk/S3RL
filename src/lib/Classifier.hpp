/**
 *  @ RSK
 *  @ Version 1.1        05.04.2010
 * 
 *  Classifier classes to train classifiers, and classify and test samples
 * 
 */

#ifndef CLASSIFIER_HPP
#define CLASSIFIER_HPP

#include <vector>
#include <string>

#include "maxentmodel.hpp"

using namespace std;


class Sentence;
class Sample;

class ZMEClassifier
{
 private:
  
  maxent::MaxentModel zmeModel;
  
  // the number of times the classifier has been trained in a bootstrapping
  // process (-1 for initialization, 0 for the base classifier, and so on)
  unsigned no;

  double cpuUsage;
  double elapsedTime;
  
 public:

  ZMEClassifier();
  
  double getElapsedTime();

  // train() is provided in 2 versions: one accepts a vector of sentences
  // and extracts samples of propositions of each sentence and train by 
  // those samples; the other directly accepts samples and trains using them
  void train(vector<Sentence *> &, unsigned short, unsigned short, bool, 
             const int &, const size_t &, const string &, const double &,
             bool ,const string &, ofstream &);
  void train(vector<Sample *> &, unsigned short, unsigned short, bool,
             const int &, const size_t &, const string &, const double &,
             bool ,const string &, ofstream &);

  // classify() is provided in 2 versions: one accepts a vector of sentences
  // and extracts samples of propositions of each sentence and labels those 
  // samples; the other directly accepts samples and labels them
  // returns the number of sentences of samples labeled 
  unsigned classify(vector<Sentence *> &, unsigned short, bool, ofstream &);
  unsigned classify(vector<Sample *> &, unsigned short, bool, ofstream &);
};

#endif /* CLASSIFIER_HPP */
