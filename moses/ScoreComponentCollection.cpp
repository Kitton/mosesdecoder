// $Id$
#include <vector>

#include "ScoreComponentCollection.h"
#include "StaticData.h"

using namespace std;

namespace Moses
{

ScoreComponentCollection::ScoreIndexMap ScoreComponentCollection::s_scoreIndexes;
size_t ScoreComponentCollection::s_denseVectorSize = 0;

ScoreComponentCollection::ScoreComponentCollection() : m_scores(s_denseVectorSize)
{}


void ScoreComponentCollection::RegisterScoreProducer
(const FeatureFunction* scoreProducer)
{
  size_t start = s_denseVectorSize;
  size_t end = start + scoreProducer->GetNumScoreComponents();
  VERBOSE(1, "FeatureFunction: " << scoreProducer->GetScoreProducerDescription() << " start: " << start << " end: " << (end-1) << endl);
  s_scoreIndexes[scoreProducer] = pair<size_t,size_t>(start,end);
  s_denseVectorSize = end;
}


float ScoreComponentCollection::GetWeightedScore() const
{
  return m_scores.inner_product(StaticData::Instance().GetAllWeights().m_scores);
}

void ScoreComponentCollection::MultiplyEquals(float scalar)
{
  m_scores *= scalar;
}

// Multiply all weights of this sparse producer by a given scalar
void ScoreComponentCollection::MultiplyEquals(const FeatureFunction* sp, float scalar)
{
  std::string prefix = sp->GetScoreProducerDescription() + FName::SEP;
  for(FVector::FNVmap::const_iterator i = m_scores.cbegin(); i != m_scores.cend(); i++) {
    std::stringstream name;
    name << i->first;
    if (name.str().substr( 0, prefix.length() ).compare( prefix ) == 0)
      m_scores[i->first] = i->second * scalar;
  }
}

// Count weights belonging to this sparse producer
size_t ScoreComponentCollection::GetNumberWeights(const FeatureFunction* sp)
{
  std::string prefix = sp->GetScoreProducerDescription() + FName::SEP;
  size_t weights = 0;
  for(FVector::FNVmap::const_iterator i = m_scores.cbegin(); i != m_scores.cend(); i++) {
    std::stringstream name;
    name << i->first;
    if (name.str().substr( 0, prefix.length() ).compare( prefix ) == 0)
      weights++;
  }
  return weights;
}

void ScoreComponentCollection::DivideEquals(float scalar)
{
  m_scores /= scalar;
}

void ScoreComponentCollection::CoreDivideEquals(float scalar)
{
  m_scores.coreDivideEquals(scalar);
}

void ScoreComponentCollection::DivideEquals(const ScoreComponentCollection& rhs)
{
  m_scores.divideEquals(rhs.m_scores);
}

void ScoreComponentCollection::MultiplyEquals(const ScoreComponentCollection& rhs)
{
  m_scores *= rhs.m_scores;
}

void ScoreComponentCollection::MultiplyEqualsBackoff(const ScoreComponentCollection& rhs, float backoff)
{
  m_scores.multiplyEqualsBackoff(rhs.m_scores, backoff);
}

void ScoreComponentCollection::MultiplyEquals(float core_r0, float sparse_r0)
{
  m_scores.multiplyEquals(core_r0, sparse_r0);
}

std::ostream& operator<<(std::ostream& os, const ScoreComponentCollection& rhs)
{
  os << rhs.m_scores;
  return os;
}
void ScoreComponentCollection::L1Normalise()
{
  m_scores /= m_scores.l1norm_coreFeatures();
}

float ScoreComponentCollection::GetL1Norm() const
{
  return m_scores.l1norm();
}

float ScoreComponentCollection::GetL2Norm() const
{
  return m_scores.l2norm();
}

float ScoreComponentCollection::GetLInfNorm() const
{
  return m_scores.linfnorm();
}

size_t ScoreComponentCollection::L1Regularize(float lambda)
{
  return m_scores.l1regularize(lambda);
}

void ScoreComponentCollection::L2Regularize(float lambda)
{
  m_scores.l2regularize(lambda);
}

size_t ScoreComponentCollection::SparseL1Regularize(float lambda)
{
  return m_scores.sparseL1regularize(lambda);
}

void ScoreComponentCollection::SparseL2Regularize(float lambda)
{
  m_scores.sparseL2regularize(lambda);
}

void ScoreComponentCollection::Save(ostream& out) const
{
  ScoreIndexMap::const_iterator iter = s_scoreIndexes.begin();
  for (; iter != s_scoreIndexes.end(); ++iter ) {
    string name = iter->first->GetScoreProducerDescription();
    IndexPair ip = iter->second; // feature indices
    if (ip.second-ip.first == 1) {
      out << name << " " << m_scores[ip.first] << endl;
    } else {
      for (size_t i=ip.first; i < ip.second; ++i) {
        ostringstream fullname;
        fullname << name << "_" << (i + 1 - ip.first);
        out << fullname.str() << " " << m_scores[i] << endl;
      }
    }
  }

  // write sparse features
  m_scores.write(out);
}

void ScoreComponentCollection::Save(const string& filename) const
{
  ofstream out(filename.c_str());
  if (!out) {
    ostringstream msg;
    msg << "Unable to open " << filename;
    throw runtime_error(msg.str());
  }
  Save(out);
  out.close();
}

void ScoreComponentCollection::Assign(const FeatureFunction* sp, const string line)
{
  istringstream istr(line);
  while(istr) {
    string namestring;
    FValue value;
    istr >> namestring;
    if (!istr) break;
    istr >> value;
    FName fname(sp->GetScoreProducerDescription(), namestring);
    m_scores[fname] = value;
  }
}

void ScoreComponentCollection::InvertDenseFeatures(const FeatureFunction* sp)
{

  Scores old_scores = GetScoresForProducer(sp);
  Scores new_scores(old_scores.size());

  for (size_t i = 0; i != old_scores.size(); ++i) {
    new_scores[i] = -old_scores[i];
  }

  Assign(sp, new_scores);
}

void ScoreComponentCollection::ZeroDenseFeatures(const FeatureFunction* sp)
{
  size_t numScores = sp->GetNumScoreComponents();
  Scores vec(numScores, 0);

  Assign(sp, vec);
}

//! get subset of scores that belong to a certain sparse ScoreProducer
FVector ScoreComponentCollection::GetVectorForProducer(const FeatureFunction* sp) const
{
  FVector fv(s_denseVectorSize);
  std::string prefix = sp->GetScoreProducerDescription() + FName::SEP;
  for(FVector::FNVmap::const_iterator i = m_scores.cbegin(); i != m_scores.cend(); i++) {
    std::stringstream name;
    name << i->first;
    if (name.str().substr( 0, prefix.length() ).compare( prefix ) == 0)
      fv[i->first] = i->second;
  }
  return fv;
}

}


