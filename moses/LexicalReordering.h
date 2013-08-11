#ifndef moses_LexicalReordering_h
#define moses_LexicalReordering_h

#include <string>
#include <vector>
#include "Factor.h"
#include "Phrase.h"
#include "TypeDef.h"
#include "Util.h"
#include "WordsRange.h"

#include "LexicalReorderingState.h"
#include "LexicalReorderingTable.h"
#include "moses/FF/StatefulFeatureFunction.h"


namespace Moses
{

class Factor;
class Phrase;
class Hypothesis;
class InputType;

/** implementation of lexical reordering (Tilman ...) for phrase-based decoding
 */
class LexicalReordering : public StatefulFeatureFunction
{
public:
  LexicalReordering(const std::string &line);
  virtual ~LexicalReordering();
  void Load();

  virtual bool IsUseable(const FactorMask &mask) const;

  virtual const FFState* EmptyHypothesisState(const InputType &input) const;

  void InitializeForInput(const InputType& i) {
    m_table->InitializeForInput(i);
  }

  Scores GetProb(const Phrase& f, const Phrase& e) const;

  virtual FFState* Evaluate(const Hypothesis& cur_hypo,
                            const FFState* prev_state,
                            ScoreComponentCollection* accumulator) const;

  virtual FFState* EvaluateChart(const ChartHypothesis&,
                                 int /* featureID */,
                                 ScoreComponentCollection*) const {
    CHECK(0); // not valid for chart decoder
    return NULL;
  }

private:
  bool DecodeCondition(std::string s);
  bool DecodeDirection(std::string s);
  bool DecodeNumFeatureFunctions(std::string s);

  LexicalReorderingConfiguration *m_configuration;
  std::string m_modelTypeString;
  std::vector<std::string> m_modelType;
  LexicalReorderingTable* m_table;
  //std::vector<Direction> m_direction;
  std::vector<LexicalReorderingConfiguration::Condition> m_condition;
  //std::vector<size_t> m_scoreOffset;
  //bool m_oneScorePerDirection;
  std::vector<FactorType> m_factorsE, m_factorsF;
  std::string m_filePath;
};

}

#endif
