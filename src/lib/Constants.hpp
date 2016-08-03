/**
 *  @ RSK
 *  @ Version 0.9        16.03.2010
 * 
 *  All required constants and global variables
 *   
 */

#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#define DEFAULT_CONLL_LABELED_PATH "../../corpus/labeled"
#define DEFAULT_CONLL_TRAIN_WORDS_NAME "train.words.random"
#define DEFAULT_CONLL_TRAIN_SYNT_CHA_NAME "train.synt.cha.random"
#define DEFAULT_CONLL_TRAIN_SYNT_DEP_NAME "train.synt.dep.random"
#define DEFAULT_CONLL_TRAIN_PROPS_NAME "train.props.random"

#define DEFAULT_CONLL_UNLABELED_PATH "../../corpus/unlabeled"
#define DEFAULT_CONLL_UNLABELED_DATA_NAME "unlabeled.wsj.cha.random.reverse"
#define DEFAULT_CONLL_UNLABELED_SYNT_DEP_NAME "unlabeled.wsj.synt.dep.random.reverse"

#define DEFAULT_CONLL_TEST_DATA_PATH "../../corpus/test"
#define DEFAULT_CONLL_TEST_WSJ_WORDS_NAME "test.wsj.words"
#define DEFAULT_CONLL_TEST_WSJ_SYNT_CHA_NAME "test.wsj.synt.cha"
#define DEFAULT_CONLL_TEST_WSJ_SYNT_DEP_NAME "test.wsj.synt.dep"
#define DEFAULT_CONLL_TEST_WSJ_PROPS_NAME "test.wsj.props"
#define DEFAULT_CONLL_TEST_BROWN_WORDS_NAME "test.brown.words"
#define DEFAULT_CONLL_TEST_BROWN_SYNT_CHA_NAME "test.brown.synt.cha"
#define DEFAULT_CONLL_TEST_BROWN_SYNT_DEP_NAME "test.brown.synt.dep"
#define DEFAULT_CONLL_TEST_BROWN_PROPS_NAME "test.brown.props"
#define DEFAULT_CONLL_DEV_WSJ_WORDS_NAME "devel.24.words"
#define DEFAULT_CONLL_DEV_WSJ_SYNT_CHA_NAME "devel.24.synt.cha"
#define DEFAULT_CONLL_DEV_WSJ_SYNT_DEP_NAME "devel.24.synt.dep"
#define DEFAULT_CONLL_DEV_WSJ_PROPS_NAME "devel.24.props"

#define DEFAULT_CONLL_TEST_OUTPUT_PATH "../../output"

#define MAX_TRAIN_POSITIVE 0    // 0 = all 
#define MAX_TRAIN_NEGATIVE 0    // 0 = all 

#define DEFAULT_LOG_PATH "../../log"
#define LOG_LABELED_SAMPLE_GENERATION_ANALYSIS_FILE "../../log/LabeledSGAnalysis.log"
#define LOG_UNLABELED_SAMPLE_GENERATION_ANALYSIS_FILE "../../log/UnlabeledSGAnalysis.log"
#define LOG_NEGATIVE_SAMPLES_FILE "../../log/Negatives.log"
#define LOG_POSITIVE_SAMPLES_FILE "../../log/Positives.log"
#define LOG_TEST_SAMPLE_GENERATION_ANALYSIS_FILE "../../log/TestSGAnalysis.log"
#define LOG_TEST_SAMPLES_FILE "../../log/TestSamples.log"
#define LOG_POOL_SELECTION_FILE_NAME "../../log/PoolSelection.log"
#define LOG_SELFTRAINING_OUTPUT_FILE_PREFIX "../../log/SelfTraining"
#define LOG_COTRAINING_OUTPUT_FILE_PREFIX "../../log/CoTraining"
#define LOG_LEARNING_CURVE_OUTPUT_FILE_PREFIX "../../log/LCurve"

#define ZME_TRAINING_SAMPLES_FILE "../../learning/ZMETrain"
#define ZME_TRAINING_MODEL_FILE "../../learning/ZMEModel"
#define ZME_TEST_SAMPLES_FILE "../../learning/ZMETest"

// the minumum number of sentences that should be selected by selection
// criteria to consider there is room for further improvment
#define DEFAULT_SELF_TRAINING_CONVERGENCE_SENTENCE_COUNT 5
#define DEFAULT_CO_TRAINING_CONVERGENCE_SENTENCE_COUNT 5
// the minumum number of samples that should be selected by selection
// criteria to consider there is room for further improvment
#define DEFAULT_SELF_TRAINING_CONVERGENCE_SAMPLE_COUNT 200
#define DEFAULT_CO_TRAINING_CONVERGENCE_SAMPLE_COUNT 200

// followings define the relation of a span of elements (e.g. words)
// to another span in a sequence (e.g. sentence)
#define BEFORE 0
#define AFTER 1
#define EMBEDDED 2
#define INCLUDE 3
#define ALIGNED 4
#define OVERLAPED 5

// Family relationships
#define CHILD 1
#define PARENT 2
#define GRANDCHILD 3
#define GRANDPARENT 4
#define SIBLING 5
#define COUSIN 6
#define DESCENDANT 7
#define ANCESTOR 8

// Common label selection strategy with multi-view classification
#define AGREEMENT_ONLY 1              // only agreed-upon label is selected 
                                      // (non-agreed is left empty; the 
                                      //  predication of each view is used
                                      //  by itself)
#define AGREEMENT_CONFIDENCE 2        // if no agreed-upon label exist, the
                                      // most confident label is selected
#define AGREEMENT_PREFERED_VIEW 3     // if no agreed-upon label exist, the
                                      // prediction of one of the views,
                                      // which is prefered, is selected
#define CONFIDENECE_ONLY 4            // only most confident prediction
                                      // is selected

// Co-training views (to be more readable inside the code)
// Other views can be defined based on use
#define COMMON_VIEW 0
#define VIEW_1 1
#define VIEW_2 2


/**
 *  followings will be used to point to the feature vector elements without 
 *  a need to know the index of vector element storing values for a 
 *  specific feature
 *  
 *  Value -1 for a feature type indicates that it will not be used, so 
 *  the feature extraction function will not extract it
 * 
 *  Any change to feature selection can be encapsuleted here
 * 
 *  Note:
 *  - set the USED_FEATURE_TYPE_COUNT to the number of enabled features
 *  (value != -1) to tell the featureVector array size to its definition
 *  in Sample class 
 * 
 *  - set the value of each feature starting from 0 on, so that it accords
 *  to array indexing
 * 
 *  - There is another fucntion in Utils.hpp (getFeatureName()) to convert
 *  the feature type numerical constant to string. It can convert the type
 *  to a string represenating the name or simply convert the feature no
 *  to string type. Currently, the latter is used, so there is no need 
 *  to edit that function when changing the feature set.
 */

// constituent-based features
#define PT_F 1                         // Phrase Type
#define PATH_F 2                       // Path
#define CW_F 3                         // Content Word
#define CWL_F 4                        // Content Word Lemma
#define CWP_F 5                        // Content Word POS
#define GC_F 6                         // Governing Category
#define PS_F 7                         // Predicate Subcategorization
#define CS_F 8                         // Constituent Subcategorization
#define CVNCP_F 9                      // Count of Clauses(S*)+NP+VP in Path
#define CPD_F 10			           // Distance between Constituent and Predicate
#define HLC_F 11			           // Head Word's Location in Constituent
#define PTHLEN_F 12                    // Path Length
#define POSITION_F 23                  // Position
#define PV_F 14                        // Predicate Voice
#define PF_F 15                        // Predicate surface form

// dependency-based features
#define AWF_F 51                       // Argument word form
#define AWR_F 52                       // Argument word relation with its head (parent)
#define PR_F 53                        // Predicate relation with its head (parent)
#define AWHF_F 54                      // Lexical form of head of argument word
#define AWHL_F 55                      // Lemma of head of argument word
#define AWHP_F 56                      // POS tag of head of argument word
#define PCRP_F 57                      // Relation pattern of predicates children
#define AWCRP_F 58                     // Relation pattern of argument word children
#define PCPP_F 59                      // POS pattern of predicates children
#define AWCPP_F 60                     // POS pattern of argument word children
#define RPATH_F 61                     // Relation path from the argument word to the predicate 
#define ARPATH_F 62                    // Relation path from the argument word to the predicate augmented with left/right direction
#define PPATH_F 63                     // POS path from the argument word to the predicate
#define APPATH_F 64                    // POS path from the argument word to the predicate augmented with left/right direction
#define FAMREL_F 65                    // Family relationship between argument word and predicate
#define PSRP_F 66                      // Relation pattern of predicate's siblings
#define PSPP_F 67                      // POS pattern of predicate's siblings
#define PPSRP_F 68                     // Relation pattern of predicate's parent's siblings
#define PPSPP_F 69                     // POS pattern of predicate's parent's siblings
#define AWSRP_F 70                     // Relation pattern of argument word's siblings
#define AWSPP_F 71                     // POS pattern of argument word's siblings
#define AWPSRP_F 72                    // Relation pattern of argument word's parent's siblings
#define AWPSPP_F 73                    // POS pattern of argument word's parent's siblings
#define AWLRF_F 74                     // Word forms of Leftmost+Rightmost dependents of argument word
#define AWLSF_F 75                     // Word forms of left sibling of argument word
#define LCAPOS_F 76                    // POS tag of least common ancestor of argument word and predicate
#define AWLCRPATH_F 77                 // Relation path from the argument word to least common ancestor with left/right direction
#define AWLCPPATH_F 78                 // POS path from the argument word to least common ancestor
#define DPTHLEN_F 79                   // Dependency path length from the argument word to the predicate 
#define LPATH_F 80                     // Lemma path from the argument word to the predicate
#define AWLCLPATH_F 81                 // Lemma path from the argument word to least common ancestor
#define LCRRPATH_F 82                  // Relation path from least common ancestor to root
#define LCRPPATH_F 83                  // POS path from least common ancestor to root
#define DDPTHLEN_F 84                  // Dependency path length from the argument word to the predicate divided into up path and down path length
#define ISCAP_F 85                     // Whether he argument word starts with a capital letter or not
#define ISWH_F 86                      // Whether the argument word is WH word or not

// general features
#define PL_F 101                       // Predicate lemma
#define PP_F 102                       // Predicate POS
#define PVP_F 103                      // Predicate voice + Position
#define HW_F 104                       // Head Word
#define HWL_F 105                      // Head Word Lemma
#define HWP_F 106                      // Head Word POS
#define CPI_F 107			           // Compound Predicate Identifier

#define TRIAL_F 200		               // to conveniently try a new feature
#define E_O_F 0                        // end of features signal in the feature constant array (below)

//  set 1 (CG) : complete constituent-based feature set (16 features: constituent-based + general features)
//  set 2 (C)  : pure constituent-based feature set (10 features)
//  set 3 (D)  : pure dependency-based feature set (17 features)
//  set 4 (DG) : complete dependency-based feature set (23 features: dependency-based + general features)
//  set 5 (CDG): competelte feature set (33 features: constituent-based + dependency-based + general features)
//  set 6 (CGB): balanced constituent-based feature set (12 features: constituent-based + PP_F + HWP_F)
//  set 7 (DGB): balanced dependency-based feature set (21 features: dependency-based + PL_F + PVP_F + HWL_F + CPI_F) 
//  set 8 (DGUB): unbalancing dependency-based feature set (20 features: dependency-based + PL_F + HWL_F + CPI_F) 
const unsigned short FEATURES [8][34] = {{PT_F, PATH_F, CWL_F, CWP_F, GC_F, PS_F, CS_F, CVNCP_F, CPD_F, HLC_F, PL_F, PP_F, PVP_F, HWL_F, HWP_F, CPI_F, E_O_F},
                                         {PT_F, PATH_F, CWL_F, CWP_F, GC_F, PS_F, CS_F, CVNCP_F, CPD_F, HLC_F, E_O_F},
                                         {AWR_F, PR_F, AWHL_F, AWHP_F, PCRP_F, AWCRP_F, PCPP_F, AWCPP_F, ARPATH_F, APPATH_F, FAMREL_F, LCAPOS_F, AWLCPPATH_F, DPTHLEN_F, LPATH_F, ISCAP_F, ISWH_F, E_O_F},
                                         {AWR_F, PR_F, AWHL_F, AWHP_F, PCRP_F, AWCRP_F, PCPP_F, AWCPP_F, ARPATH_F, APPATH_F, FAMREL_F, LCAPOS_F, AWLCPPATH_F, DPTHLEN_F, LPATH_F, ISCAP_F, ISWH_F, PL_F, PP_F, PVP_F, HWL_F, HWP_F, CPI_F, E_O_F},
                                         {PT_F, PATH_F, CWL_F, CWP_F, GC_F, PS_F, CS_F, CVNCP_F, CPD_F, HLC_F, AWR_F, PR_F, AWHL_F, AWHP_F, PCRP_F, AWCRP_F, PCPP_F, AWCPP_F, ARPATH_F, APPATH_F, FAMREL_F, LCAPOS_F, AWLCPPATH_F, DPTHLEN_F, LPATH_F, ISCAP_F, ISWH_F, E_O_F},
                                         {PT_F, PATH_F, CWL_F, CWP_F, GC_F, PS_F, CS_F, CVNCP_F, CPD_F, HLC_F, PP_F, HWP_F, E_O_F},
                                         {AWR_F, PR_F, AWHL_F, AWHP_F, PCRP_F, AWCRP_F, PCPP_F, AWCPP_F, ARPATH_F, APPATH_F, FAMREL_F, LCAPOS_F, AWLCPPATH_F, DPTHLEN_F, LPATH_F, ISCAP_F, ISWH_F, PL_F, PVP_F, HWL_F, CPI_F, E_O_F},
                                         {AWR_F, PR_F, AWHL_F, AWHP_F, PCRP_F, AWCRP_F, PCPP_F, AWCPP_F, ARPATH_F, APPATH_F, FAMREL_F, LCAPOS_F, AWLCPPATH_F, DPTHLEN_F, LPATH_F, ISCAP_F, ISWH_F, PVP_F, HWL_F, CPI_F, E_O_F}};

// number of views used for co-training
#define FEATURE_VIEW_COUNT 2


/**
 *  Constants
 */

// this list is taken from Swirl
const std::string AUX_VERBS[] = {"be", "am", "is", "was", "are", "were", "been", "being", 
                                 "get", "got", "gotten", "getting", "geting", "gets", ""};
                                 
const std::string CORE_ARGS[] = {"A0", "A1", "A2", "A3", "A4", "A5", "AA", ""};

const std::string WH[] = {"what", "which", "who", "how", "whose", "whom",
                          "when", "where", "why", ""};


/**
 *  Global Variables
 */

// Unfortunately, we had to use global variable instead of passing the 
// feature view as parameter to a function for a specific situation. This
// happens when the getLabeingProbability() or getPrdArgLabelProb() are called
// by isMoreLikelyLabeling() and isMoreLikelyLabel() to sort the sentences
// or samples based on the probability assigned by the classifier for each
// view. In this situation, since the latter two functions are function types 
// of comp for sorting, in C++ algorithm library, they cannot accept extra
// parameters to pass the required feature view. So, a global variable is 
// used to resolve the problem.
extern unsigned short gvActiveFeatureView;
 
#endif /* CONSTANTS_HPP */
