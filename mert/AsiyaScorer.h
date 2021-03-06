#ifndef MERT_ASIYA_SCORER_H_
#define MERT_ASIYA_SCORER_H_

#include <ostream>
#include <string>
#include <vector>

#include "Types.h"
#include "ScoreData.h"
#include "SentenceLevelScorer.h"
#include "ScopedVector.h"
#include "Util.h"
#include "Data.h"
#include "ScoreStats.h"

namespace MosesTuning
{

const int kAsiyaNgramOrder = 4;

class NgramCounts;
class Reference;

/**
 * Asiya scoring
 */
class AsiyaScorer: public SentenceLevelScorer
{
public:
  enum ReferenceLengthType {
    AVERAGE,
    CLOSEST,
    SHORTEST
  };

  explicit AsiyaScorer(const std::string& config = "");
  ~AsiyaScorer();

  virtual void setReferenceFiles(const std::vector<std::string>& referenceFiles);
  bool OpenReference(const char* filename, std::size_t file_id);
  const std::vector<Reference*>& GetReferences() const { return m_references.get(); }
  int CalcReferenceLength(std::size_t sentence_id, std::size_t length);

  virtual void doScoring();
  virtual statscore_t calculateScore(const std::vector<int>& comps) const;
  virtual std::size_t NumberOfScores() const { return 1; }

  virtual void addCandidateSentence(const std::string& sid, const std::string& sentence);
  virtual void prepareStats(std::size_t sindex, const std::string& text, ScoreStats& entry);
  std::vector<std::vector <ScoreStats> > getAllScoreStats();

private:
  ScopedVector<Reference> m_references;

  ReferenceLengthType m_ref_length_type;

  // reference files
  std::vector<std::string> m_reference_files;
  //source file
  std::string m_source_file;
  //config file for Asiya
  std::string m_config_file;
  //file with translation variants.
  std::string m_translation_file;
  //1st part of the name for files with reference translation.
  std::string m_reference_file;
  std::string m_dir;
  std::string used_metric;
  bool needToNormalizeScores;
  double minNormalization, maxNormalization; //borders for an initial normalization interval.

  // candidate sentences [sentence_idx][i_best]
  typedef std::vector<std::string> stringVector;
  std::vector<stringVector> m_candidate_sentences;
  std::vector<double> scores;

  // no copying allowed
  AsiyaScorer(const AsiyaScorer&);
  AsiyaScorer& operator=(const AsiyaScorer&);

  std::string execCommand(std::string cmd);
  void writeCandidateFile();
  void writeConfigFile();
  void writeReferenceFiles();
  void callAsiya();
  void readscores(std::string commandOutput);
};

float calculateAsiyaScore(const std::vector<float>& stats);

}

#endif  // MERT_ASIYA_SCORER_H_
