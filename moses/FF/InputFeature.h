#pragma once

#include "InputFeature.h"
#include "StatelessFeatureFunction.h"

namespace Moses
{


class InputFeature : public StatelessFeatureFunction
{
protected:
  size_t m_numInputScores;
  size_t m_numRealWordCount;

public:
  InputFeature(const std::string &line);

  void SetParameter(const std::string& key, const std::string& value);

  bool IsUseable(const FactorMask &mask) const {
    return true;
  }

  size_t GetNumInputScores() const {
    return m_numInputScores;
  }
  size_t GetNumRealWordsInInput() const {
    return m_numRealWordCount;
  }

  virtual void Evaluate(const InputType &input
                        , const InputPath &inputPath
                        , ScoreComponentCollection &scoreBreakdown) const;

};


}

