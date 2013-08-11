#include "AsiyaScorer.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <climits>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>

#include "util/check.hh"
#include "Reference.h"
#include "Util.h"
#include "Vocabulary.h"
#include "Util.h"
#include "ScoreData.h"
#include "Data.h"
#include "FileStream.h"

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

namespace {

// configure regularisation
const char KEY_REFLEN[] = "reflen";
const char REFLEN_AVERAGE[] = "average";
const char REFLEN_SHORTEST[] = "shortest";
const char REFLEN_CLOSEST[] = "closest";

} // namespace

namespace MosesTuning
{

AsiyaScorer::AsiyaScorer(const string& config)
    : SentenceLevelScorer("Asiya", config),
      m_ref_length_type(CLOSEST),
      m_dir("temp"),
      minNormalization(-1),
      maxNormalization(0)
{
    cout << "AsiyaScorer::AsiyaScorer" << endl;
    m_source_file = m_dir + "/trans.txt";        //Fake source file
    m_config_file = m_dir + "/Asiya.config";
    m_translation_file = m_dir + "/trans.txt";
    m_reference_file = m_dir + "/ref.txt_";

    std::ifstream in("../asiya.metric");
    in >> used_metric;
    in.close();

    needToNormalizeScores = false;
    //if scores aren't in interval [0;1] normalize it to it.
//    if ( used_metric.find("-WER")!=string::npos || used_metric.find("-TER")!=string::npos || used_metric.find("-PER")!=string::npos )
//        needToNormalizeScores = true;

    //todo. save from config the source file and save it!
    const string reflen = getConfig(KEY_REFLEN, REFLEN_CLOSEST);
    if (reflen == REFLEN_AVERAGE) {
        m_ref_length_type = AVERAGE;
    } else if (reflen == REFLEN_SHORTEST) {
        m_ref_length_type = SHORTEST;
    } else if (reflen == REFLEN_CLOSEST) {
        m_ref_length_type = CLOSEST;
    } else {
        throw runtime_error("Unknown reference length strategy: " + reflen);
    }
}

AsiyaScorer::~AsiyaScorer() {}


void AsiyaScorer::setReferenceFiles(const vector<string>& referenceFiles)
{
    m_references.reset();

    mert::VocabularyFactory::GetVocabulary()->clear();
    //todo. remove the m_reference_files vector

    //load reference data
    for (size_t i = 0; i < referenceFiles.size(); ++i) {
        TRACE_ERR("Loading reference from " << referenceFiles[i] << endl);
        //todo. check out if the file exists and throw an error
        m_reference_files.push_back( string(referenceFiles[i]) );
    }
}


statscore_t AsiyaScorer::calculateScore(const vector<int>& comps) const
{
    double result = double(comps[0])/1000000.;

    return result;
}

float calculateAsiyaScore(const vector<float>& stats)
{
    double result = double(stats[0] + 1)/1000000.;

    return result;
}

void AsiyaScorer::addCandidateSentence(const string& sid, const string& sentence)
{
    int last_i = m_candidate_sentences.size();
    int idx = atoi( sid.c_str() );

    while ( last_i <= idx ) {
        vector<string> tmp;
        m_candidate_sentences.push_back(tmp);
        last_i ++;
    }
    m_candidate_sentences[idx].push_back(sentence);
}

std::vector<std::vector <ScoreStats> > AsiyaScorer::getAllScoreStats()
{
    std::vector<std::vector <ScoreStats> > result;
    int current_vector = -1;

    for (int i = 0; i < scores.size(); ++i)
    {
        if (i%m_candidate_sentences[0].size() == 0)
        {
            ++current_vector;
            std::vector <ScoreStats> vec;
            result.push_back(vec);
        }

        vector<ScoreStatsType> stats;
        stats.push_back(int(scores[i]*1000000));
        ScoreStats entry;
        entry.set(stats);

        result[current_vector].push_back(entry);
    }
    return result;
}

void AsiyaScorer::doScoring()
{
    if (m_candidate_sentences.empty())
        return;
    mkdir(m_dir.c_str(), 0777);
    writeReferenceFiles();
    writeCandidateFile();
    writeConfigFile();
    callAsiya();

//    for (int i = 0; i < scores.size(); ++i)
//    {
//        //put just 1 score now.
//        vector<ScoreStatsType> stats(1);
//        ScoreStats entry;
//        m_score_data->add(scoreentry, sentence_index);
//    }

    //score each sentence
    //m_scorer->prepareStats(sentence_index, sentence, scoreentry);
    // save the score for previous sentence. Do it aling with previous function
    //m_score_data->add(scoreentry, sentence_index);
}

void AsiyaScorer::readscores(string commandOutput)
{
    std::stringstream ss(commandOutput);

    std::string line;
    //TODO change it later.
    std::string searchedWord = "UNKNOWN_SET";

    while(std::getline(ss,line))
    {
        if (line.find(searchedWord) != std::string::npos)
        {
            std::stringstream ssLine(line);
            std::string set, doc, seg, metric, score;
            ssLine >>set >> doc >> seg >> metric >> score;
            double temp = atof(score.c_str());
            if (needToNormalizeScores) {
                if (temp < minNormalization)
                    temp = minNormalization;
                temp = (temp - minNormalization)/(maxNormalization - minNormalization);
            }

            scores.push_back(temp);
        }
    }
}

void AsiyaScorer::writeReferenceFiles()
{
    for (size_t i = 0; i < m_reference_files.size(); ++i)
    {
        std::stringstream ss;
        ss << m_reference_file << i;
        string filename = ss.str();
        ofstream file(filename.c_str());
        if (file.is_open())
        {
            inputfilestream inputRef(m_reference_files[i].c_str());

            //Check if its opened correctly.
            if (!inputRef.good())
            {
                cout << "------AsiyaScorer::writeReferenceFile. Can't read " << m_reference_files[i];
                inputRef.close();
                file.close();
                continue;
            }

            //Number of reference and translated sentences should be equal.
            size_t countRefSentences = std::count(std::istreambuf_iterator<char>(inputRef), std::istreambuf_iterator<char>(), '\n');
            inputRef.seekg(0, ios::beg);    //go to the begining.
            if (m_candidate_sentences.size() != countRefSentences)
            {
                cout << "-------Not equal number of translated and reference sentences " << m_reference_files[i] << endl;
                inputRef.close();
                file.close();
                continue;
            }
            //Create new reference file(s) with each ref.sentence repeated N-times in row for every N-best translation.
            for (size_t i1 = 0; i1 < m_candidate_sentences.size(); ++i1)
            {
                string line;
                while (getline(inputRef, line))
                {
                    if (line.empty()) continue;
                    for (size_t j = 0; j < m_candidate_sentences[i1].size(); ++j)
                        file << line << endl;
                    break;
                }
            }
            inputRef.close();
        }
        file.close();
    }
}

void AsiyaScorer::writeCandidateFile()
{
    //Write all translations into one file.
    ofstream candfile(m_translation_file.c_str());
    if ( candfile.is_open() )
        for (size_t i = 0; i < m_candidate_sentences.size(); ++i)
            for (size_t j = 0; j < m_candidate_sentences[i].size(); ++j)
                candfile << m_candidate_sentences[i][j]  << endl;

    candfile.close();
}


void AsiyaScorer::writeConfigFile()
{
    ofstream config_file;
    config_file.open (m_config_file.c_str());
    if ( config_file.is_open() ) {
        config_file << "input=raw"  << endl;
        config_file << "srclang=es"  << endl;
        config_file << "trglang=en"  << endl;
        config_file << "src=" << m_source_file  << endl;
        for (size_t i = 0; i < m_reference_files.size(); ++i)
            config_file << "ref=" << m_reference_file << i << endl;

        config_file << "sys=" << m_translation_file << endl;
    }
    config_file.close();
}


void AsiyaScorer::callAsiya()
{
    //perl  ~/../operador/asiya//bin/Asiya.pl  temp/Asiya.config  -eval single -m BLEU -g seg
    string perl_location = "perl ";
    string asiya_location = " ~/../operador/asiya/bin/Asiya.pl ";
    string evalType = "single";

    //If a few metrics used - get a normalized mean.
    if ( used_metric.find(",")!=string::npos )
        evalType = "ulc";

    string params = " -eval " + evalType + " -g seg -o smatrix -remake -m " + used_metric;
    string run_command;
    run_command = perl_location + asiya_location + " " +  m_config_file + " " + params;
    //stderr->stdout
    run_command += " 2>&1";

    const int N_LINES_LIMIT = 5000; //limit for chunking
    string lines = execCommand("wc -l " + m_translation_file);

    ofstream logFile("temp/log.txt", ios::app);
    logFile << "lines = " << lines;

    int n_lines = atoi(lines.c_str());
    if (n_lines > N_LINES_LIMIT){
        //copy files
        string com = "cp " + m_translation_file + " " + m_translation_file + "_copy";
        execCommand(com);
        string reference_file = m_reference_file + "0";
        com = "cp " + reference_file + " " + reference_file + "_copy";
        execCommand(com);

        int begin = 0;
        while (true) {
            int tail = n_lines - begin;
            stringstream ss;
            ss << tail;
            string tail_str = ss.str();

            ss.str(std::string());
            ss << N_LINES_LIMIT;
            string head_str = ss.str();
            com = "tail -n " + tail_str + " " + m_translation_file + "_copy" + " | head -n " + head_str + " > " + m_translation_file;
            execCommand(com);
            com = "tail -n " + tail_str + " " + reference_file + "_copy" + " | head -n " + head_str + " > " + reference_file;
            execCommand(com);

            logFile << com << endl;

            string result = execCommand(run_command);
            readscores(result);

            begin += N_LINES_LIMIT;
            if (begin > n_lines)
                break;
        }
    }
    else {
        string result = execCommand(run_command);
        readscores(result);
    }
    logFile.close();
}

string AsiyaScorer::execCommand(string cmd)
{
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}


void AsiyaScorer::prepareStats(size_t sid, const string& text, ScoreStats& entry)
{
    //just add a sentence to the condidate list.
    std::ostringstream s;
    s << sid;
    const string sidStr(s.str());
    addCandidateSentence(sidStr, text);

//    cout << "asiya prepare stats. " << text << endl;
//    if (sid >= m_reference_files.size())
//    {
//        stringstream msg;
//        msg << "Sentence id (" << sid << ") not found in reference set";
//        throw runtime_error(msg.str());
//    }

//    // initialize the stats for this line
//    vector<ScoreStatsType> stats(1); // this is an array for all the array of metric scores. now only one.

//    //run asiya here
//    // create the config file
//    // create the command call
//    // read the output and save it in the stats

//    stats[0] = 1; //todo. what shuld be the initialization value?
//    entry.set(stats);
}

//float AsiyaScorer::score(const candidates_t& candidates)
//{
//    doScoring();
//    float score;
//    if (!scores.empty())
//        score = this->scores[0];
//    else
//        score = -1;
//    cout << "SCORE: " << score << endl;
//    return score;
//}

}

