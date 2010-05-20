void Affine_Precompute(vector<Point2d> controlPoints, vector<Point2d>& mesh);
void Affine_doWarp(vector<Point2d> newCtrlPoints, vector<Point2d>& newMesh);
void Rigid_doWarp(vector<Point2d> controlPoints, vector<Point2d> newCtrlPoints, vector<Point2d>& newMesh);
void Rigid_Precompute(vector<Point2d> controlPoints, vector<Point2d>& mesh);
void Rigid_release();
void Rigid_doAll(vector<Point2d> controlPoints, vector<Point2d> newCtrlPoints, vector<Point2d>& newMesh);