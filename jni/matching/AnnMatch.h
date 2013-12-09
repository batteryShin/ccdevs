#ifndef _ANN_MATCH_H
#define _ANN_MATCH_H

#include "kpmtypes.h"

#include "ANN/ANN.h"

class CAnnMatch
{
public:
	CAnnMatch(void);
	~CAnnMatch(void);

public:
	ANNpointArray m_refpts;
	ANNkd_tree* m_kdTree;
	std::vector<int> m_match;

public:
	void Clear();
	void Construct(std::vector<FeatureVector>& fv);
	void Match(std::vector<FeatureVector>& fv);

	void Construct(std::vector<SURFfV>& fv);
	void Match(std::vector<SURFfV>& fv);

	std::vector<int> GetMatch() { return m_match; }
};

#endif
