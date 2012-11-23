#include "AsiyaScorer.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <climits>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "util/check.hh"
#include "Reference.h"
#include "Util.h"
#include "Vocabulary.h"
#include "Util.h"
#include "ScoreData.h"
#include "Data.h"
#include "FileStream.h"

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
    : StatisticsBasedScorer("Asiya", config),
      m_ref_length_type(CLOSEST),
      m_source_file("./temp/trans.txt"),         //Fake source file
      m_config_file("./temp/Asiya.config"),
      m_translation_file("./temp/trans.txt"),
      m_reference_file("./temp/ref.txt_")
{
    cout << "asiya constructor." << endl;
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
    float logasiya = 0.0;
    // reflength divided by test length
    //todo. read it from the scores file
    return exp(logasiya);
}

void AsiyaScorer::addCandidateSentence(const string& sid, const string& sentence )
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

void AsiyaScorer::doScoring()
{
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
    std::string searchedWord = "BLEU";

    while(std::getline(ss,line))
    {
        if (line.find(searchedWord) != std::string::npos)
        {
            std::stringstream ssLine(line);
            std::string set, doc, seg, metric, score;
            ssLine >> set >> doc >> seg >> metric >> score;
            double temp = atof(score.c_str());
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
        ofstream file;
        file.open((filename.c_str()));
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
    std::stringstream ss;
    ss << "./temp/trans.txt";
    string candfilename = ss.str();
    ofstream candfile;
    candfile.open( candfilename.c_str() );
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
    // ~/perl ../../../operador/asiya/bin/Asiya.pl -eval single -m BLEU ./Asiya.config
    string perl_location = "perl ";
    string asiya_location = " ~/../operador/asiya//bin/Asiya.pl ";
    string params = " -eval single -m BLEU -g sys -s smatrix";
    string run_command;
    run_command = perl_location + asiya_location + " " +  m_config_file + " " + params;
    //stderr->stdout
    run_command += " 2>&1";

    string result = execCommand(run_command);
    readscores(result);
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

float AsiyaScorer::score(const candidates_t& candidates)
{
    doScoring();
    if (!scores.empty())
        return this->scores[0];
    else
        return -1;
    cout << "SCORE:" << this->scores[0];
}

}

