#include "Matcher.h"

Matcher::Matcher(IplImage *pImg, CvRect rgn):m_curObj(0),m_numObj(0)
{
    int i, j;

    if( m_curObj!=OBJECTNUM-1 )
        m_numObj++;

    m_curObj = (m_curObj+1)%OBJECTNUM;

    m_pts[m_curObj][0].x = rgn.x;                m_pts[m_curObj][0].y = rgn.y;
    m_pts[m_curObj][1].x = rgn.x+rgn.width;      m_pts[m_curObj][1].y = rgn.y;
    m_pts[m_curObj][2].x = rgn.x+rgn.width;      m_pts[m_curObj][2].y = rgn.y+rgn.height;
    m_pts[m_curObj][3].x = rgn.x;                m_pts[m_curObj][3].y = rgn.y+rgn.height;

    // refer image registration..
    m_surf = new CKeyPointMatch();
    m_preH = cvCreateMat(3,3,CV_32FC1);
    m_surf->SetReferImageSURF(m_pts[m_curObj], pImg, m_curObj);
}

Matcher::~Matcher()
{
    cvReleaseMat(&m_preH);
    delete m_surf;
}

CvRect Matcher::match(IplImage* pImg) {
    if( m_numObj>0 )
    {
        int n, i, j;
        cvSetIdentity(m_preH);
        for(n=0; n<m_numObj; n++) {
            m_surf->SetQueryImageSURF(pImg, n);
            m_surf->RemoveOutlier(m_preH, n);


            CVec2d arrSrcPts[4], arrDstPts[4];
            for(i=0; i<4; i++) {
                arrSrcPts[i].x = m_pts[m_curObj][i].x;
                arrSrcPts[i].y = m_pts[m_curObj][i].y;
            }

            m_surf->m_Homography[n].transform(arrSrcPts, arrDstPts);
/*
            ARParam param = m_ARToolKitProj.GetCParam();

            FindGLModelView(pts, &(m_surf->m_Homography[n]), prevbox, &tmpHomography);
            m_surf->m_Homography[n].computePose(true, param.mat[0][0], param.mat[1][1]);

            m_PlanarPose[ 0] = m_surf->m_Homography[n].m_matTransformation[0][0];
            m_PlanarPose[ 1] = m_surf->m_Homography[n].m_matTransformation[1][0];
            m_PlanarPose[ 2] = m_surf->m_Homography[n].m_matTransformation[2][0];
            m_PlanarPose[ 3] = m_surf->m_Homography[n].m_matTransformation[3][0];
            m_PlanarPose[ 4] = m_surf->m_Homography[n].m_matTransformation[0][1];
            m_PlanarPose[ 5] = m_surf->m_Homography[n].m_matTransformation[1][1];
            m_PlanarPose[ 6] = m_surf->m_Homography[n].m_matTransformation[2][1];
            m_PlanarPose[ 7] = m_surf->m_Homography[n].m_matTransformation[3][1];
            m_PlanarPose[ 8] = m_surf->m_Homography[n].m_matTransformation[0][2];
            m_PlanarPose[ 9] = m_surf->m_Homography[n].m_matTransformation[1][2];
            m_PlanarPose[10] = m_surf->m_Homography[n].m_matTransformation[2][2];
            m_PlanarPose[11] = m_surf->m_Homography[n].m_matTransformation[3][2];
            m_PlanarPose[12] = m_surf->m_Homography[n].m_matTransformation[0][3];
            m_PlanarPose[13] = m_surf->m_Homography[n].m_matTransformation[1][3];
            m_PlanarPose[14] = m_surf->m_Homography[n].m_matTransformation[2][3];
            m_PlanarPose[15] = m_surf->m_Homography[n].m_matTransformation[3][3];
*/
//            m_surf->DrawOutput(m_dispDib, box, n+1);
//
            return cvRect( arrDstPts[0].x, arrDstPts[0].y, 
                            arrDstPts[1].x-arrDstPts[0].x, arrDstPts[2].y-arrDstPts[1].y );
        }

    }

}

/*
inline double Matcher::GetDist(int x1, int y1, int x2, int y2)
{
    return sqrt( (double)(x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) );
}

inline double Matcher::GetAngl(int x1, int y1, int x2, int y2)
{
    double rad = atan2( (double)y1-y2, (double)x2-x1 );
    return (rad*180/3.141592);
}
*/
