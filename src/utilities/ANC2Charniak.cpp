/**
 *  @ RSK
 *  @ Version 0.1        14.01.2010
 * 
 *  excutable file to read ANC text files including sentence boundaries
 *  (produced by ANC Tool), process it and convert to Charniak parser's 
 *  input format (<s> </s>), and gathering corpus analysis data
 *  
 *  The output of the program can be separate Charniak input files
 *  for each ANC text file or a merged Charniak input files for each
 *  ANC directory based on the executable argument
 * 
 */

#include <iostream>
#include <fstream>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>

#include "Utils.hpp"

using namespace std;


#define MIN_SENTENCE_LENGTH 3
#define MAX_SENTENCE_LENGTH 100


struct corpusAnalysis
{
 unsigned int fileCount;
 unsigned int dirCount;
 unsigned int sntncBoundaryCount;
 unsigned int overlapCount;
 unsigned int formulaCount;
 unsigned int shortCount;
 unsigned int longCount;
 unsigned int incompleteCount;
 unsigned int ignoredMBCount;
 };

bool processDir(const string &, bool, bool, const string &, ostream &, 
                corpusAnalysis &, ostream &);
bool processFile(const string &, ostream &, corpusAnalysis &);
bool processSentence(string &, corpusAnalysis &);
void initAnalysisStruct(corpusAnalysis &);
void logCorpusAnalysis(corpusAnalysis &, ostream &);
void logDirAnalysis(corpusAnalysis &, ostream &, const string &);
void logFileAnalysis(corpusAnalysis &, ostream &);
void updateGlobalAnalysis(corpusAnalysis &, corpusAnalysis &, bool);


static void usage(const char * execName)
{
 cout << "Usage: " << execName << "[option][..]" << endl
      << "<i=[Source directory]>" << endl
      << "<o=[Output directory]>" << endl
      << "<s(Include subdirectories)>" << endl
      << "<m(Merged file per directory)>" << endl
      <<  endl;
}

int main(int argc, char ** argv)
{
 string vSourceDir = "../../corpus/OANC/original";
 string vOutputDir = "../../corpus/OANC/Charniak/input";
 bool vSubDirs = false;
 bool vMerg = false;
 
 const string ctmpStr  = "help";
 if(argv[1] == ctmpStr.c_str())
 {
  usage(argv[0]);
  exit(-1);
  }
 
 for (unsigned short cntArg = 1; cntArg < argc; cntArg++)
 { 
  if (argv[cntArg][0] == 'i')
  { 
   vSourceDir = argv[cntArg] + 2;
   continue;
   }
   
  if (argv[cntArg][0] == 'o')
  { 
   vOutputDir = argv[cntArg] + 2;
   continue;
   }
   
  if (argv[cntArg][0] == 's')
  { 
   vSubDirs = true;
   continue;
   }
   
  if (argv[cntArg][0] == 'm')
  { 
   vMerg = true;
   continue;
   }

  }   
 
 /**
  *  processing source directory, and all subdirectories if the vSubDirs
  *  option is set, to convert all included files
  */
 
 string vLogFile = vOutputDir + "/" + "CorpusStat.log";
 ofstream strmLog (vLogFile.c_str());
 if(!strmLog)
 {
  cerr << "\nCan't create log file: " << vLogFile << endl;
  return -1;
  }
 
 corpusAnalysis vCorpusAnalysis;
 initAnalysisStruct(vCorpusAnalysis);
 
 ofstream strmCharniak;
 if (vMerg)
 {
  string pCharniakFile = (vOutputDir + "/" + extractFileName(vSourceDir, false) + ".cha.input");
  strmCharniak.open(pCharniakFile.c_str());
  if(!strmCharniak)
  {
   cerr << "\nCan't create output file: " << pCharniakFile << endl;
   return -1;
   }
  }
 
 cout << "\nProcess started ..." << endl;
 if (processDir(vSourceDir, vSubDirs, vMerg, vOutputDir, strmCharniak,
                vCorpusAnalysis, strmLog))
  cout << "Process is done!" << endl;
 else 
  cerr << "\nProcess was unsuccessful! Please check directories and files"
       << endl << endl;
 
 logCorpusAnalysis(vCorpusAnalysis, strmLog);
 logCorpusAnalysis(vCorpusAnalysis, cout);
}

/**
 * 
 *  Function definitions
 * 
 */ 

/**
 *  a recurseive funstion to process directory structure
 * 
 *  It verifies all directories and files inside the passed pSourceDir
 *  and recursively calls itself for directories and calls processFile()
 *  for files. It also logs and collects analysis statistics of every 
 *  folder and file into the global corpus analysis
 * 
 */
bool processDir(const string &pSourceDir, bool pSubDirs, bool pMerg,
                const string &pOutputDir, ostream &pCharniakStream,
                corpusAnalysis &pCAnalysis, ostream &pLogStream)
{
 DIR *vDir;
 struct dirent *vDirEntry; 
 vDir = opendir(pSourceDir.c_str());
 
 if (vDir != NULL)
 {
  while ((vDirEntry = readdir(vDir)))
  {
   if ((strcmp(vDirEntry->d_name, ".") == 0) ||
       (strcmp(vDirEntry->d_name, "..") == 0))
    continue;
   else
   {
   	bool vIsDir = isDir(pSourceDir + "/" + vDirEntry->d_name); 
    if ((vIsDir) && (pSubDirs))
    {
     cout << "\nEntering directory: /" << vDirEntry->d_name << endl;
     pLogStream << "\nDirectory: /" << vDirEntry->d_name << endl;
  
     corpusAnalysis vDirAnalysis;
     initAnalysisStruct(vDirAnalysis);
     
     string vOutputDir = pOutputDir + "/" + vDirEntry->d_name;
     if (mkdir(vOutputDir.c_str(), 0777) != 0)
     {
      cerr << "Can't create directory: " 
           << pOutputDir + "/" + vDirEntry->d_name << endl;
      return false;
      }
     
     ofstream strmCharniak;
     // if all output files for this directory should be merged, then
     // create stream here and send to processDir() which will process
     // the files of this directory
     if (pMerg)
     {
      string pCharniakFile = (vOutputDir + "/" + vDirEntry->d_name + ".cha.input");
      strmCharniak.open(pCharniakFile.c_str());
      if(!strmCharniak)
      {
       cerr << "\nCan't create output file: " << pCharniakFile << endl;
       return -1;
       }
      }

     processDir(pSourceDir + "/" + vDirEntry->d_name, pSubDirs, pMerg, 
                vOutputDir, strmCharniak, vDirAnalysis, pLogStream);
                
     logDirAnalysis(vDirAnalysis, pLogStream, vDirEntry->d_name);
     logDirAnalysis(vDirAnalysis, cout, vDirEntry->d_name);
     updateGlobalAnalysis(pCAnalysis, vDirAnalysis, true);
     }
    else if (!vIsDir)
    {
     corpusAnalysis vFileAnalysis;
     initAnalysisStruct(vFileAnalysis);
     
     // if each output file for this directory should be separate, then
     // create stream here
     if (pMerg)
     {
      if (!processFile(pSourceDir + "/" + vDirEntry->d_name,
                       pCharniakStream, vFileAnalysis))
       return false;
      }
     else 
     {
      string pCharniakFile = (pOutputDir + "/" + 
                              extractFileName(pSourceDir + "/" + vDirEntry->d_name, false) +
                              ".cha.input");
      ofstream strmCharniak (pCharniakFile.c_str());
      if(!strmCharniak)
      {
       cerr << "\nCan't create output file: " << pCharniakFile << endl;
       return -1;
       }

      if (!processFile(pSourceDir + "/" + vDirEntry->d_name,
                      strmCharniak, vFileAnalysis))
       return false;
      }

     pLogStream << endl << pSourceDir << "/" << vDirEntry->d_name;
     logFileAnalysis(vFileAnalysis, pLogStream);
     logFileAnalysis(vFileAnalysis, cout);
     updateGlobalAnalysis(pCAnalysis, vFileAnalysis, false);
     }
    }
   } 
  closedir(vDir);
  }
 else 
  return false;
     
 return true;
}

/**
 *  extracts sentences from a file, processes them by processSentence(),
 *  and add to existing Charniak input file or a new file with the same 
 *  name based on the value of pAppend parameter
 */
bool processFile(const string &pFile, ostream &pStream, corpusAnalysis &pCAnalysis)
{
 string vLine;
 unsigned int cntSentence = 0;
 
 ifstream strmFile (pFile.c_str());
     
 if(!strmFile)
 {
  cerr << "Can't open file: " << pFile << endl;
  return false;
  }
 else 
  cout << "\nProcessing file: " << pFile << endl;
 
 // looping to read lines
 while (!strmFile.eof())
 {
  size_t vBeginPos, vEndPos;
  string tmpSentence;
  
  // looping to read lines to find a begining tag
  while (!strmFile.eof())
  {
   vBeginPos = vLine.find("<s id=");
   if (vBeginPos != string::npos)
   {
    if (vBeginPos != 0)
     vLine.erase(0, vBeginPos - 1);
    vBeginPos = vLine.find(">") + 1;
    vLine.erase(0, vBeginPos);
    break;
    }
   else
    getline(strmFile, vLine);
   }
  
  // looping to read lines to find a end tag and collecting sentence 
  // parts on the way
  while (!strmFile.eof())
  {
   vEndPos = vLine.find("</s>");
   if (vEndPos != string::npos)
   {
    tmpSentence += vLine.substr(0, vEndPos);
    vLine = vLine.erase(0, vEndPos + 4);
    break;
    }
   else
   {
    // trim \r\n from the end of line if any exits
    trimLine(vLine);
    // a space is put at the end of line to prevent the last word from
    // being sticked to the first word of next line
    tmpSentence += (vLine + " ");
   	getline(strmFile, vLine);
    }
   }  
  
  // if the last line before eof was empty line, ignore the sentence
  if (trim(tmpSentence) == "")
   continue;
  
  if (processSentence(tmpSentence, pCAnalysis))
  {
   pStream << tmpSentence << endl;
   pCAnalysis.sntncBoundaryCount++; 
   }
  
  cntSentence++;
  cout << "\r" << cntSentence << " sentences processed" << flush;
  }
 
 strmFile.close();
 return true;
}

/**
 * processed the extracted sentence, correct some known issues, and check
 * for the sentence eligibility to be selected, then return true if it is
 * selected
 */

bool processSentence(string &pSentence, corpusAnalysis &pCAnalysis)
{
 size_t tmpPosition = 0;
 
 // if there is a "<s" or "</s>" in the sentence, there has been overlap,
 // so ignore this sentence
 if ((pSentence.find("<s") != string::npos) ||
     (pSentence.find("</s>") != string::npos))
 {
  pCAnalysis.overlapCount++;
  return false;
  }
  
 // if there is a "(" without ")", the sentence is incomplete, so ignore
 tmpPosition = pSentence.find_last_of('(') ;
 if ((tmpPosition != string::npos) && 
     (pSentence.find_first_of(')', tmpPosition + 1) == string::npos))
 {
  pCAnalysis.incompleteCount++;
  return false;
  }
  
 // if there is a ")" without "(" preceding it, the sentence is incomplete,
 // so ignore
 tmpPosition = pSentence.find_first_of(')') ;
 if ((tmpPosition != string::npos) && 
     (pSentence.find_last_of('(', tmpPosition - 1) == string::npos))
 {
  pCAnalysis.incompleteCount++;
  return false;
  }
  
 // if there is a "[" without "]", the sentence is incomplete, so ignore
 tmpPosition = pSentence.find_last_of('[') ;
 if ((tmpPosition != string::npos) && 
     (pSentence.find_first_of(']', tmpPosition + 1) == string::npos))
 {
  pCAnalysis.incompleteCount++;
  return false;
  }
  
 // if there is a "]" without "[" preceding it, the sentence is incomplete,
 // so ignore
 tmpPosition = pSentence.find_first_of(']') ;
 if ((tmpPosition != string::npos) && 
     (pSentence.find_last_of('[', tmpPosition - 1) == string::npos))
 {
  pCAnalysis.incompleteCount++;
  return false;
  }
  
 // if there is a "=" in the sentence, it contains a formula, most probably
 // not a sentence; so ignore it
 if (pSentence.find('=') != string::npos)
 {
  pCAnalysis.formulaCount++;
  return false;
  }
  
 // We found that if a charachter with code -30 was found in the string,
 // the following 2 characters are multibyte. 
 // We treat some of them and ignore sentences with unknown multibytes.
 while (true)
 {
  tmpPosition = pSentence.find(-30);
  if (tmpPosition != string::npos)
  {
   // replacing "’" with "'"
   if ((pSentence[tmpPosition + 1] == '\200') &&
       (pSentence[tmpPosition + 2] == '\231'))
   {    
    pSentence.replace(tmpPosition, 3, "'");
    continue;
    }
    
   // replacing special space with normal space
   if ((pSentence[tmpPosition + 1] == '\200') &&
       (pSentence[tmpPosition + 2] == '\202'))
   {    
    pSentence.replace(tmpPosition, 3, " ");
    continue;
    }
    
   // replacing any '“' and '”' with '"'
   if ((pSentence[tmpPosition + 1] == '\200') &&
       ((pSentence[tmpPosition + 2] == '\234') ||
        (pSentence[tmpPosition + 2] == '\235')))
   {    
    pSentence.replace(tmpPosition, 3, "\"");
    continue;
    }
    
   // replacing any '—' with '-'
   if ((pSentence[tmpPosition + 1] == '\200') &&
       ((pSentence[tmpPosition + 2] == '\223') ||
        (pSentence[tmpPosition + 2] == '\224')))
   {    
    pSentence.replace(tmpPosition, 3, "-");
    continue;
    }
    
   pCAnalysis.ignoredMBCount++;
   return false; 
   }
  else
   break;
  } 

 // replacing any "&quot;" (ANC Tool changes any " to "&quot;") with "
 while (true)
 {
  tmpPosition = pSentence.find("&quot;");
  if (tmpPosition != string::npos)
   pSentence.replace(tmpPosition, 6, "\"");
  else
   break; 
  }
 
 // replacing any "&lt;" (ANC Tool changes any < to "&lt;") with <
 while (true)
 {
  tmpPosition = pSentence.find("&lt;");
  if (tmpPosition != string::npos)
   pSentence.replace(tmpPosition, 4, "<");
  else
   break; 
  }
 
 // replacing any "&gt;" (ANC Tool changes any > to "&gt;") with >
 while (true)
 {
  tmpPosition = pSentence.find("&gt;");
  if (tmpPosition != string::npos)
   pSentence.replace(tmpPosition, 4, ">");
  else
   break; 
  }
 
 // replacing any "&amp;" (ANC Tool changes any & to "&amp;") with &
 while (true)
 {
  tmpPosition = pSentence.find("&amp;");
  if (tmpPosition != string::npos)
   pSentence.replace(tmpPosition, 5, "&");
  else
   break; 
  }
 
 // replacing any '."' '".' to prevent incorrect parse results
 while (true)
 {
  tmpPosition = pSentence.find(".\"");
  if (tmpPosition != string::npos)
   pSentence.replace(tmpPosition, 2, "\".");
  else
   break; 
  }
 
 // removing "(Figure ..)"
 while (true)
 {
  tmpPosition = pSentence.find("(Fig");
  if (tmpPosition != string::npos)
  {
   size_t tmpPosition2 = pSentence.find_first_of(')', tmpPosition + 1);
   if (tmpPosition2 != string::npos)
    pSentence.erase(tmpPosition, tmpPosition2 - tmpPosition + 1);
   }
  else
   break; 
  }
 
 // removing "[...]" assuming that those are citations
 while (true)
 {
  tmpPosition = pSentence.find('[');
  if (tmpPosition != string::npos)
  {
   size_t tmpPosition2 = pSentence.find_first_of(']', tmpPosition + 1);
   if (tmpPosition2 != string::npos)
    pSentence.erase(tmpPosition, tmpPosition2 - tmpPosition + 1);
   }
  else
   break; 
  }
 
 // ignoring short and long sentences
 vector<string> vcTokens;
 tokenize(pSentence, vcTokens, " \t\n\r");
 if (vcTokens.size() < MIN_SENTENCE_LENGTH)
 {
  pCAnalysis.shortCount++;
  return false;
  }
 else if (vcTokens.size() > MAX_SENTENCE_LENGTH) 
 {
  pCAnalysis.longCount++;
  return false;
  }

 pSentence = "<s> " + pSentence + " </s>";
 return true; 
}

void initAnalysisStruct(corpusAnalysis &pCA)
{
 pCA.fileCount = 0;
 pCA.dirCount = 0;
 pCA.sntncBoundaryCount = 0;
 pCA.overlapCount = 0;
 pCA.formulaCount = 0;
 pCA.shortCount = 0;
 pCA.longCount = 0;
 pCA.incompleteCount = 0;
 pCA.ignoredMBCount = 0;
}

void updateGlobalAnalysis(corpusAnalysis &pGlobalAnalysis,
                          corpusAnalysis &pLocalAnalysis,
                          bool pDir)
{
 if (pDir)
 {
  pGlobalAnalysis.dirCount += (pLocalAnalysis.dirCount + 1);
  pGlobalAnalysis.fileCount += pLocalAnalysis.fileCount;
  }
 else 
  pGlobalAnalysis.fileCount++;
 pGlobalAnalysis.sntncBoundaryCount += pLocalAnalysis.sntncBoundaryCount;
 pGlobalAnalysis.overlapCount += pLocalAnalysis.overlapCount;
 pGlobalAnalysis.formulaCount += pLocalAnalysis.formulaCount;
 pGlobalAnalysis.shortCount += pLocalAnalysis.shortCount;
 pGlobalAnalysis.longCount += pLocalAnalysis.longCount;
 pGlobalAnalysis.incompleteCount += pLocalAnalysis.incompleteCount;
 pGlobalAnalysis.ignoredMBCount += pLocalAnalysis.ignoredMBCount;
}

void logCorpusAnalysis(corpusAnalysis &pCA, ostream &pStream)
{
 pStream << endl
         << pCA.fileCount << " files in " << pCA.dirCount << " directories" << endl
         << pCA.sntncBoundaryCount << " sentence boundaries" << endl 
         << pCA.overlapCount << " overlaps (ignored)" << endl
         << pCA.formulaCount << " formulas (ignored)" << endl
         << pCA.shortCount << " shorter than " << MIN_SENTENCE_LENGTH << " (ignored)" << endl
         << pCA.longCount << " longer than " << MAX_SENTENCE_LENGTH << " (ignored)" << endl
         << pCA.incompleteCount << " incomplete (ignored)" << endl
         << pCA.ignoredMBCount << " multibytes ignored" << endl;
}

void logDirAnalysis(corpusAnalysis &pDA, ostream &pStream, const string &pDir)
{
 pStream << endl
         << pDA.fileCount << " files in /" << pDir << endl
         << pDA.sntncBoundaryCount << " sentence boundaries" << endl 
         << pDA.overlapCount << " overlaps (ignored)" << endl
         << pDA.formulaCount << " formulas (ignored)" << endl
         << pDA.shortCount << " shorter than " << MIN_SENTENCE_LENGTH << " (ignored)" << endl
         << pDA.longCount << " longer than " << MAX_SENTENCE_LENGTH << " (ignored)" << endl
         << pDA.incompleteCount << " incomplete (ignored)" << endl
         << pDA.ignoredMBCount << " multibytes ignored" << endl;
}

void logFileAnalysis(corpusAnalysis &pFA, ostream &pStream)
{
 pStream << endl
         << pFA.sntncBoundaryCount << " sentence boundaries" << endl 
         << pFA.overlapCount << " overlaps (ignored)" << endl
         << pFA.formulaCount << " formulas (ignored)" << endl
         << pFA.shortCount << " shorter than " << MIN_SENTENCE_LENGTH << " (ignored)" << endl
         << pFA.longCount << " longer than " << MAX_SENTENCE_LENGTH << " (ignored)" << endl
         << pFA.incompleteCount << " incomplete (ignored)" << endl
         << pFA.ignoredMBCount << " multibytes ignored" << endl;
}
