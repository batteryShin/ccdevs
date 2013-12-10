#include "AnnMatch.h"

CAnnMatch::CAnnMatch(void)
{
	m_kdTree = NULL;
	m_refpts = NULL;
}

CAnnMatch::~CAnnMatch(void)
{
	Clear();
	annClose();
}

void CAnnMatch::Clear()
{
	if( m_refpts )
	{
		annDeallocPts(m_refpts);
		m_refpts = NULL;
	}

	if( m_kdTree ) 
	{
		delete m_kdTree;
		m_kdTree = NULL;
	}

	m_match.clear();
}

void CAnnMatch::Construct(std::vector<FeatureVector>& fv)
{
	Clear();

	//-------------------------------------------------------------------------
	// ANN Initialization
	//-------------------------------------------------------------------------

	int num = (int)(fv.size());   // number of data points
	int dim = FEATURE_DIM;      // dimension

	// 아래 선언은 double** points; 와 완전히 동일하다.
	m_refpts = annAllocPts(num, dim); // allocate data points

	std::vector<FeatureVector>::iterator iter;
	FeatureVector zf;

	int i;
	for( i = 0, iter = fv.begin() ; iter != fv.end() ; ++iter, i++ )
	{
		zf = *iter;
		memcpy(m_refpts[i], iter->v, sizeof(double)*dim);
	}

	// build a Kd-Tree
	m_kdTree = new ANNkd_tree(m_refpts, num, dim);
}

void CAnnMatch::Match(std::vector<FeatureVector>& fv)
{
	//-------------------------------------------------------------------------
	// KdTree Search
	//-------------------------------------------------------------------------

	int k = 2;                  // number of nearest neighbors
	int num = (int)fv.size();   // number of data points
	int dim = FEATURE_DIM;      // dimension
	double eps = 0;             // error bound

    if( !m_kdTree ) {
        return;
    }

	// 아래 선언은 double* point; 와 완전히 동일하다. 
	ANNpoint point = annAllocPt(dim);       // allocate query point
	ANNidxArray  nnidx = new ANNidx[k];     // allocate near neigh indices
	ANNdistArray dists = new ANNdist[k];    // allocate near neighbor dists

	// Initializes
	m_match.clear();

	std::vector<FeatureVector>::iterator iter;

	for( iter = fv.begin() ; iter != fv.end() ; ++iter )
	{
		memcpy(point, iter->v, sizeof(double)*dim);

		m_kdTree->annkSearch(       // search
			point,                  // query point
			k,                      // number of near neighbors
			nnidx,                  // nearest neighbors (returned)
			dists,                  // distance (returned)
			eps);                   // error bound
/*
		if( dists[0] > 0.01 )
		{
			m_match.push_back(-1);
		}
		else
		{
*/			if(dists[1]/dists[0] < 2.f)
				m_match.push_back(-1);
			else
				m_match.push_back(nnidx[0]);
//		}

	}

	delete [] nnidx;
	delete [] dists;
	annDeallocPt(point);
}

void CAnnMatch::Construct(std::vector<SURFfV>& fv)
{
	Clear();

	//-------------------------------------------------------------------------
	// ANN Initialization
	//-------------------------------------------------------------------------

	int num = (int)fv.size();   // number of data points
	int dim = SURF_FEATURE_DIM;      // dimension

	// 아래 선언은 double** points; 와 완전히 동일하다.
	m_refpts = annAllocPts(num, dim); // allocate data points

    std::vector<SURFfV>::iterator iter;
	SURFfV surf;

	int i = 0;
	for( i = 0, iter = fv.begin() ; iter != fv.end() ; ++iter, ++i )
	{
		surf = *iter;
		memcpy(m_refpts[i], iter->v, sizeof(double)*dim);
	}

	// build a Kd-Tree
	m_kdTree = new ANNkd_tree(m_refpts, num, dim);
}

void CAnnMatch::Match(std::vector<SURFfV>& fv)
{
	//-------------------------------------------------------------------------
	// KdTree Search
	//-------------------------------------------------------------------------

	int k = 2;                  // number of nearest neighbors
	int num = (int)fv.size();   // number of data points
	int dim = SURF_FEATURE_DIM;      // dimension
	double eps = 0;             // error bound

    if( !m_kdTree ) {
        return;
    }

	// 아래 선언은 double* point; 와 완전히 동일하다. 
	ANNpoint point = annAllocPt(dim);       // allocate query point
	ANNidxArray  nnidx = new ANNidx[k];     // allocate near neigh indices
	ANNdistArray dists = new ANNdist[k];    // allocate near neighbor dists

	// Initializes
	m_match.clear();

    std::vector<SURFfV>::iterator iter;

	for( iter = fv.begin() ; iter != fv.end() ; ++iter )
	{
		memcpy(point, iter->v, sizeof(double)*dim);

		m_kdTree->annkSearch(       // search
			point,                  // query point
			k,                      // number of near neighbors
			nnidx,                  // nearest neighbors (returned)
			dists,                  // distance (returned)
			eps);                   // error bound

// 		if( dists[0] > 4 )
//		{
//			m_match.push_back(-1);
//		}
//		else
//		{
			if(dists[1]/dists[0] < 1.6f)
				m_match.push_back(-1);
			else
				m_match.push_back(nnidx[0]);
//		}
	}

	delete [] nnidx;
	delete [] dists;
	annDeallocPt(point);
}
