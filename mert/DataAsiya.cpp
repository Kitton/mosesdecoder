/*
 *  DataAsiya.cpp
 *  mert - Minimum Error Rate Training
 *
 *  Created by Nicola Bertoldi on 13/05/08.
 *
 */

#include <algorithm>
#include <cmath>
#include <fstream>

#include "DataAsiya.h"
#include "FileStream.h"
#include "Scorer.h"
#include "AsiyaScorer.h"

using namespace std;

namespace MosesTuning
{
DataAsiya::DataAsiya(Scorer* scorer, const string &sparseweightsfile)
    : Data(scorer, sparseweightsfile)
{
  TRACE_ERR("DataAsiya::m_score_type " << m_score_type << endl);
  TRACE_ERR("DataAsiya::Scorer type from Scorer: " << m_scorer->getName() << endl);
}

DataAsiya::~DataAsiya(){
    //nothing todo
}


void DataAsiya::loadNBest(const string &file)
{
  TRACE_ERR("loading nbest from DataAsiya " << file << endl);
  inputfilestream inp(file); // matches a stream with a file. Opens the file
  if (!inp.good())
    throw runtime_error("Unable to open: " + file);

  ScoreStats scoreentry;
  string line, sentence_index, sentence, feature_str, alignment;

  AsiyaScorer* a_scorer = dynamic_cast<AsiyaScorer*>(m_scorer);
  
  /*todo. change this loop. instead of obtaining the score for each sentence, obtain all the scores at once!*/
  while (getline(inp, line, '\n')) {
    if (line.empty()) continue;
    // adding statistics for error measures
    scoreentry.clear();

    getNextPound(line, sentence_index, "|||"); // first field
    getNextPound(line, sentence, "|||");       // second field
    getNextPound(line, feature_str, "|||");    // third field

    if (line.length() > 0) {
      string temp;
      getNextPound(line, temp, "|||"); //fourth field sentence score
      if (line.length() > 0) {
        getNextPound(line, alignment, "|||"); //fourth field only there if alignment scorer
      }
    }
    //TODO check alignment exists if scorers need it
    if (a_scorer->useAlignment()) {
      sentence += "|||";
      sentence += alignment;
    }
    // prepare stats gets all the scores for sentence_i of sentence_index
//    a_scorer->addCandidateSentence(sentence_index, sentence);
    a_scorer->prepareStats(atoi(sentence_index.c_str()), sentence, scoreentry);

    // examine first line for name of features
    if (!existsFeatureNames()) {
      InitFeatureMap(feature_str);
    }
    AddFeatures(feature_str, atoi(sentence_index.c_str()));
  }

  a_scorer->doScoring();
//  TRACE_ERR("before getAllScoreStats" << endl);

  std::vector<std::vector <ScoreStats> > allScoreStats = a_scorer->getAllScoreStats();
  for (int i = 0; i < allScoreStats.size(); ++i)
      for(int j = 0; j < allScoreStats[i].size(); ++j)
      {
          stringstream ss;
          ss << i;
          m_score_data->add(allScoreStats[i][j], atoi(ss.str().c_str()));
//          TRACE_ERR("allScoreStats[" << i << "].size() " << allScoreStats[i].size() << " " << allScoreStats[i][j] << endl);
      }


  inp.close();
//  a_scorer->doScoring( m_score_data );

  //score each sentence
  //a_scorer->prepareStats(sentence_index, sentence, scoreentry);
  // save the score for previous sentence. Do it aling with previous function
  //m_score_data->add(scoreentry, sentence_index);  
  
}




//void DataAsiya::save(const std::string &featfile, const std::string &scorefile, bool bin) {
//  if (bin)
//    cerr << "Binary write mode is selected" << endl;
//  else
//    cerr << "Binary write mode is NOT selected" << endl;

//  m_feature_data->save(featfile, bin);
//  m_score_data->save(scorefile, bin);
//}


}

