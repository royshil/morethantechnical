#include "stdafx.h"

Mat weights;

typedef unsigned int uint;

void Affine_Precompute(vector<Point2d> controlPoints, vector<Point2d>& mesh) {
	uint i, j;
	double epsilon = 0.000001f;
	double alpha = 1;

	uint cpSz = controlPoints.size();
	uint mshSz = mesh.size();

	//Matrix<double> p ( cpSz, 3 );
	//Matrix<double> tp;
	//Matrix<double> point ( 1, 3 );
	//Matrix<double> w ( cpSz, cpSz );
	//Matrix<double> ans;
	Mat p(cpSz,3,CV_64FC1);
	Mat tp;
	Mat point(1,3,CV_64FC1);
	Mat w(cpSz,cpSz,CV_64FC1);
	double _ansD[100] = {0};
	Mat ans(1,cpSz,CV_64FC1,_ansD);

	//weights = Matrix<double> ( mshSz, cpSz );
	//weights.zero ( );
	//w.zero ( );
	weights = Mat::zeros(mshSz,cpSz,CV_64FC1);
	w.setTo(Scalar(0));

	for ( j = 0; j < cpSz; j++ )
	{
		//Vertex *v = &(controlPoints[j]);
		Point2d v = controlPoints[j];
		double* _pPtr = p.ptr<double>(j);
		//p [ j ] [ 0 ] = v->getX ( );
		//p [ j ] [ 1 ] = v->getY ( );
		//p [ j ] [ 2 ] = 1;
		//_pPtr[ 0 ] = v->getX ( );
		//_pPtr[ 1 ] = v->getY ( );
		//_pPtr[ 2 ] = 1;
		_pPtr[0] = v.x;
		_pPtr[1] = v.y;
		_pPtr[2] = 1;
	}
	//tp = p.transpose ( );
	tp = p.t();

	for ( i = 0; i < mshSz; i++ )
	{
		//MathVector2 mv = mesh[i];
		Point2d mv = mesh[i];
		double* _weightsPtr = weights.ptr<double>(i);
		for ( j = 0; j < cpSz; j++ )
		{
			//MathVector2 cmv ( controlPoints[j].getVert ( ) );
			Point2d cmv = controlPoints[j];

			Point2d _tmp(cmv.x-mv.x, cmv.y-mv.y);
			//double temp = ( cmv - mv ).lengthSq ( );
			double temp = _tmp.x*_tmp.x + _tmp.y*_tmp.y;
			if ( temp < epsilon )
			{
				//weights [ i ] [ j ] = 1;
				_weightsPtr[j] = 1;
				break;
			}
			//w [ j ] [ j ] = 1 / pow ( temp, alpha );
			if(alpha != 1.0)
				w.ptr<double>(j)[j] = 1 / pow ( temp, alpha );
			else 
				w.ptr<double>(j)[j] = 1 / temp;
		}	

		if ( j < cpSz )
		{
			continue;
		}

		//point [ 0 ] [ 0 ] = mv [ 0 ];
		//point [ 0 ] [ 1 ] = mv [ 1 ];
		//point [ 0 ] [ 2 ] = 1;
		double* _ptPtr = point.ptr<double>(0);
		_ptPtr[ 0 ] = mv.x;
		_ptPtr[ 1 ] = mv.y;
		_ptPtr[ 2 ] = 1;

		//Matrix<double> ident ( 3, 3 );
		//ident.identity ( );
		double _identD[9] = {1,0,0,0,1,0,0,0,1};
		Mat ident(3,3,CV_64FC1,_identD);
		Mat _a = tp * w * p;
		ident = _a.inv();
		//( tp * w * p ).solve ( ident );

		ans = point * ident * tp * w;

		for ( j = 0; j < cpSz; j++ )
		{
			//weights [ i ] [ j ] = ans [ 0 ] [ j ];
			//_weightsPtr[j] = ((double*)(ans.data))[j];
			_weightsPtr[j] = _ansD[j];
		}
	}
}

void Affine_doWarp(vector<Point2d> newCtrlPoints, vector<Point2d>& newMesh) {
	uint i, j;

	uint mSz = newMesh.size();
	uint ctrlSz = newCtrlPoints.size();

	for ( i = 0; i < mSz; i++ )
	{
		//MathVector2 newV ( 0, 0 );
		Point2d newV(0,0);
		double* _weightsPtr = weights.ptr<double>(i);
		double _wPt[11];
		memcpy(_wPt,_weightsPtr,sizeof(double)*11);

		for ( j = 0; j < ctrlSz; j++ )
		{
			//Vertex *v = &(newCtrlPoints[j]);
			Point2d v = newCtrlPoints[j];
			//MathVector2 mv ( &((*v) [ 0 ]) );
			//newV += mv * _weightsPtr[ j ];
			newV.x += v.x * _weightsPtr[j];
			newV.y += v.y * _weightsPtr[j];
		}

		newMesh[i] = newV;
	}
}


//TODO: memory release...
float* rigid_px;
float* rigid_py;
float* rigid_ww;
float** rigid_w;
int rigid_w_sz;

void Rigid_Precompute(vector<Point2d> controlPoints, vector<Point2d>& mesh) 
{
	uint i, j;
	float alpha = 1.0f;

	uint ctrlSz = controlPoints.size();
	uint mSz = mesh.size();

	rigid_px = new float [ mSz ];
	rigid_py = new float [ mSz ];
	rigid_ww = new float [ mSz ];
	rigid_w = new float * [ mSz ];
	rigid_w_sz = mSz;
	for ( i = 0; i < mSz; i++ )
	{
		rigid_w [ i ] = new float [ ctrlSz ];
	}

	float* px = rigid_px, *py = rigid_py, *ww = rigid_ww;
	float** w = rigid_w;

	for ( i = 0; i < mSz; i++ )
	{
		//MathVector2 mvx = baseObject->getVertex ( i );
		Point2f mvx = mesh[i];

		px [ i ] = 0;
		py [ i ] = 0;
		ww [ i ] = 0;
		for ( j = 0; j < ctrlSz; j++ )
		{
			//Vertex *v = controlMesh->getVertex ( j );
			//MathVector2 mv ( &((*v) [ 0 ]) );
			Point2f mv = controlPoints[j];

			float dx = mv.x - mvx.x;
			float dy = mv.y - mvx.y;
			//w [ i ] [ j ] = ( mv - mvx ).lengthSq ( );
			w [ i ] [ j ] = dx*dx + dy*dy;

			if(alpha != 1.0f) {
				w [ i ] [ j ] = 1.0f / pow ( w [ i ] [ j ], alpha );
			} else {
				w [ i ] [ j ] = 1.0f / w [ i ] [ j ];
			}
			px [ i ] += w [ i ] [ j ] * mv.x;
			py [ i ] += w [ i ] [ j ] * mv.y;
			ww [ i ] += w [ i ] [ j ];
		}
	}
}

void Rigid_doWarp(vector<Point2d> controlPoints, vector<Point2d> newCtrlPoints, vector<Point2d>& newMesh)
{

	uint mSz = newMesh.size();
	uint ctrlSz = newCtrlPoints.size();

	uint i, j;
	float qx, qy, qxpx, qxpy, qypx, qypy;
	float c, s, tx, ty;

	float* px = rigid_px, *py = rigid_py, *ww = rigid_ww;
	float** w = rigid_w;

	for ( i = 0; i < mSz; i++ )
	{
		//MathVector2 x = baseObject->getVertex ( i );
		//MathVector2 newV ( 0, 0 );
		Point2f x = newMesh[i];
		Point2f newV(0,0);

		qx = qy = qxpx = qxpy = qypx = qypy = 0;

		for ( j = 0; j < ctrlSz; j++ )
		{
			//Vertex *v = deformedControlMesh->getVertex ( j );
			//MathVector2 dmv ( &((*v) [ 0 ]) );
			Point2f dmv = newCtrlPoints[j];
			//v = controlMesh->getVertex ( j );
			//MathVector2 mv ( &((*v) [ 0 ]) );
			Point2f mv = controlPoints[j];

			//qx += w [ i ] [ j ] * dmv [ 0 ];
			//qy += w [ i ] [ j ] * dmv [ 1 ];
			//qxpx += w [ i ] [ j ] * dmv [ 0 ] * mv [ 0 ];
			//qxpy += w [ i ] [ j ] * dmv [ 0 ] * mv [ 1 ];
			//qypx += w [ i ] [ j ] * dmv [ 1 ] * mv [ 0 ];
			//qypy += w [ i ] [ j ] * dmv.y * mv.y;
			qx += w [ i ] [ j ] * dmv.x;
			qy += w [ i ] [ j ] * dmv.y;
			qxpx += w [ i ] [ j ] * dmv.x * mv.x;
			qxpy += w [ i ] [ j ] * dmv.x * mv.y;
			qypx += w [ i ] [ j ] * dmv.y * mv.x;
			qypy += w [ i ] [ j ] * dmv.y * mv.y;
		}

		c = -px [ i ] * qx - py [ i ] * qy + ( qxpx + qypy ) * ww [ i ];
		s = py [ i ] * qx - px [ i ] * qy + ( qypx - qxpy ) * ww [ i ];
		float m = sqrt ( c * c + s * s );
		c /= m;
		s /= m;
		tx = ( s * py [ i ] - c * px [ i ] + qx ) / ww [ i ];
		ty = ( - c * py [ i ] - s * px [ i ] + qy ) / ww [ i ];

		newV.x = c * x.x - s * x.y + tx;
		newV.y = s * x.x + c * x.y + ty;

		//deformedObject->setVertex ( i, newV );
		newMesh[i] = newV;
	}

	delete[] rigid_px;
	delete[] rigid_py;
	delete[] rigid_ww;
	for(int i=0;i<rigid_w_sz;i++) delete[] rigid_w[i];
	delete[] rigid_w;
}

void Rigid_release() {
	
}

void Rigid_doAll(vector<Point2d> controlPoints, vector<Point2d> newCtrlPoints, vector<Point2d>& newMesh) {
	uint i, j;
	float alpha = 1.0f;

	uint ctrlSz = controlPoints.size();
	uint mSz = newMesh.size();

	float* px;
	float* py;
	float* ww;
	float** w;

	px = new float [ mSz ];
	py = new float [ mSz ];
	ww = new float [ mSz ];
	//vector<float> px(mSz);
	//vector<float> py(mSz);
	//vector<float> ww(mSz);

	w = new float * [ mSz ];
	for ( i = 0; i < mSz; i++ )
	{
		w [ i ] = new float [ ctrlSz ];
	}
	//vector<vector<float>> w(mSz,vector<float>(ctrlSz));

	float qx, qy, qxpx, qxpy, qypx, qypy;
	float c, s, tx, ty;

	for ( i = 0; i < mSz; i++ )
	{
		Point2f mvx = newMesh[i];

		px [ i ] = 0;
		py [ i ] = 0;
		ww [ i ] = 0;
		for ( j = 0; j < ctrlSz; j++ )
		{
			Point2f mv = controlPoints[j];

			float dx = mv.x - mvx.x;
			float dy = mv.y - mvx.y;
			w [ i ] [ j ] = dx*dx + dy*dy;

			if(alpha != 1.0f) {
				w [ i ] [ j ] = 1.0f / pow ( w [ i ] [ j ], alpha );
			} else {
				w [ i ] [ j ] = 1.0f / w [ i ] [ j ];
			}
			px [ i ] += w [ i ] [ j ] * mv.x;
			py [ i ] += w [ i ] [ j ] * mv.y;
			ww [ i ] += w [ i ] [ j ];
		}
	//}


	//for ( i = 0; i < mSz; i++ )
	//{
		Point2f x = mvx;
		Point2f newV(0,0);

		qx = qy = qxpx = qxpy = qypx = qypy = 0;

		for ( j = 0; j < ctrlSz; j++ )
		{
			Point2f dmv = newCtrlPoints[j];
			Point2f mv = controlPoints[j];

			qx += w [ i ] [ j ] * dmv.x;
			qy += w [ i ] [ j ] * dmv.y;
			qxpx += w [ i ] [ j ] * dmv.x * mv.x;
			qxpy += w [ i ] [ j ] * dmv.x * mv.y;
			qypx += w [ i ] [ j ] * dmv.y * mv.x;
			qypy += w [ i ] [ j ] * dmv.y * mv.y;
		}

		c = -px [ i ] * qx - py [ i ] * qy + ( qxpx + qypy ) * ww [ i ];
		s = py [ i ] * qx - px [ i ] * qy + ( qypx - qxpy ) * ww [ i ];
		float m = sqrt ( c * c + s * s );
		c /= m;
		s /= m;
		tx = ( s * py [ i ] - c * px [ i ] + qx ) / ww [ i ];
		ty = ( - c * py [ i ] - s * px [ i ] + qy ) / ww [ i ];

		newV.x = c * x.x - s * x.y + tx;
		newV.y = s * x.x + c * x.y + ty;

		newMesh[i] = newV;
	}

	delete[] px;
	delete[] py;
	delete[] ww;
	for(int i=0;i<mSz;i++) delete[] w[i];
	delete[] w;
}