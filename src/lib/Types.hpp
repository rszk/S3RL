/**
 *  @ RSK
 *  @ Version 0.1        27.11.2009
 * 
 *  Type definitions and common data structures are gathered here to
 *  prevent some problems like compilation dependency
 * 
 * 
 *  Note: to print the values of vector elements, use:
 *        *(myVector._M_impl._M_start)@myVector.size()
 *   
 */

#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>

// a type for defining index of the word in sentence starting from 1
typedef short WordIdx;

/**
 *  The WordSpan is an abstract granularity for determining real argument 
 *  boundaries according to propbank annotation. This is stored because
 *  when other tokenization granularity are used which is derived from 
 *  automatic tools, there may be no exact alignment between PropBank 
 *  tokenization and them for some cases. (see Surdeanu & Turmo (2005)
 *  for example for contstituent-by-constituent tokenization)
 * 
 *  So, keeping this information, helps identify mismatches and take proper
 *  approach. Also, it can be used to analyze errors and inconsistencies
 *  in data.  
 */
struct WordSpan
{
 WordIdx start;    
 WordIdx end;
};

class Word;
typedef Word * WordPtr;

// Constituent is used instead of TBTree to increase representativeness 
// in Constituent-by-constituent tokenization
class TBTree;
typedef TBTree* Constituent;

class DTree;
typedef DTree* DTNode;

// because of importance, ArgLabel is defined to increase representativeness;
// it may be later converted to an enumeration!
typedef std::string ArgLabel;


#endif /* TYPES_HPP */
