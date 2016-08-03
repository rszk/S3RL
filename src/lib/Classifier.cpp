/**
 *  @ RSK
 *  @ Version 1.1        05.04.2010
 * 
 *  (refer to Classifier.hpp)
 * 
 */

#include <fstream>
#include <iostream>

#include "maxentmodel.hpp"

#include "Classifier.hpp"
#include "Sample.hpp"
#include "Sentence.hpp"
#include "Utils.hpp"


ZMEClassifier::ZMEClassifier()
{
 no = -1;
 cpuUsage = 0;
 elapsedTime = 0;
}

double ZMEClassifier::getElapsedTime()
{
 return elapsedTime;
}

/**
 *  This version of train() accepts a vector of example training sentences
 *  and extract samples of propositions of each sentence and train by those
 *  samples. 
 *  
 *  pContextView determines the view to extract context events for samples,
 *  and pLabelView determines the view to extract the label for samples. 
 * 
 *  The benefit is that any sentence- or proposition-based post-processing
 *  for optimization and also some quality criteria for selection from 
 *  newly labeled unlabeled samples (for self-training purposes) are made
 *  possible.
 * 
 *  Because training with both labeled (gold) data and newly labeled unlabeled
 *  data (predicted) is done by this function, and a Sample contains both
 *  gold and predicted labels, the 3rd parameter is used to determine which 
 *  is aimed.
 */
void ZMEClassifier::train(vector<Sentence *> &pSentences,
                          unsigned short pContextView,
                          unsigned short pLabelView,
                          bool pGold,
                          const int &pVerbose,
                          const size_t &pIteration,
                          const string &pMethod,
                          const double &pGaussian,
                          bool pSaveModel,
                          const string &pModelFile,
                          ofstream &pLog)
{
 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
  
 maxent::verbose = pVerbose;
 zmeModel.begin_add_event();

 unsigned int cntSamples = 0;
 vector<Sentence *>::iterator itSentence;
 for (itSentence = pSentences.begin(); 
      itSentence < pSentences.end(); 
      itSentence++)
 {
  vector<Proposition *> vProps;
  
  (*itSentence)->getProps(vProps);
  for (vector<Proposition *>::iterator itProp = vProps.begin(); 
       itProp < vProps.end(); 
       itProp++)
  {
   vector<Sample *> vSamples;
   
   (*itProp)->getSamples(vSamples);	     
   for (vector<Sample *>::iterator itSample = vSamples.begin(); 
        itSample < vSamples.end(); 
        itSample++)
   {
    vector<string> context;
    (*itSample)->getZMEContext(context, pContextView);
   
    if (pGold)
     zmeModel.add_event(context, (*itSample)->getGoldArgLabel(), 1);
    else
     // When self-training, a portion of samples have only gold labels
     // (labeled data). So, the gold label should be used instead of 
     // predicted one which does not exist. getPrdOrGoldArgLabel() does
     // it.
     zmeModel.add_event(context, (*itSample)->getPrdOrGoldArgLabel(pLabelView), 1);
    
    cntSamples++;   
    }
   } 

  cout << "\r" << itSentence - pSentences.begin() + 1 << " sentences processed ("
               << cntSamples << " events added)          " << flush;
  } 
 
 pLog << itSentence - pSentences.begin() << " sentences processed ("
               << cntSamples << " events added)          " << flush;

 if (pVerbose == 1)
  cout << endl;
 zmeModel.end_add_event();

 // training model
 zmeModel.train(pIteration, pMethod, pGaussian);
 no++;

 //saving training model
 if (pSaveModel)
 {
  mout("\nSaving trained model of view " + intToStr(pContextView) + " ...\n", cout, pLog);
  zmeModel.save(pModelFile + intToStr(pContextView));
  mout("Saving trained model is done!\n", cout, pLog);
  }
 
 vEndClk = clock();
 cpuUsage = ((double) (vEndClk - vStartClk)) / CLOCKS_PER_SEC;
 time(&vEndTime);
 elapsedTime = difftime(vEndTime, vStartTime);
}


/**
 *  This version of train() directly accepts samples and trains using them
 * 
 *  Because training with both labeled (gold) data and newly labeled unlabeled
 *  data (predicted) is done by this function, and a Sample contains both
 *  gold and predicted labels, the 3rd parameter is used to determine which 
 *  is aimed.
 */
void ZMEClassifier::train(vector<Sample *> &pSamples,
                          unsigned short pContextView,
                          unsigned short pLabelView,
                          bool pGold,
                          const int &pVerbose,
                          const size_t &pIteration,
                          const string &pMethod,
                          const double &pGaussian,
                          bool pSaveModel,
                          const string &pModelFile,
                          ofstream &pLog)
{
 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
  
 maxent::verbose = pVerbose;
 zmeModel.begin_add_event();

 // adding samples to events
 vector<Sample *>::iterator itSample;
 for (itSample = pSamples.begin(); 
      itSample < pSamples.end(); 
      itSample++)
 {
  vector<string> context;
  (*itSample)->getZMEContext(context, pContextView);
   
  if (pGold)
   zmeModel.add_event(context, (*itSample)->getGoldArgLabel(), 1);
  else
   // When self-training, a portion of samples have only gold labels
   // (labeled data). So, the gold label should be used instead of 
   // predicted one which does not exist. getPrdOrGoldArgLabel() does
   // it.
   zmeModel.add_event(context, (*itSample)->getPrdOrGoldArgLabel(pLabelView), 1);
  
  cout << "\r" << itSample - pSamples.begin() + 1
               << " samples added" << flush;
  }

 pLog << itSample - pSamples.begin()
     << " samples added" << flush;

 if (pVerbose == 1)
  cout << endl;
 zmeModel.end_add_event();

 // training model
 zmeModel.train(pIteration, pMethod, pGaussian);
 no++;

 //saving training model
 if (pSaveModel)
 {
  mout("\nSaving trained model ...\n", cout, pLog);
  zmeModel.save(pModelFile);
  mout("Saving trained model is done!\n", cout, pLog);
  }
 
 vEndClk = clock();
 cpuUsage = ((double) (vEndClk - vStartClk)) / CLOCKS_PER_SEC;
 time(&vEndTime);
 elapsedTime = difftime(vEndTime, vStartTime);
}

/**
 *  This version of classify() accepts a vector of labeling sentences
 *  and extract samples of propositions of each sentence for the given 
 *  feature view and labels those samples
 * 
 *  The benefit is that any sentence- or proposition-based post-processing
 *  for optimization is made possible
 * 
 *  returns the number of sentences labeled
 */
unsigned ZMEClassifier::classify(vector<Sentence *> &pSentences,
                                 unsigned short pFeatureView, 
                                 bool pCheckConstraints, 
                                 ofstream &pLog)
{
 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
  
 unsigned int cntSamples = 0, cntIgnored = 0;
 vector<Sentence *>::iterator itSentence;
 
 for (itSentence = pSentences.begin(); 
      itSentence < pSentences.end(); 
      itSentence++)
 {
  bool vIgnore = false;
  
  vector<Proposition *> vProps;
  
  (*itSentence)->getProps(vProps);
  for (vector<Proposition *>::iterator itProp = vProps.begin(); 
       (itProp < vProps.end()) && (!vIgnore); 
       itProp++)
  {
   vector<Sample *> vSamples;
   
   (*itProp)->getSamples(vSamples);	     
   for (vector<Sample *>::iterator itSample = vSamples.begin(); 
        (itSample < vSamples.end()) && (!vIgnore); 
        itSample++)
   {
    vector<string> vContext;
    (*itSample)->getZMEContext(vContext, pFeatureView);
   
    vector<pair<ArgLabel, double> > vPrdLabels;          
    zmeModel.eval_all(vContext, vPrdLabels);
    
    // if this sample was not labeled becasue it has been previously labeled
    // with this no of the classifier, do not continue labeling this sentence
    if (!(*itSample)->setPrdArgLabels(vPrdLabels, pFeatureView, no, pCheckConstraints))
     vIgnore = true;
    
    cntSamples++;   
    }
   } 

  if (vIgnore)
   cntIgnored++;
  
  cout << "\r" << itSentence - pSentences.begin() + 1 << " sentences processed ("
               << cntSamples << " samples)          " << flush;
  } 
 
 cout << endl << itSentence - pSentences.begin() - cntIgnored << " sentences labeled";
 cout << endl << cntIgnored << " sentences ignored relabeling" << endl;

 pLog << itSentence - pSentences.begin() - cntIgnored << " sentences labeled" << endl;
 pLog << cntIgnored << " sentences ignored relabeling" << endl;

 vEndClk = clock();
 cpuUsage = ((double) (vEndClk - vStartClk)) / CLOCKS_PER_SEC;
 time(&vEndTime);
 elapsedTime = difftime(vEndTime, vStartTime);
 
 return itSentence - pSentences.begin() - cntIgnored;
}

/**
 *  This version of classify() accepts a vector of samples and labels those
 *  samples
 */
unsigned ZMEClassifier::classify(vector<Sample *> &pSamples, 
                                 unsigned short pFeatureView,
                                 bool pCheckConstraints, 
                                 ofstream &pLog)
{
 clock_t vStartClk, vEndClk;
 time_t vStartTime, vEndTime;
 
 vStartClk = clock();
 time(&vStartTime);
  
 unsigned int cntSamples = 0, cntIgnored = 0;

 for (vector<Sample *>::iterator itSample = pSamples.begin(); 
      itSample < pSamples.end(); 
      itSample++)
 {
  vector<string> vContext;
  (*itSample)->getZMEContext(vContext, pFeatureView);
 
  vector<pair<ArgLabel, double> > vPrdLabels;          
  zmeModel.eval_all(vContext, vPrdLabels);
  // if the sample is not labeled (due to no change in the classifier which
  // had assigned the previous label), increase the number of samples ignored 
  // labeling
  if (!(*itSample)->setPrdArgLabels(vPrdLabels, pFeatureView, no, pCheckConstraints))
   cntIgnored++;
    
  cntSamples++;   

  cout << "\r" << cntSamples << " samples processed          " << flush;
  }

 cout << endl << cntSamples - cntIgnored << " samples labeled";
 cout << endl << cntIgnored << " samples ignored labeling" << endl;

 pLog << cntSamples - cntIgnored << " samples labeled" << endl;
 pLog << cntIgnored << " samples ignored labeling" << endl;

 vEndClk = clock();
 cpuUsage = ((double) (vEndClk - vStartClk)) / CLOCKS_PER_SEC;
 time(&vEndTime);
 elapsedTime = difftime(vEndTime, vStartTime);

 return cntSamples - cntIgnored;
}
