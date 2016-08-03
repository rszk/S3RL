/**
 *  @ RSK
 *  @ Version 1.2        18.05.2010
 * 
 *  (refer to CoNLLLoader.hpp)
 * 
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <time.h>

#include "Constants.hpp"
#include "CoNLLLoader.hpp"
#include "Sentence.hpp"
#include "Utils.hpp"
#include "TBTree.hpp"
#include "DTree.hpp"
#include "DataAnalysis.hpp"


using namespace std;


CoNLLLoader::CoNLLLoader(string &pWords, 
                         string &pSyntCha,
                         string &pSyntDep,
                         string &pProps, 
                         int pMaxSentences)
{
 words = pWords;
 syntCha = pSyntCha;
 syntDep = pSyntDep;
 props = pProps;
 maxSentences = pMaxSentences;

 loadedSentencesCount = 0;
 loadedWordsCount = 0;
 loadedCnstntCount = 0;
 loadedPropsCount = 0;
 loadedArgsCount = 0;
 
 cpuUsage = 0;
 elapsedTime = 0;
   
 dataAnalysis = new DataAnalysis;
}
 
CoNLLLoader::~CoNLLLoader()
{
 delete dataAnalysis;
}
 
double CoNLLLoader::getElapsedTime()
{
 return elapsedTime;
}


/**
 *  @ 19.11.2009
 * 
 *  Opens all required CoNLL data files once (currently only words, charniak
 *  parses, and propositions) then reads them into a vector of a Sentence object
 *  sentence by sentence, where each of these sentences itself is read
 *  word by word (each word with all of its annotations).
 * 
 *  Only a specified number of sentences will be loaded according to the
 *  maxSentences attribute of the class, unless it is set to 0 in which 
 *  case all sentences will be loaded.
 * 
 */
int CoNLLLoader::loadData(vector<Sentence *> & pSentences)
{
 string tmpWordForm, tmpSynt, tmpDep, tmpProps, tmpLemma;
 vector<string> tmpvcSynt, tmpvcDep, tmpvcProps;
 int cntLine = 0;
 int cntSentence = 0;
 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
  
 ifstream strmWords (words.c_str());
 if(! strmWords)
 {
  cerr << "Can't open file: " << words << endl;
  return -1;
  }

 ifstream strmSyntCha (syntCha.c_str());
 if(! strmSyntCha)
 {
  cerr << "Can't open file: " << syntCha << endl;
  return -1;
  }

 ifstream strmSyntDep (syntDep.c_str());
 if(! strmSyntDep)
 {
  cerr << "Can't open file: " << syntDep << endl;
  return -1;
  }

 ifstream strmProps (props.c_str());
 if(! strmProps)
 {
  cerr << "Can't open file: " << props << endl;
  return -1;
  }
 
 // looping to read sentences
 while (!(strmWords.eof() || strmSyntCha.eof()))
 {
  // checking if the maximum number of sentences to be read is reached
  if ((maxSentences != 0) && (cntSentence == maxSentences))
   break;
  
  // reading the 1st line (in CoNLL format) of a sentence from files
  getline(strmWords, tmpWordForm);
  getline(strmSyntCha, tmpSynt);
  getline(strmSyntDep, tmpDep);
  getline(strmProps, tmpProps);
  
  // If the 1st line of a sentence is empty, there is a blank line at the
  // begining of the sentence, which is not allowed, or it is the last line
  // which is left empty by CoNLL format and signals the end of file.
  // In both cases the procedure ends.
  if (isEmptyLine(tmpWordForm))
   break;
  
  cntLine++;

  pSentences.push_back(new Sentence);
  
  tmpHeadIdxs.clear();
  
  // looping to read words and annotations
  while (!isEmptyLine(tmpWordForm))
  { 
   Word * tmpWord = new Word ();
   // loading word form; word lemma is set after reading its POS
   tmpWord->setWordForm(trim(tmpWordForm));
   
   // loading propositions and their arguments of the sentence
   tmpvcProps.clear();
   tokenize(tmpProps, tmpvcProps, " \t\n\r");
   appendToProps(*(pSentences.back()), tmpvcProps, pSentences.back()->getLength()+1);

   // storing the current word in the current sentence
   pSentences.back()->appendWord(tmpWord);
   
   // loading syntactic constituents containing the current word
   // Also, if the current word is marked as predicate, it assigns
   // the related constituent to the proposition predicate
   tmpvcSynt.clear();
   tokenize(tmpSynt, tmpvcSynt, " \t\n\r");
   tmpWord->setWordLemma(extractLemma(tmpWord->getWordForm(), tmpvcSynt.at(0)));
   appendToTBTree(*(pSentences.back()), tmpvcSynt.at(0), tmpvcSynt.at(1), 
                  pSentences.back()->getLength(),
                  ((tmpvcProps[0]) != "-"));

   // loading dependency syntax (head and relation)
   tmpvcDep.clear();
   tokenize(tmpDep, tmpvcDep, " \t\n\r");
   tmpHeadIdxs.push_back(strToInt(tmpvcDep[0]));
   appendToDTree(*(pSentences.back()), tmpWord, tmpvcDep[1]);
   
   // reading the next lines (in CoNLL format) of a sentence from files
   getline(strmWords, tmpWordForm);
   getline(strmSyntCha, tmpSynt);
   getline(strmSyntDep, tmpDep);
   getline(strmProps, tmpProps);
   cntLine++;
   }
  
  // controlling CoNLL files consistency
  if ((isEmptyLine(tmpWordForm)) && (!isEmptyLine(tmpSynt)))
  {
   cerr << "\nThere is a mismatch between word file and syntax file in line "
        << cntLine << endl;
   return -1;
   }
  
  if ((isEmptyLine(tmpWordForm)) && (!isEmptyLine(tmpProps)))
  {
   cerr << "\nThere is a mismatch between word file and props file in line "
        << cntLine << endl;
   return -1;
   }

  if ((isEmptyLine(tmpWordForm)) && (!isEmptyLine(tmpDep)))
  {
   cerr << "\nThere is a mismatch between word file and dependency file in line "
        << cntLine << endl;
   return -1;
   }

  //  setting heads of dependency tree of the senetence which had been delayed
  //  to after completing the reading if it
  pSentences.back()->setDTreeHeads(tmpHeadIdxs);
  
  cntSentence++;
  
  cout << "\r" << cntSentence << " sentences loaded" << flush;
  /*
  cout << endl;
  cout << cntSentence << "- ";
  pSentences.back()->display(cout);
  pSentences.back()->displayDTree(cout);
  pSentences.back()->displayTree();
  pSentences.back()->displayProps();
  */
  
  // analyze sentence for checking consistency of loading and also
  // extracting statistics according to DataAnalysis struture
  pSentences.back()->analyzeData(dataAnalysis);
  
  //getchar();
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
 *  @ 27.11.2009
 * 
 *  extracts propositions and their arguments from CoNLL data for each
 *  sentence
 * 
 *  one line at a call is passed to this function and it creates propositions
 *  when sees the first line and collects their argument labels and boundaries
 *  increasingly by each line 
 * 
 *  to map an argument to a constituent token, in the next step which
 *  is to load syntactic constituents, after completing the boundaries of
 *  a constituent (TBTree node), it looks for arguments loaded so far 
 *  for the propositions of current sentence and is assigned to one if the
 *  boundaries exactly align (this way, some arguments may remain unassigned
 *  due to errornous parses, which will be treated in a post-processing
 *  if needed)
 * 
 *  this is more efficient than alternative method, which is to load 
 *  syntactic constituents first and then look for alignment for and 
 *  argument among those constituents, because number of constituents
 *  to search is substantially more than number of arguments
 */
void CoNLLLoader::appendToProps(Sentence &pCurrSentence,
                                const vector<string> &propsLine, 
                                const WordIdx &pWord)
{
 string tmpLabel = "";
 short unsigned propCount = propsLine.size()-1;
 
 // if this is the begining of sentence, create propostions of the sentence
 // and set the size of the vector keeping last read arguments
 if (pWord == 1)
 {
  pCurrSentence.appendProp(propCount);
  loadedPropsCount +=  propCount;
  lastArg.resize(propCount);
  predRead = 0;
  }
 
 // If this line contains a lemma, then increase the number of predicate
 // lemma read so far (++predRead) and then assign this lemma to the 
 // proper proposition according to this number.
 // This is the best tried way of setting lemma which is also compatible 
 // with the desing of all CoNLL's data.
 if (propsLine.at(0) != "-")
 { 
  predRead++;
  pCurrSentence.setPropPredLemma(predRead, propsLine.at(0));
  pCurrSentence.setPropPredWord(predRead, pWord);
  }
 
 for (unsigned int cntrProp=1; cntrProp <= propCount; cntrProp++)
  for (unsigned int cntrTagChr=0; 
       cntrTagChr < propsLine[cntrProp].size(); 
       cntrTagChr++)
   if (propsLine[cntrProp][cntrTagChr] == '*')
   {
    if (!tmpLabel.empty())
    {
     // If this is the label for predicate ("V" or "C-V" according to 
     // CoNLL annotation),set the predicate's span start; span end will 
     // be set when the closing bracket is reached, and constituent will
     // be assigned later when appending the TBTree nodes (appendToTBTree
     // function); lemma has already been set above.
     //
     // Otherwise, it should be core or adjunct argument label, create it
     // setting its span start and label; span end and constituent 
     // assignment is done later like predicate 
     if (tmpLabel == "V")
     {
      pCurrSentence.setPropPredSpanStart(cntrProp, pWord); 
      }
     else if (tmpLabel == "C-V") 
     {
      pCurrSentence.setPropPredCSpanStart(cntrProp, pWord); 
      }	
     else  
     {
      pCurrSentence.addPropArgument(cntrProp, pWord, tmpLabel);
      loadedArgsCount++;
      }

     lastArg.at(cntrProp-1) = tmpLabel;
     tmpLabel.clear();
     }
    
    } 
   else if (propsLine[cntrProp][cntrTagChr] == ')')
   {
    if (lastArg.at(cntrProp-1) == "V")
     pCurrSentence.setPropPredSpanEnd(cntrProp, pWord); 
    else if (lastArg.at(cntrProp-1) == "C-V")
     pCurrSentence.setPropPredCSpanEnd(cntrProp, pWord);
    else
     pCurrSentence.setPropArgSpanEnd(cntrProp, 
                                     pCurrSentence.getPropArgCount(cntrProp), 
                                     pWord); 
    }
   else if (propsLine[cntrProp][cntrTagChr] != '(')
    tmpLabel.append(propsLine[cntrProp].substr(cntrTagChr,1)); 
};

/**
 *  @ 25.11.2009
 * 
 *  Analyzes the sequence of syntactic tags associated with each word
 *  (line) in CoNLL format and gradually create the tree for the sentence
 *  that is passed as parameter.
 * 
 *  To fullfil this task, this function is called several times for each 
 *  senetnce (equal to the number of the words for each sentence), so it
 *  remebers everything done in previous calles with the help of a stack
 *  as its class member
 * 
 *  Another main job is to find the aligned argument for each constituent
 *  as soon as its span is recognized (closing bracket is reached). It looks
 *  for arguments of propositions of the current sentence extracted so far
 *  whose span is exaclty the same as that of itself.
 * 
 */
void CoNLLLoader::appendToTBTree(Sentence &pCurrSentence,
                                 const string &pPOS, 
                                 const string &pTagStr, 
                                 const WordIdx pWord,
                                 const bool &pIsPredicate)
{
 string tmpTag = "";
 
 // if this is the begining of sentence, initilize tree stack
 if (pWord == 1)
  tmpTBStack.clear();
 
 for (unsigned int i=0; i < pTagStr.size(); i++)
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
    tbTreeNode->setProbableArgs();
    // if there is probably a predicate continuation in this span (C-V)
    // set its constituent (predCCnstnt) to this node
    pCurrSentence.setProbablePropPredCCnstnt(tbTreeNode);
    // assigns the terminal node to the corresponding word object
    pCurrSentence.setWordCnstnt(pWord, tbTreeNode);
    // if this current word is a predicate, assign this constituent to 
    // the related proposition's predicate
    if (pIsPredicate)
     tbTreeNode->assignToPropPred(predRead);
   }
  else if (pTagStr[i] == ')')
  {
   tmpTBStack.back()->setLastWord(pWord);
   tmpTBStack.back()->setProbableArgs();
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
void CoNLLLoader::appendToDTree(Sentence &pSentence,
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
void CoNLLLoader::logLoadedData()
{
 string tmpLogFileName = DEFAULT_LOG_PATH;
 tmpLogFileName += "/loaded." + extractFileName(words, true) + ".log";
 ofstream strmLog (tmpLogFileName.c_str());
 if(strmLog.is_open())
 {
  strmLog << loadedSentencesCount << " sentences were loaded in " 
          << cpuUsage << "/" << elapsedTime << " seconds, containing: " 
          << endl << endl
          << loadedWordsCount << " Words" << endl
          << loadedCnstntCount << " Constituents" << endl
          << loadedPropsCount << " Propositions" << endl
          << loadedArgsCount << " Arguments" << endl
          << endl
          << endl;
  
  strmLog.close();
  }
}

/**
 *  passes name of word file is passed as the dataset ID to create the log file name
 */
void CoNLLLoader::logDataAnalysis()
{
 dataAnalysis->logDataAnalysis(words);
}
