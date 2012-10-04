#include "DataFactory.h"

#include <stdexcept>

#include "Data.h"
#include "DataAsiya.h"


using namespace std;

namespace MosesTuning
{

Data* DataFactory::getData(const string& type, Scorer *scorer) {
  if (type == "ASIYA") {
    return new DataAsiya(scorer);
  } else {
    return new Data(scorer);
  }
}

}

