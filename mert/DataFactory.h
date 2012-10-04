#ifndef MERT_DATA_FACTORY_H_
#define MERT_DATA_FACTORY_H_

#include <vector>
#include <string>

#include "Scorer.h"

namespace MosesTuning
{
  

class Data;

class DataFactory
{
public:
  static Data* getData(const std::string& type, Scorer* scorer);

private:
  DataFactory() {}
  ~DataFactory() {}
};

}

#endif  // MERT_DATA_FACTORY_H_
