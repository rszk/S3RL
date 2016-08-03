/**
 *  @ RSK
 *  @ Version 1.1        03.03.2010
 * 
 *  All function are motivated from CoNLLLoader functions, and it has been
 *  tried to retain the maximum consistency between these two. So, some 
 *  code may seem unreasonable or improvable.
 * 
 *  (refer to ULoader.hpp)
 * 
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <time.h>

#include "Constants.hpp"
#include "ULoader.hpp"
#include "Sentence.hpp"
#include "Utils.hpp"
#include "TBTree.hpp"
#include "DTree.hpp"
#include "DataAnalysis.hpp"


using namespace std;


ULoader::ULoader(string &pDataFile, string &pSyntDepFile, int pMaxSentences)
{
 dataFile = pDataFile;
 syntDepFile = pSyntDepFile;
 maxSentences = pMaxSentences;

 loadedSentencesCount = 0;
 loadedWordsCount = 0;
 loadedCnstntCount = 0;
 loadedPredsCount = 0;
 
 cpuUsage = 0;
 elapsedTime = 0;
}
 
double ULoader::getElapsedTime()
{
 return elapsedTime;
}


/**
 *  @ 22.01.2010
 * 
 *  Opens unlabeled data file in CoNLL format (containing words, Charniak
 *  parses and POS tags) then reads them into a vector of a Sentence object
 *  sentence by sentence, where each of these sentences itself is read
 *  word by word (each word with all of its annotations).
 * 
 *  Only a specified number of sentences will be loaded according to the
 *  maxSentences attribute of the class, unless it is set to 0 in which 
 *  case all sentences will be loaded.
 * 
 */
int ULoader::loadData(vector<Sentence *> & pSentences)
{
 string tmpLine, tmpDep;
 vector<string> tmpvcTokens, tmpvcDep;
 int cntLine = 0;
 int cntSentence = 0;
 bool tmpIsPredicate = false;
 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
  
 ifstream strmData (dataFile.c_str());
 if(! strmData)
 {
  cerr << "Can't open file: " << dataFile << endl;
  return -1;
  }

 ifstream strmSyntDep (syntDepFile.c_str());
 if(! strmSyntDep)
 {
  cerr << "Can't open file: " << syntDepFile << endl;
  return -1;
  }

 // looping to read sentences
 while (!strmData.eof())
 {
  // checking if the maximum number of sentences to be read is reached
  if ((maxSentences != 0) && (cntSentence == maxSentences))
   break;
  
  // reading the 1st line (in CoNLL format) of a sentence from files
  getline(strmData, tmpLine);
  getline(strmSyntDep, tmpDep);
  
  // If the 1st line of a sentence is empty, there is a blank line at the
  // begining of the sentence, which is not allowed, or it is the last line
  // which is left empty by CoNLL format and signals the end of file.
  // In both cases the procedure ends.
  if (isEmptyLine(tmpLine))
   break;
  
  cntLine++;

  pSentences.push_back(new Sentence);
  
  tmpHeadIdxs.clear();
  
  // looping to read words and annotations
  while (!isEmptyLine(tmpLine))
  { 
   // tokenizing the line read
   tmpvcTokens.clear();
   tokenize(tmpLine, tmpvcTokens, " \t\n\r");
   
   Word * tmpWord = new Word ();
   
   // loading word form and lemma
   tmpWord->setWordForm(trim(tmpvcTokens[0]));
   tmpWord->setWordLemma(extractLemma(tmpWord->getWordForm(), tmpvcTokens[1]));
   
   // currently, we consider all words with the POS tag starting with "VB"
   // as predicate except auxiliary verbs (including "be" verbs) wrongly 
   // tagged with "VB"
   tmpIsPredicate = isVerbPredicate(tmpvcTokens[0], tmpvcTokens[1]);
   
   if (tmpIsPredicate)
   {
    string tmpPredLemma = tmpWord->getWordLemma();
    appendToProps(*(pSentences.back()), tmpPredLemma, pSentences.back()->getLength()+1);
    }

   // storing the current word in the current sentence
   pSentences.back()->appendWord(tmpWord);

   // loading syntactic constituents containing the current word
   // Also, if the current word is marked as predicate, it assigns
   // the related constituent to the proposition predicate
   appendToTBTree(*(pSentences.back()), tmpvcTokens[1], tmpvcTokens[2], 
                  pSentences.back()->getLength(), tmpIsPredicate);
   
   // loading dependency syntax (head and relation)
   tmpvcDep.clear();
   tokenize(tmpDep, tmpvcDep, " \t\n\r");
   tmpHeadIdxs.push_back(strToInt(tmpvcDep[0]));
   appendToDTree(*(pSentences.back()), tmpWord, tmpvcDep[1]);
   
   // reading the next lines (in CoNLL format) of a sentence from files
   getline(strmData, tmpLine);
   getline(strmSyntDep, tmpDep);
   cntLine++;
   }
  
  // processes all propositions of the sentence to figure out whether their
  // predicates is simple, compound or discontinues compound, and then 
  // changes span end (if needed) and sets the continuation constituent 
  // and its span too
  pSentences.back()->estimatePropPredBoundaries();
  
  // controlling CoNLL files consistency
  if ((isEmptyLine(tmpLine)) && (!isEmptyLine(tmpDep)))
  {
   cerr << "\nThere is a mismatch between data file and dependency  file in line "
        << cntLine << endl;
   return -1;
   }
  
  //  setting heads of dependency tree of the senetence which had been delayed
  //  to after completing the reading if it
  pSentences.back()->setDTreeHeads(tmpHeadIdxs);
  
  cntSentence++;
  
  cout << "\r" << cntSentence << " sentences loaded" << flush;
  
/*cout << endl;
  cout << cntSentence << "- ";
  pSentences.back()->display(cout);
  pSentences.back()->displayDTree(cout);
  pSentences.back()->displayTree();
  pSentences.back()->displayProps();
  getchar();
*/ 
  
 }
 
 vEndClk = clock();
 cpuUsage = ((double) (vEndClk - vStartClk)) / CLOCKS_PER_SEC;
 time(&vEndTime);
 elapsedTime = difftime(vEndTime, vStartTime);

 loadedSentencesCount = cntSentence;
 loadedWordsCount = cntLine - cntSentence;
 
 return 0;
};

/**
 *  @ 22.01.2010
 * 
 *  append a proposition for the sentence and sets its lemma and word
 * 
 */
void ULoader::appendToProps(Sentence &pCurrSentence,
                                const string &predLemma, 
                                const WordIdx &pWord)
{
 pCurrSentence.appendProp(1);
 loadedPredsCount++;
 
 pCurrSentence.setPropPredSpanStart(pCurrSentence.getPropCount(), pWord); 
 // span end may be changed later when identifying predicate compoundness
 pCurrSentence.setPropPredSpanEnd(pCurrSentence.getPropCount(), pWord); 
 pCurrSentence.setPropPredLemma(pCurrSentence.getPropCount(), predLemma);
 pCurrSentence.setPropPredWord(pCurrSentence.getPropCount(), pWord);
};

/**
 *  @ 22.01.2010
 * 
 *  Analyzes the sequence of syntactic tags associated with each word
 *  (line) in CoNLL format and gradually create the tree for the sentence
 *  that is passed as parameter.
 * 
 *  To fullfil this task, this function is called several times for each 
 *  senetnce (equal to the number of the words for each sentence), so it
 *  remebers everything done in previous calles with the help of a stack
 *  as its class member.
 */
void ULoader::appendToTBTree(Sentence &pCurrSentence,
                             const string &pPOS, 
                             const string &pTagStr, 
                             const WordIdx pWord,
                             const bool &pIsPredicate)
{
 string tmpTag = "";
 
 // if this is the begining of sentence, initilize tree stack
 if (pWord == 1)
  tmpTBStack.clear();
 
 for (unsigned int i = 0; i < pTagStr.size(); i++)
  if (pTagStr[i] == '(')
  {
   if (!tmpTag.empty())
   {
    // if the stack is empty, this is the topmost node and parent is NULL,
    // otherwise parent is the top node in stack
    // it is also not considered as constituent
    if (tmpTBStack.empty())
    {
     try
     {
      TBTree *tbTreeNode = new TBTree (pCurrSentence, tmpTag, pWord);
      pCurrSentence.setTBTree(tbTreeNode);
      tmpTBStack.push_back(tbTreeNode);
      }
     catch (bad_alloc &)
     {  
      cerr << "Memory allocation failed for TBTree!" << endl;
      exit(EXIT_FAILURE);
      }
     }
    else
    {
     try
     {
      TBTree *tbTreeNode = new TBTree (pCurrSentence, *tmpTBStack.back(),
                                       tmpTag, pWord);
      loadedCnstntCount++;
      tmpTBStack.push_back(tbTreeNode);
      }
     catch (bad_alloc &)
     {  
      cerr << "Memory allocation failed for TBTree!" << endl;
      exit(EXIT_FAILURE);
      }
     }
    tmpTag.clear(); 
    } 
   }
  else if (pTagStr[i] == '*')
  {
   if (!tmpTag.empty())
   {
    try
    {
     TBTree *tbTreeNode = new TBTree (pCurrSentence, *tmpTBStack.back(), 
                                      tmpTag, pWord);
     loadedCnstntCount++;
     tmpTBStack.push_back(tbTreeNode);
     tmpTag.clear(); 
     }
    catch (bad_alloc &)
    {  
     cerr << "Memory allocation failed for TBTree!" << endl;
     exit(EXIT_FAILURE);
     }
    } 
    // POS tag of current word is added to the tree as the leaf node
    // with firstWord = lastWord but not pushed back to stack, since 
    // it will not be needed to be assigned as parent for some others.
    TBTree *tbTreeNode = new TBTree (pCurrSentence, *tmpTBStack.back(), 
                                     pPOS, pWord);
    
    loadedCnstntCount++;
    tbTreeNode->setLastWord(pWord);
    // assigns the terminal node to the corresponding word object
    pCurrSentence.setWordCnstnt(pWord, tbTreeNode);
    // if this current word is a predicate, assign this constituent to 
    // the related proposition's predicate
    if (pIsPredicate)
     tbTreeNode->assignToPropPred(pCurrSentence.getPropCount());
   }
  else if (pTagStr[i] == ')')
  {
   tmpTBStack.back()->setLastWord(pWord);
   tmpTBStack.pop_back();
   }
  else
   tmpTag.append(pTagStr.substr(i,1)); 
};


/**
 *  @ 03.03.2010
 * 
 *  Creates a dependency tree node (DTree node or DTNode) and assigns
 *  the corresponding sentence and word together with its dependency relation
 *  to the parent (head). However, since this function is called pregressively
 *  as words are read from input, the parent may have not been created yet.
 *  Thus, parents (heads) of all nodes are set after completing the reading 
 *  and creating each sentence.
 * 
 */
void ULoader::appendToDTree(Sentence &pSentence,
                            WordPtr &pWord, 
                            const string &pDepRel)
{
 DTNode dTreeNode = new DTree (pSentence, pWord, pDepRel);
 pWord->setDTNode(dTreeNode);
}


/**
 *  Logs the loading results such as number of sentences and words read 
 *  from data into LoadedData.log file in /log
 */
void ULoader::logLoadedData()
{
 ofstream strmLog (LOG_LOADED_UNLABELD_DATA_FILE);
 if(strmLog.is_open())
 {
  strmLog << loadedSentencesCount << " sentences were loaded in " 
          << cpuUsage << "/" << elapsedTime << " seconds, containing: " 
          << endl << endl
          << loadedWordsCount << " words" << endl
          << loadedCnstntCount << " Constituents" << endl
          << loadedPredsCount << " Predicates" << endl
          << endl << endl;
  
  strmLog.close();
  }
}
