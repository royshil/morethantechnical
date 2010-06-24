
namespace VirtualSurgeon {

	enum face_warp_method {FACE_WARP_RIGID,FACE_WARP_AFFINE};

	class VirtualSurgeon_Warp {
	public:
		VirtualSurgeon_Warp(VIRTUAL_SURGEON_PARAMS& _p):p(_p) {};

		void doWarp(VirtualSurgeonFaceData& model_p, 
					 vector<Point2d> face_points,
					 vector<Point2d> model_points,
					 Mat& model_im,
					 Mat& model_mask,
					 Mat& im,
					 Mat& im_mask);

	private:
		VIRTUAL_SURGEON_PARAMS p;

		void face_warp(vector<Mat* >& imgs,
			   vector<Point2d>& points1,
			   vector<Point2d>& points2, 
			   Size im2_size, 
			   face_warp_method method,
			   double alpha);
		void face_warp_read_points(std::string& str, Size im_size, vector<Point2d>& points);

		//TODO: memory release...
		float* rigid_px;
		float* rigid_py;
		float* rigid_ww;
		float** rigid_w;
		int rigid_w_sz;

		void Affine_Precompute(vector<Point2d> controlPoints, vector<Point2d>& mesh);
		void Affine_doWarp(vector<Point2d> newCtrlPoints, vector<Point2d>& newMesh);
		void Rigid_doWarp(vector<Point2d> controlPoints, vector<Point2d> newCtrlPoints, vector<Point2d>& newMesh);
		void Rigid_Precompute(vector<Point2d> controlPoints, vector<Point2d>& mesh);
		void Rigid_release();
		void Rigid_doAll(vector<Point2d> controlPoints, vector<Point2d> newCtrlPoints, vector<Point2d>& newMesh);
	};
}//ns