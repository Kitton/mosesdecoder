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
public:
  explicit DataAsiya(Scorer* scorer, const std::string& sparseweightsfile="");
  explicit DataAsiya();
  ~DataAsiya();
  
  void loadNBest(const std::string &file);

//  virtual void save(const std::string &featfile, const std::string &scorefile, bool bin=false);


};

}

#endif  // MERT_DATA_ASIYA_H_
