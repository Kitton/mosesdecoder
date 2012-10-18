#include "AsiyaScorer.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <climits>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "util/check.hh"
#include "Reference.h"
#include "Util.h"
#include "Vocabulary.h"
#include "Util.h"
#include "FeatureData.h"
#include "ScoreData.h"
#include "Data.h"


using namespace std;

namespace {

// configure regularisation
const char KEY_REFLEN[] = "reflen";
const char REFLEN_AVERAGE[] = "average";
const char REFLEN_SHORTEST[] = "shortest";
const char REFLEN_CLOSEST[] = "closest";

} // namespace

namespace MosesTuning
{

AsiyaScorer::AsiyaScorer(const string& config)
    : StatisticsBasedScorer("Asiya", config),
      m_ref_length_type(CLOSEST)
{
  cout << "asiya constructor." << endl;
  //todo. save from config the source file and save it!
  const string reflen = getConfig(KEY_REFLEN, REFLEN_CLOSEST);
  if (reflen == REFLEN_AVERAGE) {
    m_ref_length_type = AVERAGE;
  } else if (reflen == REFLEN_SHORTEST) {
    m_ref_length_type = SHORTEST;
  } else if (reflen == REFLEN_CLOSEST) {
    m_ref_length_type = CLOSEST;
  } else {
    throw runtime_error("Unknown reference length strategy: " + reflen);
  }
}

AsiyaScorer::~AsiyaScorer() {}


void AsiyaScorer::setReferenceFiles(const vector<string>& referenceFiles)
{
   cout << "asiya setreferencefiles." << endl;
  // Make sure reference data is clear
  m_references.reset();
  mert::VocabularyFactory::GetVocabulary()->clear();
  //todo. remove the m_reference_files vector

  //load reference data
  for (size_t i = 0; i < referenceFiles.size(); ++i) {
    TRACE_ERR("Loading reference from " << referenceFiles[i] << endl);
    //todo. check out if the file exists and throw an error
    m_reference_files.push_back( string(referenceFiles[i]) );
  }
}

void AsiyaScorer::setSourceFile(const string& sourceFile)
{
    cout << "asiya setsourcefile." << endl;
    TRACE_ERR("Loading source from " << sourceFile << endl);
    //todo. check out if the file exists and throw an error
    m_source_file =  string(sourceFile);
}

void AsiyaScorer::setConfigFile(const string &configFile)
{
    TRACE_ERR("Loading config from " << configFile << endl);
    m_config_file = string(configFile);
}



statscore_t AsiyaScorer::calculateScore(const vector<int>& comps) const
{
cout << "asiya calculate score. who is calling this function? " << endl;
  float logasiya = 0.0;
  // reflength divided by test length
  //todo. read it from the scores file
  return exp(logasiya);
}


void AsiyaScorer::addCandidateSentence(const string& sid, const string& sentence )
{
    int last_i = m_candidate_sentences.size();
    int idx = atoi( sid.c_str() );

    while ( last_i <= idx ) {
        vector<string> tmp;
        m_candidate_sentences.push_back(tmp);
        last_i ++;
    }
    m_candidate_sentences[idx].push_back(sentence);
}


void AsiyaScorer::doScoring( ScoreDataHandle m_score_data )
{
    cout << "doscoring " << endl;
    writeCandidateFile();
    writeConfigFile();
    callAsiya();
    //readScores();
    //score each sentence
  //m_scorer->prepareStats(sentence_index, sentence, scoreentry);
  // save the score for previous sentence. Do it aling with previous function
  //m_score_data->add(scoreentry, sentence_index);
}


void AsiyaScorer::writeCandidateFile()
{
    cout << "writing candidate file.." << endl;
    for (size_t i = 0; i < m_candidate_sentences.size(); ++i) {
        std::stringstream ss;
        ss << m_source_file << "." << i << ".out";
        string candfilename = ss.str();
        ofstream candfile;
        candfile.open( candfilename.c_str() );
        if ( candfile.is_open() ) {
            for (size_t j = 0; j < m_candidate_sentences.size(); ++j) {
                candfile << m_candidate_sentences[i][j]  << endl;
            }
            candfile.close();
        }
    }
}


void AsiyaScorer::writeConfigFile()
{

}


void AsiyaScorer::callAsiya()
{
    // ~/perl ../bin/Asiya.pl ./Asiya.config
    string perl_location = "~/perl";
    string asiya_location = "~/asiya//bin/Asiya.pl";
    string asiya_config_location = "Asiya.config";
    string run_command;
    run_command = perl_location + " " + asiya_location + " " + asiya_config_location;
    //Not a good variant, should try a special version for perl scripts.
    //Dont forget about "export ASIYA_HOME=~/asiya/"
    system(run_command.c_str());
}


void AsiyaScorer::prepareStats(size_t sid, const string& text, ScoreStats& entry)
{
    cout << "asiya prepare stats." << endl;
  if (sid >= m_reference_files.size()) {
    stringstream msg;
    msg << "Sentence id (" << sid << ") not found in reference set";
    throw runtime_error(msg.str());
  }

  // initialize the stats for this line
  vector<ScoreStatsType> stats(1); // this is an array for all the array of metric scores. now only one.

  //run asiya here
  // create the config file
  // create the command call
  // read the output and save it in the stats

  stats[0] = 1; //todo. what shuld be the initialization value?
  entry.set(stats);
}


}

