/**
 *  @ RSK
 *  @ Version 1.1        05.04.2010
 * 
 *  (refer to Sample.hpp)
 * 
 */

#include <string>
#include <fstream>
#include <iostream>

#include "Sample.hpp"
#include "Sentence.hpp"
#include "TBTree.hpp"
#include "DTree.hpp"
#include "Utils.hpp"

using namespace std;


/**
 *  Upon creation of a Sample object, it extract features and add 
 *  itself into the training file in the format of Zhang Maximum Entropy
 *  tool if an stream is assigned to pstrmZME
 *  pViewCount determines the number of views (e.g. 1 for self-training
 *  and 2 or more for co-training) for each of which the features will be
 *  extracted, and pFeatureSets contains feature set numbers for each of 
 *  those views
 */
Sample::Sample(Proposition * const &pProp, Constituent &pCnstnt, 
               unsigned short pViewCount, unsigned short pFeatureSets[],
               ArgLabel &pLabel, ofstream &pStrmZME)
{
 prop = pProp;
 cnstntToken = pCnstnt;
 goldLabel = pLabel;
 selPrdLabel.first = "";
 selPrdLabel.second = 0;
  
 pProp->addSample(this); 

 for (unsigned short i = 0; i < pViewCount; i++)
 {
  classifierNo[i] = -1;
  activeFeatureSets[i] = pFeatureSets[i] - 1;
  // Feature views are refered by numbers starting from 1, but they are
  // converted to array index (starting from 0).
  extractFeatures(i + 1);
  if (pStrmZME)
   writeToZME(i + 1, pStrmZME);
  }
  
}

Proposition *Sample::getProp()
{
 return prop;
}

string Sample::getPredForm()
{
 return prop->getPredForm();
}

string Sample::getCnstntLabel()
{
 return cnstntToken->getSyntTag();
}

Constituent Sample::getCnstntToken()
{
 return cnstntToken;
}

WordSpan Sample::getWordSpan()
{
 return cnstntToken->getWordSpan();
}

string Sample::getWordsStr()
{
 return cnstntToken->getWordsStr();
}

string Sample::getGoldArgLabel()
{
 return goldLabel;
}

/**
 *  assigning predicted labels to sample
 * 
 *  Based on the value of pCheckConstraints, it will check the global SRL
 *  constraints and resolve conflicts as a post-processing step.
 */  
bool Sample::setPrdArgLabels(vector<pair<ArgLabel, double> > &pLabels,
                             unsigned short pFeatureView, 
                             unsigned short pClassifierNo,
                             bool pCheckConstraints)
{

 if (!prdLabels[pFeatureView - 1].empty())
 {
  // if the current label has already been predicted by the same classifier,
  // do not change it and return false;
  if (pClassifierNo == classifierNo[pFeatureView - 1])
   return false;
  else
   prdLabels[pFeatureView - 1].clear();
  }
 
 // if the most probable label is not NULL, check and resolve global SRL
 // constraints
 if (pCheckConstraints)
  if (pLabels.at(0).first != "NULL")
   resolveConstraints(pLabels, pFeatureView);
  
 prdLabels[pFeatureView - 1] = pLabels;
 
 classifierNo[pFeatureView - 1] = pClassifierNo;
 return true;
}

short Sample::getClassifierNo(unsigned short pFeatureView)
{
 return classifierNo[pFeatureView - 1];
}

ArgLabel Sample::getPrdArgLabel(unsigned short pFeatureView)
{
 // if the requested feature view is 0, it means that the selected common
 // label is requested
 if (pFeatureView == 0)
  return selPrdLabel.first; 
 
 if (prdLabels[pFeatureView - 1].empty())
  return "";
 else
  return prdLabels[pFeatureView - 1].at(0).first;
}

double Sample::getPrdArgLabelProb(unsigned short pFeatureView)
{
 // if the requested feature view is 0, it means that the selected common
 // label is requested
 if (pFeatureView == 0)
  return selPrdLabel.second; 

 if (prdLabels[pFeatureView - 1].empty())
  return 0;
 else 
  return prdLabels[pFeatureView - 1].at(0).second;
}

double Sample::getAvgPrdArgLabelProb()
{
 double tmpProbSum = 0;
 
 for (unsigned short cntView = 1; cntView <= FEATURE_VIEW_COUNT; cntView++)
   tmpProbSum += getPrdArgLabelProb(cntView);
 
 return (tmpProbSum / FEATURE_VIEW_COUNT);

}

ArgLabel Sample::getPrdOrGoldArgLabel(unsigned short pFeatureView)
{
 // if the requested feature view is 0, it means that the selected common
 // label is requested
 if (pFeatureView == 0)
 {
  if (!selPrdLabel.first.empty())
   return selPrdLabel.first;
  else 
   // if selected common label is empty, the sample is gold
   return goldLabel;
  }

 if (!prdLabels[pFeatureView - 1].empty())
  return prdLabels[pFeatureView - 1].at(0).first;
 else
  return goldLabel;
}

void Sample::setSelPrdLabel(ArgLabel pLabel, double pProb)
{
 selPrdLabel.first = pLabel;
 selPrdLabel.second = pProb;
}

ArgLabel Sample::getSelPrdLabel()
{
 return selPrdLabel.first;
}

double Sample::getSelPrdLabelProb()
{
 return selPrdLabel.second;
}

void Sample::setSelectedPrdLabel(unsigned short pSelectionMethod,
                                 unsigned short pPreferedView)
{
  switch (pSelectionMethod)
  {
   case (AGREEMENT_ONLY):
    if (isPrdArgLabelsAgreed())
     setSelPrdLabel(getPrdArgLabel(1), getAvgPrdArgLabelProb());
    else
     setSelPrdLabel("NULL", 0); 
    break;
   case (AGREEMENT_CONFIDENCE):
    if (isPrdArgLabelsAgreed())
     setSelPrdLabel(getPrdArgLabel(1), getAvgPrdArgLabelProb());
    else
     setSelPrdLabel(getMCPrdLabel(), getMCPrdLabelProb()); 
    break;
   case (AGREEMENT_PREFERED_VIEW):
    if (isPrdArgLabelsAgreed())
     setSelPrdLabel(getPrdArgLabel(1), getAvgPrdArgLabelProb());
    else
     setSelPrdLabel(getPrdArgLabel(pPreferedView), getPrdArgLabelProb(pPreferedView)); 
    break;
   case (CONFIDENECE_ONLY):
    setSelPrdLabel(getMCPrdLabel(), getMCPrdLabelProb()); 
    break;
   }
}  

ArgLabel Sample::getMCPrdLabel()
{
 double tmpMaxProb = getPrdArgLabelProb(1);
 ArgLabel tmpMCLabel = getPrdArgLabel(1);
 
 for (unsigned short cntView = 2; cntView <= FEATURE_VIEW_COUNT; cntView++)
  if (tmpMaxProb < getPrdArgLabelProb(cntView))
  {
   tmpMaxProb = getPrdArgLabelProb(cntView);
   tmpMCLabel = getPrdArgLabel(cntView);
   }
 
return tmpMCLabel;
}

double Sample::getMCPrdLabelProb()
{
 double tmpMaxProb = getPrdArgLabelProb(1);
 
 for (unsigned short cntView = 2; cntView <= FEATURE_VIEW_COUNT; cntView++)
  if (tmpMaxProb < getPrdArgLabelProb(cntView))
   tmpMaxProb = getPrdArgLabelProb(cntView);
 
 return tmpMaxProb;
}


bool Sample::isPrdArgument(unsigned short pFeatureView)
{
 return (getPrdArgLabel(pFeatureView) != "NULL");
}

bool Sample::isPrdCoreArg(unsigned short pFeatureView)
{
 for (unsigned short i = 0; !CORE_ARGS[i].empty(); i++)
  if (getPrdArgLabel(pFeatureView) == CORE_ARGS[i])
   return true;
 
 return false;  
}

bool Sample::isPrdArgLabelsAgreed()
{
 ArgLabel tmpLabel = getPrdArgLabel(1);
 for (unsigned short cntView = 2; cntView <= FEATURE_VIEW_COUNT; cntView++)
  if (tmpLabel != getPrdArgLabel(cntView))
   return false;
 
 return true;
}

/**
 *  checking, finding, and resolving global SRL constraints:
 * 
 *  constraint 1: no core argument repeat for each proposition
 *  solution: between two samples with the same predicted core label,
 *            change the less probable one to NULL
 * 
 *  constraint 2: no argument overlap for any proposition
 *  solution: between two non-NULL-labeled overlapped samples, change
 *            the less probable one to NULL
 */
void Sample::resolveConstraints(vector<pair<ArgLabel, double> > &pLabels,
                                unsigned short pFeatureView)
{
 vector<Sample *> vSamples;
 prop->getSamples(vSamples);	     

 for (vector<Sample *>::iterator itSample = vSamples.begin(); 
      itSample < vSamples.end(); 
      itSample++)
  // if the current is not this sample, its most probable label is not
  // NULL, and it is already labeled, process the constraints
  if ((*itSample != this) && 
      ((*itSample)->getPrdArgLabel(pFeatureView) != "NULL") &&
      ((*itSample)->getPrdArgLabel(pFeatureView) != ""))
  {
   // applying constraint 1:
   if ((*itSample)->isPrdCoreArg(pFeatureView) &&
       (pLabels[0].first == (*itSample)->getPrdArgLabel(pFeatureView)))
   {
    if (pLabels[0].second > (*itSample)->getPrdArgLabelProb(pFeatureView))
    {
     (*itSample)->prdLabels[pFeatureView - 1][0].first = "NULL";
     // since the label of current sample became NULL, no need to check 
     // for other constraints
     continue;
     }
    else if (pLabels[0].second <= (*itSample)->getPrdArgLabelProb(pFeatureView))
    {
     pLabels[0].first = "NULL";
     // since the label of this sample became NULL, no need to continue
     // with other samples or for other constraints
     return;
     }
    // what does happen to its probability?? for now nothing!!!
    }

   // applying constraint 2:
   if (isOverlapped(*(*itSample)))
   {
    if (pLabels[0].second > (*itSample)->getPrdArgLabelProb(pFeatureView))
    {
     (*itSample)->prdLabels[pFeatureView - 1][0].first = "NULL";
     // since the label of current sample became NULL, no need to check 
     // for other constraints
     continue;
     }
    else if (pLabels[0].second <= (*itSample)->getPrdArgLabelProb(pFeatureView))
    {
     pLabels[0].first = "NULL";
     // since the label of this sample became NULL, no need to continue
     // with other samples or for other constraints
     return;
     }
    // what does happen to its probability?? for now nothing!!!
    }
   } 
}

bool Sample::isOverlapped(Sample &pSample)
{
 switch (getSpansRelation(getWordSpan(), pSample.getWordSpan()))
 {
  case EMBEDDED:
  case INCLUDE:
  case ALIGNED:
  case OVERLAPED:
   return true;
  default:
   return false;
  }
}


/**
 *  extract features according to the feature view passed as parameter
 */
void Sample::extractFeatures(unsigned short pFeatureView)
{
 // looping inside the feature set to extract values of all features that
 // are members of that feature set.
 for (unsigned short cntFeatures = 0; 
      FEATURES[activeFeatureSets[pFeatureView - 1]][cntFeatures] != E_O_F;
      cntFeatures++)
 {
  switch (FEATURES[activeFeatureSets[pFeatureView - 1]][cntFeatures])
  {    
   case (TRIAL_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("TRIAL", extractTrialFeature()));
    break;

   case (PT_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PT", extractPhraseType()));
    break;

   case (PL_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PL", extractPredLemma()));
    break;

   case (PP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PP", extractPredPOS()));
    break;

   case (PVP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PVP", extractPredVoice() + extractPosition()));
    break;

   case (PATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PATH", extractPath()));
    break;
  
   case (CVNCP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("CVNCP", extractCVNCountInPath(pFeatureView)));
    break;
  
   case (PTHLEN_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PTHLEN", extractPathLength(pFeatureView)));
    break;
  
   case (HW_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("HW", extractHeadWord()));
    break;
  
   case (CW_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("CW", extractContentWord()));
    break;

   case (HWL_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("HWL", extractHeadWordLemma()));
    break;
  
   case (CWL_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("CWL", extractContentWordLemma()));
    break;

   case (GC_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("GC", extractGovCat()));
    break;
  
   case (PS_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PS", extractPredSubcat()));
    break;
  
   case (CS_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("CS", extractCnstntSubcat()));
    break;
  
   case (CPD_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("CPD", extractCPDistance()));
    break;
  
   case (CPI_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("CPI", extractCPIdentifier()));
    break;
 
   case (HLC_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("HLC", extractHWLocation()));
    break;

   case (HWP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("HWP", extractHeadWordPOS()));
    break;

   case (CWP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("CWP", extractContentWordPOS()));
    break;

   case (POSITION_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("POSITION", extractPosition()));
    break;

   case (PV_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PV", extractPredVoice())); 
    break;

   case (PF_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PF", extractPredForm())); 
    break;

   case (AWF_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWF", extractAWForm())); 
    break;

   case (AWR_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWR", extractAWRel())); 
    break;

   case (PR_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PR", extractPredRel())); 
    break;

   case (AWHF_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWH", extractAWHeadForm())); 
    break;

   case (AWHL_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWHL", extractAWHeadLemma())); 
    break;

   case (AWHP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWHP", extractAWHeadPOS())); 
    break;
    
   case (PCRP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PCRP", extractPredChildrenRP())); 
    break;
    
   case (AWCRP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWCRP", extractAWChildrenRP())); 
    break;

   case (PCPP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PCPP", extractPredChildrenPP())); 
    break;
    
   case (AWCPP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWCPP", extractAWChildrenPP())); 
    break;

   case (RPATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("RPATH", extractRelPath())); 
    break;

   case (ARPATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("ARPATH", extractARelPath())); 
    break;

   case (PPATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PPATH", extractPOSPath())); 
    break;

   case (APPATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("APPATH", extractAPOSPath())); 
    break;

   case (FAMREL_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("FAMREL", extractFamilyRelation())); 
    break;

   case (PSRP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PSRP", extractPredSiblingsRP())); 
    break;
    
   case (PSPP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PSPP", extractPredSiblingsPP())); 
    break;
    
   case (PPSRP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PPSRP", extractPredParentSiblingsRP())); 
    break;
    
   case (PPSPP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("PPSPP", extractPredParentSiblingsPP())); 
    break;
    
   case (AWSRP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWSRP", extractAWSiblingsRP())); 
    break;
    
   case (AWSPP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWSPP", extractAWSiblingsPP())); 
    break;
    
   case (AWPSRP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWPSRP", extractAWParentSiblingsRP())); 
    break;
    
   case (AWPSPP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWPSPP", extractAWParentSiblingsPP())); 
    break;
    
   case (AWLRF_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWLRF", extractAWLRChildrenForms())); 
    break;
    
   case (AWLSF_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWLSF", extractAWLSiblingForm())); 
    break;
    
   case (LCAPOS_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("LCAPOS", extractLCommonAncestorPOS())); 
    break;
    
   case (AWLCRPATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWLCRPATH", extractAWRelPathToLCA())); 
    break;
    
   case (AWLCPPATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWLCPPATH", extractAWPOSPathToLCA())); 
    break;
    
   case (DPTHLEN_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("DPTHLEN", extractDepPathLength(pFeatureView))); 
    break;
    
   case (LPATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("LPATH", extractLemmaPath())); 
    break;
    
   case (AWLCLPATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("AWLCLPATH", extractAWLemmaPathToLCA())); 
    break;
    
   case (LCRRPATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("LCRRPATH", extractLSARelPathToRoot())); 
    break;
    
   case (LCRPPATH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("LCRPPATH", extractLSAPOSPathToRoot())); 
    break;
    
   case (DDPTHLEN_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("DDPTHLEN", extractDividedDepPathLength(pFeatureView))); 
    break;
    
   case (ISCAP_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("ISCAP", extractIsCapital())); 
    break;
    
   case (ISWH_F):
    featureVectors[pFeatureView - 1].push_back(pair<string, string> ("ISWH", extractIsWH())); 
    break;
    
   } 
  }
}

/**
 *  This is to try new hypothesized features without need to repeatedly 
 *  define and clear new functions for them. The approved feature can then
 *  be moved to its permanent definition
 */
string Sample::extractTrialFeature()
{
 return "";
}

/**
 *  we currently use the phrase type of the constituent regardless of it
 *  being terminal in the tree (PT is POS tag) or a non-terminal (PT is
 *  syntactic category)
 */
string Sample::extractPhraseType()
{
 return cnstntToken->getSyntTag();
}

/**
 *  This extract the position of constituent in relation to the terminal
 *  constituent of predicate. It guarantees that only BEFORE or AFTER
 *  occures, but in case of not using pruning, overlapping may be the case
 *  and the value for the feature be not either of them!
 */
string Sample::extractPosition()
{
 if (getSpansRelation(cnstntToken->getWordSpan(),
                      prop->getPredCnstntSpan()) == BEFORE)
  return "b";
 else if (getSpansRelation(cnstntToken->getWordSpan(),
                           prop->getPredCnstntSpan()) == AFTER)
  return "a";
 else
  return "";
}

string Sample::extractPredLemma()
{
 return lowerCase(prop->getPredLemma());
}

string Sample::extractPredPOS()
{
 return prop->getPredPOS();
}

string Sample::extractPredVoice()
{
 if (prop->getPredVoice())
  return "a";
 else
  return "p";
}

string Sample::extractPath()
{
 return prop->getPredCnstnt()->getPathTo(cnstntToken);
}

/**
 *  To keep efficient, this first looks at path feature status and if it
 *  is disabled or not extracted yet, it extracts the path string. So, 
 *  the most efficient way is to first extract path then its length (in 
 *  extractFeatures() function) if the path itself is enabled.
 */
string Sample::extractPathLength(unsigned short pFeatureView)
{
 string vPath;
 
 unsigned short tmpIdx = indexIn(PATH_F, 
                                 FEATURES[activeFeatureSets[pFeatureView - 1]], 
                                 sizeof(FEATURES[activeFeatureSets[pFeatureView - 1]]) / 
                                 sizeof(FEATURES[activeFeatureSets[pFeatureView - 1]][0]));
 if ((tmpIdx != -1) && !featureVectors[pFeatureView - 1][tmpIdx].second.empty())
  vPath = featureVectors[pFeatureView - 1][tmpIdx].second; 
 else
  vPath = extractPath();
 
 vector<string> tmpvcPath;
 tokenize(vPath, tmpvcPath, "><");
 return intToStr(tmpvcPath.size()); 
}

string Sample::extractCVNCountInPath(unsigned short pFeatureView)
{
 string vPath;
 
 unsigned short tmpIdx = indexIn(PATH_F, 
                                 FEATURES[activeFeatureSets[pFeatureView - 1]], 
                                 sizeof(FEATURES[activeFeatureSets[pFeatureView - 1]]) / 
                                 sizeof(FEATURES[activeFeatureSets[pFeatureView - 1]][0]));
 if ((tmpIdx != -1) && !featureVectors[pFeatureView - 1][tmpIdx].second.empty())
  vPath = featureVectors[pFeatureView - 1][tmpIdx].second; 
 else
  vPath = extractPath();
 
 vector<string> tmpvcPath;
 tokenize(vPath, tmpvcPath, "><");
 
 unsigned short vCount = 0;
 for (unsigned short cntPath = 0; cntPath < tmpvcPath.size(); cntPath++)
  if ((tmpvcPath[cntPath][0] == 'S') ||
      (tmpvcPath[cntPath] == "VP") ||
      (tmpvcPath[cntPath] == "NP"))
   vCount++;
 
 return intToStr(vCount);
}

string Sample::extractHeadWord()
{
 return lowerCase(cnstntToken->getHeadWordForm());
}	

string Sample::extractContentWord()
{
 return lowerCase(cnstntToken->getContentWordForm());
}

string Sample::extractHeadWordLemma()
{
 return lowerCase(cnstntToken->getHeadWordLemma());
}	

string Sample::extractContentWordLemma()
{
 return lowerCase(cnstntToken->getContentWordLemma());
}

string Sample::extractHeadWordPOS()
{
 return cnstntToken->getHeadWordCnstnt()->getSyntTag();
}	

string Sample::extractContentWordPOS()
{
 return cnstntToken->getContentWord()->getSyntTag();
}	

string Sample::extractPredSubcat()
{
 return prop->getPredCnstnt()->getSubcat();
}	

string Sample::extractCnstntSubcat()
{
 return cnstntToken->getSubcat();
}

/**
 *  This feature is originally used for only NPs, but we found it here 
 *  useful when extracted for all phrase types.
 */
string Sample::extractGovCat()
{
 if (cnstntToken->getSyntTag() == "NP")
  return cnstntToken->getGovCat();
 else
  return ""; 
}	

/**
 *  Distance between constituent and predicate so that it also accommodates
 *  the position of constituent in relation to predicate by the sign of 
 *  distance
 */
string Sample::extractCPDistance()
{
 short int vDistance = 0;
 
 vDistance = prop->getPredCnstntSpan().start - cnstntToken->getWordSpan().start;
 
 if (vDistance < 0) 
  return intToStr(vDistance + 1);
 else
  return intToStr(vDistance - cnstntToken->getWordsCount()); 
}

/**
 *  An identifier of compound verb predicate:
 *  -CC: if it is continous compund verb (verb with particle; e.g. "get up"
 *  -DC: if it is discontinous compound verb (e.g. "take it off")
 *  -S: simple
 */
string Sample::extractCPIdentifier()
{
 if ((prop->getPredSpan().end - prop->getPredSpan().start) > 0)
  return "CC";
 else if (prop->getPredCCnstnt() != NULL)
  return "DC";
 else 
  return "S"; 
}

/**
 *  To codify the location of head word in constituent: a string formed
 *  by concatenating the distance of HW from begining of constituent, a 
 *  "/" as divider, and its distance from the end of constituent
 */
string Sample::extractHWLocation()
{
 return intToStr(cnstntToken->getHeadWordCnstnt()->getWordSpan().start -
                 cnstntToken->getWordSpan().start) + "/" + 
        intToStr(cnstntToken->getWordSpan().end - 
                 cnstntToken->getHeadWordCnstnt()->getWordSpan().end);
}

string Sample::extractPredForm()
{
 return lowerCase(prop->getPredForm());
}

string Sample::extractAWForm()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return lowerCase(vDepArgWord->getWordForm());
}	

string Sample::extractAWRel()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getDepRel();
}	

string Sample::extractPredRel()
{
 return prop->getPredWord()->getDTNode()->getDepRel();
}	

string Sample::extractAWHeadForm()
{
 DTNode vHead = cnstntToken->getDepArgWord()->getDTNode()->getParent();
 if (vHead->isRoot())
  return "";
 else
  return lowerCase(vHead->getWordForm());
}

string Sample::extractAWHeadLemma()
{
 DTNode vHead = cnstntToken->getDepArgWord()->getDTNode()->getParent();	
 if (vHead->isRoot())
  return "";
 else
  return lowerCase(vHead->getWordLemma());
}

string Sample::extractAWHeadPOS()
{
 DTNode vHead = cnstntToken->getDepArgWord()->getDTNode()->getParent();	
 if (vHead->isRoot())
  return "";
 else
  return vHead->getWordPOS();
}

string Sample::extractPredChildrenRP()
{
 return prop->getPredWord()->getDTNode()->getChldRelPattern();
}

string Sample::extractAWChildrenRP()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getChldRelPattern();
}

string Sample::extractPredChildrenPP()
{
 return prop->getPredWord()->getDTNode()->getChldPOSPattern();
}

string Sample::extractAWChildrenPP()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getChldPOSPattern();
}

string Sample::extractRelPath()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getRelPathTo(prop->getPredWord()->getDTNode());
}

string Sample::extractARelPath()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getARelPathTo(prop->getPredWord()->getDTNode());
}

string Sample::extractPOSPath()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getPOSPathTo(prop->getPredWord()->getDTNode());
}

string Sample::extractAPOSPath()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getAPOSPathTo(prop->getPredWord()->getDTNode());
}

string Sample::extractFamilyRelation()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 unsigned short vFamRel = vDepArgWord->getDTNode()->getFamRelTo(prop->getPredWord()->getDTNode());
 switch (vFamRel)
 {
  case CHILD:
  case PARENT:
  case GRANDCHILD:
  case GRANDPARENT:
  case SIBLING:
  case DESCENDANT:
  case ANCESTOR:
   return intToStr(vFamRel);
   break;
  default:
   return "0";
  }  
}

string Sample::extractPredSiblingsRP()
{
 return prop->getPredWord()->getDTNode()->getSibRelPattern();
}

string Sample::extractPredSiblingsPP()
{
 return prop->getPredWord()->getDTNode()->getSibPOSPattern();
}

string Sample::extractPredParentSiblingsRP()
{
 return prop->getPredWord()->getDTNode()->getParent()->getSibRelPattern();
}

string Sample::extractPredParentSiblingsPP()
{
 return prop->getPredWord()->getDTNode()->getParent()->getSibPOSPattern();
}

string Sample::extractAWSiblingsRP()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getSibRelPattern();
}

string Sample::extractAWSiblingsPP()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getSibPOSPattern();
}

string Sample::extractAWParentSiblingsRP()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getParent()->getSibRelPattern();
}

string Sample::extractAWParentSiblingsPP()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getParent()->getSibPOSPattern();
}

string Sample::extractAWLRChildrenForms()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 DTNode vNode = vDepArgWord->getDTNode();
  
 vector<DTNode> vChildren;	
 vNode->getChildren(vChildren);
 
 unsigned short vChildCount = vChildren.size();
 
 if (vChildCount == 1)
  return vChildren[0]->getWordForm();
 else if (vChildCount > 1)
  return vChildren[0]->getWordForm() + "+" + 
         vChildren[vChildCount - 1]->getWordForm();
 else
  return "";
}

string Sample::extractAWLSiblingForm()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 DTNode vLeftSibling = vDepArgWord->getDTNode()->getLeftSibling();
  
 if (vLeftSibling)
  return vLeftSibling->getWordForm();
 else
  return "";
}

string Sample::extractLCommonAncestorPOS()
{
 DTNode tmpNode = cnstntToken->getDepArgWord()->getDTNode();
 tmpNode = tmpNode->getLCAncestorWith(prop->getPredWord()->getDTNode());
 
 if ((!tmpNode) || (tmpNode->isRoot()))
  return "";
 else
  return tmpNode->getWordPOS();
}

string Sample::extractAWRelPathToLCA()
{	
 DTNode vArgNode = cnstntToken->getDepArgWord()->getDTNode();
 DTNode vPredNode = prop->getPredWord()->getDTNode();
 
 if (vArgNode->isDescendantOf(vPredNode) || vPredNode->isDescendantOf(vArgNode))
  return "";
 
 DTNode vLCANode = vArgNode->getLCAncestorWith(vPredNode);
 
 if (vArgNode == vLCANode)
  return "";
 else
  return vArgNode->getARelPathTo(vLCANode);
}

string Sample::extractAWPOSPathToLCA()
{	
 DTNode vArgNode = cnstntToken->getDepArgWord()->getDTNode();
 DTNode vPredNode = prop->getPredWord()->getDTNode();
 
 if (vArgNode->isDescendantOf(vPredNode) || vPredNode->isDescendantOf(vArgNode))
  return "";
 
 DTNode vLCANode = vArgNode->getLCAncestorWith(vPredNode);
 
 if (vArgNode == vLCANode)
  return "";
 else 
  return vArgNode->getPOSPathTo(vLCANode);
}

string Sample::extractAWLemmaPathToLCA()
{	
 DTNode vArgNode = cnstntToken->getDepArgWord()->getDTNode();
 DTNode vPredNode = prop->getPredWord()->getDTNode();
 
 if (vArgNode->isDescendantOf(vPredNode) || vPredNode->isDescendantOf(vArgNode))
  return "";
 
 DTNode vLCANode = vArgNode->getLCAncestorWith(vPredNode);
 
 if (vArgNode == vLCANode)
  return "";
 else 
  return vArgNode->getLemmaPathTo(vLCANode);
}

string Sample::extractDepPathLength(unsigned short pFeatureView)
{
 string vPath;
 
 unsigned short tmpIdx = indexIn(ARPATH_F, 
                                 FEATURES[activeFeatureSets[pFeatureView - 1]], 
                                 sizeof(FEATURES[activeFeatureSets[pFeatureView - 1]]) / 
                                 sizeof(FEATURES[activeFeatureSets[pFeatureView - 1]][0]));
 if ((tmpIdx != -1) && (!featureVectors[pFeatureView - 1][tmpIdx].second.empty()))
  vPath = featureVectors[pFeatureView - 1][tmpIdx].second; 
 else
  vPath = extractARelPath();
 
 vector<string> tmpvcPath;
 tokenize(vPath, tmpvcPath, "/><\\");
 return intToStr(tmpvcPath.size());
}

string Sample::extractLemmaPath()
{
 WordPtr vDepArgWord = cnstntToken->getDepArgWord();
 return vDepArgWord->getDTNode()->getLemmaPathTo(prop->getPredWord()->getDTNode());
}

string Sample::extractLSARelPathToRoot()
{
 DTNode vArgNode = cnstntToken->getDepArgWord()->getDTNode();
 DTNode vPredNode = prop->getPredWord()->getDTNode();
 
 DTNode vLCANode = vArgNode->getLCAncestorWith(vPredNode);
 
 if (vLCANode->isRoot() || vLCANode->getParent()->isRoot())
  return "";
 else 
  return vLCANode->getRelPathTo(vLCANode->getRoot());
}

string Sample::extractLSAPOSPathToRoot()
{
 DTNode vArgNode = cnstntToken->getDepArgWord()->getDTNode();
 DTNode vPredNode = prop->getPredWord()->getDTNode();
 
 DTNode vLCANode = vArgNode->getLCAncestorWith(vPredNode);
 
 if (vLCANode->isRoot() || vLCANode->getParent()->isRoot())
  return "";
 else 
  return vLCANode->getPOSPathTo(vLCANode->getRoot());
}

string Sample::extractDividedDepPathLength(unsigned short pFeatureView)
{
 string vPath;
 unsigned short vPath1Length = 0;
 unsigned short vPath2Length = 0;
 vector<string> tmpvcPath;
 unsigned short tmpIdx;
 
 // extracting the length of whole path
 tmpIdx = indexIn(ARPATH_F, 
                  FEATURES[activeFeatureSets[pFeatureView - 1]], 
                  sizeof(FEATURES[activeFeatureSets[pFeatureView - 1]]) / 
                  sizeof(FEATURES[activeFeatureSets[pFeatureView - 1]][0]));
         
 if ((tmpIdx != -1) && (!featureVectors[pFeatureView - 1][tmpIdx].second.empty()))
  vPath = featureVectors[pFeatureView - 1][tmpIdx].second; 
 else
  vPath = extractARelPath();
 
 tokenize(vPath, tmpvcPath, "/><\\");
 vPath1Length = tmpvcPath.size();

 /**
  * extracting the length of path to least common ancestor
  * Note that, because in some situations AWLCRPATH feature is not extracted
  * due to the redundancy with RPATH, the length is extracted independent 
  * of AWLCRPATH feature
  */ 

 DTNode vArgNode = cnstntToken->getDepArgWord()->getDTNode();
 DTNode vPredNode = prop->getPredWord()->getDTNode();
 
 vPath2Length = vArgNode->getPathLengthToLCA(vPredNode);

 return intToStr(vPath2Length) + "/" + intToStr(vPath1Length - vPath2Length); 
}

string Sample::extractIsCapital()
{
 string vWordForm = cnstntToken->getDepArgWord()->getWordForm();

 if (isCapital(vWordForm[0]))
  return "1";
 else
  return "0";
}

string Sample::extractIsWH()
{
 string vWordForm = cnstntToken->getDepArgWord()->getWordForm();
 for (int i = 0; !WH[i].empty(); i++)
  if (lowerCase(vWordForm) == WH[i])
   return "1";

 return "0";
}

void Sample::getZMEContext(vector<string> &pZMEFeatureVector, unsigned short pFeatureView)
{
 for (unsigned short cntFeature = 0; 
      cntFeature < featureVectors[pFeatureView - 1].size();
      cntFeature++)
  // if the value of feature is empty string, it will not be added to
  // feature vector (context)
  if (featureVectors[pFeatureView - 1][cntFeature].second != "")
  {
   pZMEFeatureVector.push_back(featureVectors[pFeatureView - 1][cntFeature].first + '=' +   
                               featureVectors[pFeatureView - 1][cntFeature].second);
   }
}

string Sample::getZMEContextStr(unsigned short pFeatureView)
{
 string vZMEContextStr;

 vector<string> vContext;
 getZMEContext(vContext, pFeatureView);
 
 for (size_t cntFeature = 0; 
      cntFeature < vContext.size();
      cntFeature++)
  vZMEContextStr += vContext[cntFeature] + " ";

 return trim(vZMEContextStr);
}

void Sample::writeToZME(unsigned short pFeatureView, ofstream &pStrmZME)
{
 if(pStrmZME.is_open())
 {
  pStrmZME.seekp(pStrmZME.tellp(),fstream::beg);

  pStrmZME << goldLabel << " " 
           << getZMEContextStr(pFeatureView) << endl;

  }
}

void Sample::logPrdLabeling(unsigned short pFeatureView, ofstream &pStrm)
{
 if(pStrm.is_open())
 {
  pStrm.seekp(pStrm.tellp(),fstream::beg);
  
  prop->getSentence()->display(pStrm);
        
  pStrm << getPrdArgLabel(pFeatureView) << "; "
        << getWordsStr() << "; ["
        << getPredForm() << "]; "
        << getPrdArgLabelProb(pFeatureView)
        << endl;
  }
}

void Sample::logPrdLabelings(unsigned short pViewCount, ofstream &pStrm)
{
 if(pStrm.is_open())
 {
  pStrm.seekp(pStrm.tellp(),fstream::beg);
  
  prop->getSentence()->display(pStrm);
        
  for (unsigned short cntView = 1; cntView <= pViewCount; cntView++)
   pStrm << getPrdArgLabel(cntView) << ";";

  // selected common label
  pStrm << "[" << getPrdArgLabel(0) << "];";
        
  pStrm << " " << getWordsStr() << "; ["
        << getPredForm() << "]; ";

  for (unsigned short cntView = 1; cntView <= pViewCount; cntView++)
   pStrm << getPrdArgLabelProb(cntView) << ";" ;
   
  pStrm << endl;
  }
}
