#ifndef moses_PhrasePairFeature_h
#define moses_PhrasePairFeature_h

#include <stdexcept>
#include <boost/unordered_set.hpp>

#include "StatelessFeatureFunction.h"
#include "moses/Factor.h"
#include "moses/Sentence.h"

namespace Moses
{

/**
  * Phrase pair feature: complete source/target phrase pair
  **/
class PhrasePairFeature: public StatelessFeatureFunction
{

  typedef std::map< char, short > CharHash;
  typedef std::vector< std::set<std::string> > DocumentVector;

  boost::unordered_set<std::string> m_vocabSource;
  DocumentVector m_vocabDomain;
  FactorType m_sourceFactorId;
  FactorType m_targetFactorId;
  bool m_unrestricted;
  bool m_simple;
  bool m_sourceContext;
  bool m_domainTrigger;
  bool m_ignorePunctuation;
  CharHash m_punctuationHash;
  std::string m_filePathSource;

public:
  PhrasePairFeature(const std::string &line);

  bool IsUseable(const FactorMask &mask) const;

  void Evaluate(const PhraseBasedFeatureContext& context,
                ScoreComponentCollection* accumulator) const;

  void EvaluateChart(const ChartBasedFeatureContext& context,
                     ScoreComponentCollection*) const {
    throw std::logic_error("PhrasePairFeature not valid in chart decoder");
  }

  void Load();
  void SetParameter(const std::string& key, const std::string& value);

};

}


#endif
