// $Id$
#include "Dictionary.h"
#include "FactorTypeSet.h"

size_t Dictionary::s_index = 0;

Dictionary::Dictionary(size_t noScoreComponent)
	:m_noScoreComponent(noScoreComponent)
	,m_index(s_index++)
{
}

Dictionary::~Dictionary() {}

void Dictionary::CleanUp() {}
