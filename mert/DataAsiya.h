/*
 *  DataAsiya.h
 *  mert - Minimum Error Rate Training
 *
 *  Created by Nicola Bertoldi on 13/05/08.
 *
 */

#ifndef MERT_DATA_ASIYA_H_
#define MERT_DATA_ASIYA_H_

#include <vector>

#include "Data.h"
#include "Util.h"
#include "FeatureData.h"
#include "ScoreData.h"

namespace MosesTuning
{
  
class Scorer;

// NOTE: there is no copy constructor implemented, so only the
// compiler synthesised shallow copy is available.
class DataAsiya : public Data
{
private:
  Scorer* m_scorer;
  std::string m_score_type;
  std::size_t m_num_scores;
  bool m_sparse_flag;
  ScoreDataHandle m_score_data;
  FeatureDataHandle m_feature_data;

public:
  explicit DataAsiya(Scorer* scorer);
  explicit DataAsiya();
  ~DataAsiya();
  
  virtual void loadNBest(const std::string &file);

  virtual void save(const std::string &featfile, const std::string &scorefile, bool bin=false);


};

}

#endif  // MERT_DATA_ASIYA_H_
