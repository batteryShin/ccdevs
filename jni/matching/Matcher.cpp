#include "Matcher.h"

Matcher::Matcher(IplImage *pImg, float* pts):m_curObj(0),m_numObj(0)
{
    int i, j;

    if( m_curObj!=OBJECTNUM-1 )
        m_numObj++;

    m_curObj = (m_curObj+1)%OBJECTNUM - 1;

    for(int i=0; i<4; i++) {
        m_pts[m_curObj][i].x = pts[2*i];
        m_pts[m_curObj][i].y = pts[2*i+1];

        m_fpts[m_curObj][2*i] = pts[2*i];
        m_fpts[m_curObj][2*i+1] = pts[2*i+1];
    }

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

float* Matcher::match(IplImage* pImg) {
    if( m_numObj>0 )
    {
        int n, i, j;
        cvSetIdentity(m_preH);
        for(n=0; n<m_numObj; n++) {
            m_surf->SetQueryImageSURF(pImg, n);
            m_surf->RemoveOutlier(m_preH, n);


            point2i src_pts[4], dst_pts[4];
            for(i=0; i<4; i++) {
                src_pts[i].x = m_pts[m_curObj][i].x;
                src_pts[i].y = m_pts[m_curObj][i].y;
            }

            CHomography tmpHomography;
            FindModelView(src_pts, &(m_surf->m_Homography[n]), dst_pts, &tmpHomography);
            
            for(int i=0; i<3; i++)
                for(int j=0; j<3; j++)
                    m_surf->m_Homography[n].m_dData[i][j] = tmpHomography.m_dData[i][j];

            m_surf->DrawOutput(pImg,dst_pts,n+1);


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
            for(int i=0; i<4; i++) {
                m_fpts[n][2*i] = dst_pts[i].x;
                m_fpts[n][2*i+1] = dst_pts[i].y;
            }

            return m_fpts[n];
        }

    }

}

void Matcher::FindModelView(point2i* srcPts, CHomography* srcH, point2i* box, CHomography* dstH)
{
    //-------------------------------------------------------------------------
    // From four corner points, find the modelview matrix of OpenGL
    //-------------------------------------------------------------------------
    CVec2d arrSrcPoints[4], arrDstPoints[4];

    arrSrcPoints[0].x = -80;    arrSrcPoints[0].y = -80;
    arrSrcPoints[1].x =  80;    arrSrcPoints[1].y = -80;
    arrSrcPoints[2].x =  80;    arrSrcPoints[2].y =  80;
    arrSrcPoints[3].x = -80;    arrSrcPoints[3].y =  80;

    CvMat* Hmat = cvCreateMat(3, 3, CV_32FC1);
    CvMat* Hinv = cvCreateMat(3, 3, CV_32FC1);

    int i, j;
    for(i=0; i<3; i++)
        for(j=0; j<3; j++)
            cvmSet(Hmat, i, j, srcH->m_dData[i][j]);

    box[0] = point2i(srcPts[0].x, srcPts[0].y); box[1] = point2i(srcPts[1].x, srcPts[1].y);
    box[2] = point2i(srcPts[2].x, srcPts[2].y); box[3] = point2i(srcPts[3].x, srcPts[3].y);

    float ab[3] = {0, 0, 0}, ax[3] = {0, 0, 0};
    CvMat bb = cvMat(3, 1, CV_32FC1, ab);
    CvMat xx = cvMat(3, 1, CV_32FC1, ax);
    cvInvert(Hmat, Hinv);

    for( i = 0 ; i < 4 ; i++ )
    {
        ab[0] = (float)box[i].x;
        ab[1] = (float)box[i].y;
        ab[2] = 1.f;
        cvMatMul(Hinv, &bb, &xx);

        box[i].x = (int)(ax[0]/ax[2] + 0.5f);
        box[i].y = (int)(ax[1]/ax[2] + 0.5f);
    }
    cvReleaseMat(&Hmat);
    cvReleaseMat(&Hinv);

    for(i=0; i<4; i++)
    {
        arrDstPoints[i].x = box[i].x;
        arrDstPoints[i].y = box[i].y;
    }

    bool fixedFocal = true;
//    ARParam param = m_ARToolKitProj.GetCParam();
    double fu = 1.0; //param.mat[0][0];
    double fv = 1.0; //param.mat[1][1];
    
    dstH->compute(arrSrcPoints, arrDstPoints, 4);
    dstH->computePose(fixedFocal, fu, fv);
}

float* Matcher::getSrcPts() {
    return m_fpts[m_curObj];
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
