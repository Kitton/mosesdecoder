#include "StatelessFeatureFunction.h"

namespace Moses
{

StatelessFeatureFunction::StatelessFeatureFunction(const std::string& description, const std::string &line)
  :FeatureFunction(description, line)
{
  m_statelessFFs.push_back(this);
}

StatelessFeatureFunction::StatelessFeatureFunction(const std::string& description, size_t numScoreComponents, const std::string &line)
  :FeatureFunction(description, numScoreComponents, line)
{
  m_statelessFFs.push_back(this);
}

}

