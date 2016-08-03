/**
 *  @ RSK
 *  @ Version 0.1        11.12.2009
 * 
 *  (refer to CoNLLSaver.h)
 * 
 */

#include <fstream>
#include <iostream>

#include "CoNLLSaver.hpp"


CoNLLSaver::CoNLLSaver(string &pPropsFile)
{
 propsFile = pPropsFile;

 savedSentencesCount = 0;
 
 cpuUsage = 0;
 elapsedTime = 0;
}

double CoNLLSaver::getElapsedTime()
{
 return elapsedTime;
}

/**
 *  iterates sentences and gathers two kind of data for each to write
 *  in output props file (labeled data):
 * 
 *  1. a vector of strings representing predicates of the sentence according
 *     to 1st column of CoNLL props file (elements are "-" or a lemma) 
 *  
 *  2. some vectors of strings , one for each proposition, representing 
 *     arguments of each proposition according to CoNLL props file (elements
 *     are startinig of argument, ending the argument, or "*")
 * 
 *  each row of the ouput file is written by iteratively choosing and writing
 *  these elements 
 */
int CoNLLSaver::saveProps(vector<Sentence *> &pSentences, unsigned short pFeatureView)
{
 int cntSentence = 0;
 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
  
 ofstream strmProps (propsFile.c_str());
 if(!strmProps)
 {
  cerr << "\nCan't create output file: " << propsFile << endl;
  return -1;
  }
 
 // iterating sentences
 for (vector<Sentence *>::iterator itSentence = pSentences.begin(); 
      itSentence < pSentences.end(); 
      itSentence++, cntSentence++)
 {  
  // getting predicates column vector
  vector<string> vPredTemplate;
  (*itSentence)->getPredTemplate(vPredTemplate);
  
  // getting argument columns vectors (in start/end format (SE))
  vector< vector<string> > vArgSEStrs;
  (*itSentence)->getPropsArgsSEStrs(vArgSEStrs, pFeatureView);
  
  short vColCount = (*itSentence)->getPropCount() + 1;
  
  // looping for writing rows
  for (short cntRow = 0; cntRow < (*itSentence)->getLength(); cntRow++)
  {
   string tmpStr;
   
   // writing first column (predicates)
   if (vPredTemplate[cntRow].empty())
   {
    strmProps.width(10); strmProps << left 
                                   << "-";
    }
   else
   {
    strmProps.width(10); strmProps << left 
                                   << vPredTemplate[cntRow];
    }
   
   // looping for writing proposition columns (arguments in start/end format)
   for (short cntCol = 1; cntCol < vColCount; cntCol++)
   {
    if (vArgSEStrs[cntCol-1][cntRow].empty())
    {
     strmProps.width(16); strmProps << right 
                                    << "* ";
     }
    else
    {
     strmProps.width(16); strmProps << right
                                    << vArgSEStrs[cntCol-1][cntRow];
     } 
    }
   strmProps << "     " << endl;
   }
  
  strmProps << endl;
  cout << "\r" << cntSentence + 1 << " sentences processed" << flush;
  }  
  
 vEndClk = clock();
 cpuUsage = ((double) (vEndClk - vStartClk)) / CLOCKS_PER_SEC;
 time(&vEndTime);
 elapsedTime = difftime(vEndTime, vStartTime);

 savedSentencesCount = cntSentence;

 return 0;
}


