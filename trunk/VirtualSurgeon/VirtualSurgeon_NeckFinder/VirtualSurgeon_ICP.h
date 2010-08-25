namespace VirtualSurgeon {
class ICP {
public:
	void doICP(Mat& X, Mat& destination);
	ICP(VIRTUAL_SURGEON_PARAMS& p):params(p),q_iter(0) {};
private:
	void ShowPoints(Mat& im, Mat& X,Scalar c = Scalar(255));
	void ShowLines(Mat& im, Mat& X, Mat& X_bar, Scalar c1 = Scalar(0,255), Scalar c2 = Scalar(0,0,255));
	void ShowQuery(Mat& destinations, Mat& query, Mat& closest);
	void findBestReansformSVD(Mat& _m, Mat& _d);
	void findBestTransform(Mat& X, Mat& X_bar);
	float flann_knn(Mat& m_destinations, Mat& m_object, vector<int>& ptpairs, vector<float>& dists = vector<float>());
	int icp_main(int argc, char** argv);

	VIRTUAL_SURGEON_PARAMS params;

	int q_iter;
	Mat q_im;
};
}//ns