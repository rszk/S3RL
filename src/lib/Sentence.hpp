/**
 *  @ RSK
 *  @ Version 1.1        03.03.2010
 * 
 *  Classes to store sentences (training, development, and testing)
 * 
 *  A Sentence is the base class
 *  A Sentence consists of some Words
 *  A Sentence has a corresponding Treebank Tree (TBTree; see TBTree.hpp)
 *  A Sentence consists of some Propositions
 *  A Proposition consists of a Predicate (which is referred to by word
 *   span and corresponding constituent
 *  A Proposition consists of some Arguments (see Argument.hpp)
 *  A Proposition has some Samples generated for training, test, or labeling
 *   (see Sample.hpp)
 * 
 *  A Word includes wordform, lemma
 */

#ifndef SENTENCE_HPP
#define SENTENCE_HPP

#include <vector>
#include <string>

#include "Types.hpp"

using namespace std;


class DataAnalysis;
class LabeledSampleGenerator;
class UnlabeledSampleGenerator;
class TestSampleGenerator;

// forward declaration (instead of including hpp) prevents circular 
// compilation dependency or include problem)
class TBTree;
class DTree;
class Argument;
class Sample;

class Word;
class Proposition;


class Sentence
{
 private:
 
  vector<Word *> words;
  TBTree *tbTree;
  DTree *dTree;
  vector<Proposition *> props;
 
 public:
 
  Sentence();
  
  int appendWord(Word * const &);
  WordPtr getWord(WordIdx);
  string getWordForm(WordIdx);
  WordIdx getWordIdx(WordPtr);
  void setWordCnstnt(WordIdx, Constituent &);
  Constituent getWordCnstnt(WordIdx);
  string getWordsStrInSpan(WordSpan &);
  string getWordsLemmaStrInSpan(WordSpan &);
  void clearWords();
  short getLength();
  void display(ostream &);
  
  DTNode getDTree();
  void setDTreeHeads(vector<WordIdx> &);
  void displayDTree(ostream &);
  
  void setTBTree(TBTree * const &);
  TBTree *getTBTree();
  void displayTree();
  
  void appendProp(const unsigned short &);
  void getProps(vector<Proposition *> &);
  short getPropCount();
  void setPropPredWord(const short &, const WordIdx &);
  void getPredTemplate(vector<string> &);
  void setPropPredSpanStart(const short &, const WordIdx &); 
  void setPropPredSpanEnd(const short &, const WordIdx &); 
  void setPropPredCSpanStart(const short &, const WordIdx &); 
  void setPropPredCSpanEnd(const short &, const WordIdx &); 
  void setProbablePropPredCCnstnt(Constituent &);
  void setPropPredLemma(const short &, const string &); 
  string getPropPredLemma(const short &);
  void estimatePropPredBoundaries();
  void addPropArgument(const short &, const WordIdx &, const string &); 
  short getPropArgCount(const short &);
  void getPropsArgsSEStrs(vector< vector<string> > &, unsigned short);
  void setPropArgSpanEnd(const short &, const short &, const WordIdx &);
  void findPropArgsInSpan(vector<Argument *> &, WordSpan &);
  void assignToPropPred(const short &, Constituent const &);
  void displayProps();
  
  void analyzeData(DataAnalysis *&);
  
  int generateLabeledSamples(LabeledSampleGenerator * const &, 
                             vector<Sample *> &, vector<Sample *> &,
                             unsigned short, unsigned short [],
                             ofstream &);
  int generateUnlabeledSamples(UnlabeledSampleGenerator * const &, 
                               vector<Sample *> &, 
                               unsigned short, unsigned short [],
                               ofstream &); 
  int generateTestSamples(TestSampleGenerator * const &, 
                          vector<Sample *> &, 
                          unsigned short, unsigned short [],
                          ofstream &);
  unsigned short getSampleCount();
  
  double getLabelingProbability(unsigned short);
  double getLabelingAgreement();
  // sets the common label between predictions of the views based on the
  // selection criterion selected by user
  void setSelectedPrdLabels(unsigned short, unsigned short);
  // logs predicated labeling of samples into a file
  void logPrdLabeling(unsigned short, ofstream &);
  void logPrdLabelings(unsigned short, ofstream &);
};




class Word
{
 private:
  
  string wordForm;
  string wordLemma;

  // the corresponding terminal TBTree node
  Constituent wordCnstnt;
  // the corresponding DTree node
  DTNode wordDTNode;
 
 public:
  
  Word ();
  
  void setWordForm(string);      
  string getWordForm();      
  string getPOS();      
  void setWordLemma(string);      
  string getWordLemma();   

  void setCnstnt(Constituent &);
  Constituent getCnstnt();
  void setDTNode(DTNode &);
  DTNode getDTNode();
  WordIdx getDepHeadWordIdx();

};



class Proposition
{
 private:
 
  Sentence *sentence;
  
  WordIdx predWord;
  // We keep predicate's span, since there is no 1-to-1 mapping between
  // predicate and constituents in CoNLL data, and a predicate is assigned
  // to one or two words (two in the case of particles)
  WordSpan predSpan;
  string predLemma;
  Constituent predCnstnt;
  // the followings are for discontinous predicates (pred Continuation Span);
  // predWord and predLemma are considered only for first part (v)
  WordSpan predCSpan;
  Constituent predCCnstnt;

  // We currently does not distinguish between constituent-based (cArgs) and 
  // dependency-based arguments (dArgs), because we derive dArgs from cArgs
  // (args currently) according to rules in Surdeanu et al. (2008) to avoid
  // errors occuring in conversion between them which is needed by co-training.
  // Also in this way, we consider a 1-to-1 relation between constituency-based 
  // and dependency-based arguments, which is not exact as mentioned in
  // the conversion process in Surdeanu et al. (2008).
  vector<Argument *> args;
  // vector<CArgument *> cArgs;
  // vector<DArgument *> dArgs;
  
  vector<Sample *> samples;    //samples generated for the proposition
 
 public:
 
  Proposition(Sentence * const &);
  
  Sentence *getSentence();
  
  void setPredWord(const WordIdx &);
  WordIdx getPredWordIdx();
  WordPtr getPredWord();
  void setPredSpanStart(const WordIdx &); 
  void setPredSpanEnd(const WordIdx &); 
  void setPredCSpanStart(const WordIdx &); 
  void setPredCSpanEnd(const WordIdx &); 
  void setPredLemma(const string &);
  WordSpan getPredSpan();
  WordSpan getPredCnstntSpan();
  short getPredLength();
  string getPredLemma();
  string getPredPOS();
  bool getPredVoice();
  string getPredForm();
  void setPredCnstnt(Constituent const &);
  Constituent getPredCnstnt();
  void setProbablePredCCnstnt(Constituent const &);
  Constituent getPredCCnstnt();
  void estimatePredBoundaries();
  
  // implementing Igo&Riloff's passive voice detection rules
  bool isIgoOrdinaryPassive1();
  bool isIgoOrdinaryPassive2();
  bool isIgoReducedPassive1();
  bool isIgoReducedPassive2();
  bool isIgoReducedPassive3();
  bool isIgoReducedPassive4();
  
  void addArgument(const WordIdx &, const string &); 
  short getArgCount();
  void setArgSpanEnd(const short &, const WordIdx &);
  void getPredSEStr(vector<string> &);
  void getArgsSEStrs(vector<string> &, unsigned short);
  Argument* findArgInSpan(WordSpan &);
  Argument* findArgMatch(Constituent &);
  void displayArgs();

  void analyzeData(DataAnalysis *&);
  // since we are not using inherietence for sample genrators, we have to 
  // override this for training and test (unfortunately)
  void checkForXPMissedArgs(LabeledSampleGenerator * const &, vector<Argument *> &);
  void checkForXPMissedArgs(TestSampleGenerator * const &, vector<Argument *> &);
  
  int generateLabeledSamples(LabeledSampleGenerator * const &, 
                              vector<Sample *> &, vector<Sample *> &,
                              unsigned short, unsigned short [], 
                              ofstream &); 
  int generatePositives(LabeledSampleGenerator * const &, 
                        vector<Sample *> &, unsigned short, unsigned short [],
                        ofstream &);
  int generateNegatives(LabeledSampleGenerator * const &, 
                        vector<Sample *> &, unsigned short, unsigned short [],
                        ofstream &);

  int generateUnlabeledSamples(UnlabeledSampleGenerator * const &, 
                               vector<Sample *> &, unsigned short, unsigned short [],
                               ofstream &); 
  
  int generateTestSamples(TestSampleGenerator * const &, 
                          vector<Sample *> &, unsigned short, unsigned short [],
                          ofstream &); 
  void addSample(Sample * const &);
  void getSamples(vector<Sample *> &);
  unsigned short getSampleCount();
};

#endif /* SENTENCE_HPP */
