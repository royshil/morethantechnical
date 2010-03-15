/**
 * License...
 *
 *	Main program
 *
 *
 */
#include "stdafx.h"
#include "main.h"

/**
 * OpenGL thread main function
 */
void start_opengl(LPVOID p) {
	frames[0].copyTo(backPxls);
	cvtColor(backPxls,backPxls,CV_BGR2RGB);
	cvFlip(&cvMat(frames[0].rows,frames[0].cols,CV_8UC3,backPxls.data),0,0);

	start_opengl_with_stereo(0,0);
}

/**
 * main program
 */
int main(int i, char** c)
{
    //VideoCapture cap("record3.wmv"); // open the default camera
	VideoCapture cap("record3.wmv");
    if(!cap.isOpened())  // check if we succeeded
        return -1;

	vector<double> rv,tv;

    Mat gray,prev;
    Mat frame;
	Mat image;
	//Mat frames[9];
	//vector<Point2f> framePoints[9];
	int captured_frames = 0;

    namedWindow("aaa",1);
	vector<Point2f> points1,points2;
	vector<uchar> status;
	vector<float> err;
	bool needToInit = true,firstAcquired = false;
	long counter = 0l;
	vector<int> whoIsBad;
	vector<Point2f> tmp_pts;
	vector<uchar> global_status(200,1), snapshot_global_status(200);
	bool frameByFrame = false;

	HANDLE threadHandle;

	/**
	 * the number of good points
	 */
	int nz = 0;

	//reprojection error
	double reproj_err = 0;


    for(;;)
    {
        cap >> frame; // get a new frame from camera
		if(frame.rows == 0) break; //video done
		frame.copyTo(image);
		cvtColor(frame,gray,CV_BGR2GRAY);
		//blur(gray,gray,Size(7,7));
		//equalizeHist(gray,gray);
		
		counter++; //frame counter

		if(needToInit) {
			goodFeaturesToTrack(gray,points1,200,0.01f,2.0);
			whoIsBad.clear();
			points2.clear();

			needToInit = false;
		} else {
			//if(!firstAcquired) {
				calcOpticalFlowPyrLK(prev,gray,points1,points2,status,err);

				unsigned int k=0,bad=0;
				//whoIsBad.clear();
				//points1.clear();
				for(unsigned int i=0;i<points2.size();i++)
				{
					//compact points
					if(status[i]) {
						//points1.push_back(points2[i]);
						k++;
						points1[i] = points2[i];
					} 
					else if(captured_frames < 2) {
						global_status[i] = 0;	//remove point only if still initializing

					//	points1[i].x = -1.0f;
					//	points1[i].y = -1.0f;
					//	bad++;
					//	whoIsBad.push_back(i);
					}

					//show good features
					if(global_status[i] == 1) {
						circle(image,points2[i],2,CV_RGB(0,255,0),CV_FILLED);
					} /*else {
						circle(image,points2[i],2,CV_RGB(255,0,0),CV_FILLED);
					}*/
				}

				//if(!firstAcquired && bad > 20) {
				//	printf("add new points\n");
				//	//too many bads, find some nicer points and add them to the vector
				//	goodFeaturesToTrack(gray,tmp_pts,whoIsBad.size(),0.01f,2.0);
				//	for(unsigned int i=0;i<whoIsBad.size();i++) {
				//		points1[whoIsBad[i]] = tmp_pts[i];
				//	}
				//}

				//points1.resize(k);
				//printf("%d good points, %d bad points\n",,bad);
				if(captured_frames < 2 )
					nz = countNonZero(Mat(global_status));
				else 
					nz = countNonZero(Mat(snapshot_global_status));

				stringstream st; 
				st << "Count: " << counter << ", " << nz << " good points";
				putText(image,st.str(),Point(10,15),FONT_HERSHEY_PLAIN,1,Scalar(0,255,0));
			//} //else {
			//	//insert 
			//}

			
		}

		gray.copyTo(prev);

		if(captured_frames >= 2) {
			//assume features triangulated - try to position new camera in 3D

			//keep only corresponding features
			points[2].clear();
			int p1sz = points1.size();
			if(nz == points1projMF.rows) {
				//no "bad" points
				if(points[2].size() != p1sz) {
					points[2] = vector<Point2d>(p1sz);
					snapshot_global_status.resize(p1sz,1);
				}
				Mat(points1).convertTo(Mat(points[2]),CV_64FC2);
			} else {
				//there are some bad points, need to "discard a few"
				vector<Point3d> new3DPoints;

				for(int ii=0;ii<p1sz;ii++) {
					if(snapshot_global_status[ii] == 1) {
						points[2].push_back(Point2d(points1[ii].x,points1[ii].y));
						new3DPoints.push_back(points1Proj[ii]);
					} else {
						//"invisible" points (bad points) will be kept for future relocalizing
						//but right now they are removed to allow good pose estimation
						points1Proj_invisible.push_back(points1Proj[ii]);
					}
				}
				snapshot_global_status.resize(nz);
				snapshot_global_status.assign(nz,1);

				points1.resize(points[2].size());
				Mat(points[2]).convertTo(Mat(points1),CV_32FC2);

				points1Proj = new3DPoints;

				points1ProjF.resize(points1Proj.size());
				points1projMF = Mat(points1ProjF);
				Mat(points1Proj).convertTo(points1projMF,CV_32FC3);
			}

			//estimate camera position
			findExtrinsics(points[2],rv,tv);

			//if(counter % 5 == 0) 
			{
				//once every 5 frames:

				//try to reproject some of the "invisible" features back into the tracking
				if(reprojectInivibles(points1,rv,tv)) {
					//some points came back to tracking..
					snapshot_global_status.resize(points1.size(),1);
				} else {
					//no points came back, so
					//estimate which image points are not aligned well with
					//3D points, and remove them
					reproj_err = keepGood2D3DMatch(points[2],rv,tv,snapshot_global_status);
				}
			}

			drawReprojectedOnImage(image,rv,tv,points[2],snapshot_global_status);

			stringstream st; st << "Reprojection error: " << reproj_err;
			putText(image,st.str(),Point(10,image.rows - 15),FONT_HERSHEY_PLAIN,1,Scalar(0,255,0));

			if(counter % 20 == 0) {
				//every 20 frames, try to triangulate more features using this view
				//and the view from 20 frames ago
				//TODO: do this.
			}

			cams[2] = tv;
			rots[2] = rv;
			frame.copyTo(frames[2]);

			//update OpenGL scene
			loadFrame(2);
		}

		imshow("aaa",image);

		char c = -1;
		if(frameByFrame) 
			c = waitKey(0);
		else
			c = waitKey(30);
		if(c == 27) {
			break;
		} else if(c == ' ') {
			if(captured_frames < 2) {
				int i = captured_frames;
				frame.copyTo(frames[i]);

				points[i] = vector<Point2d>(200);
				Mat(points1).convertTo(Mat(points[i]),CV_64FC2);
				printf("saved %d frame\n",i);
				firstAcquired = true;
				captured_frames++;

				if(captured_frames == 2) {
					//capured 2 initialization frames - try to triangulate features

					//prune points to get only feature points that correspond between
					//the 2 first keyframes
					for(int pi=0;pi<2;pi++) {
						vector<Point2d> _tmp = points[pi];
						points[pi].clear();
						for(int ii=0;ii<200;ii++) {
							if(global_status[ii] == 1) {
								points[pi].push_back(_tmp[ii]);
							}
						}
					}

					//triangulate points
					stereoInit();

					//keep only points that were triangulated successfully
					for(int pi=0;pi<2;pi++) {
						vector<Point2d> _tmp = points[pi];
						points[pi].clear();
						for(int ii=0;ii<nz;ii++) {
							if(tri_status[ii] == 1) {
								points[pi].push_back(_tmp[ii]);
							}
						}
					}

					//now track only the good & triangulated points
					int numPts = points[1].size();
					points1 = vector<Point2f>(numPts);
					points2 = vector<Point2f>(numPts);
					Mat(points[1]).convertTo(Mat(points1),CV_32FC2);
					global_status = vector<uchar>(numPts,1);
					snapshot_global_status = global_status;

					//estimate 2nd keyframe extrinsics to have a good starting point
					//of the camera position
					findExtrinsics(points[0],rv,tv);
					findExtrinsics(points[1],rv,tv);


					//Create a mutex to goveren the frame data sharing
					ghMutex = CreateMutex( 
									NULL,              // default security attributes
									FALSE,             // initially not owned
									NULL);  

					//start the OpenGL scene (in it's own thread)
					DWORD tID;
					threadHandle = CreateThread(0,0,(PTHREAD_START_ROUTINE)start_opengl,0,0,&tID);
					printf("Created opengl thread %d\n",tID);
				}
			}
		} else if(c=='r') {
			needToInit = true;
			printf("re-init\n");
		} else if(c == 'p') {
			frameByFrame = !frameByFrame;
			printf("pause\n");
		}
    }

	running = false;
	if(threadHandle != NULL)
		WaitForSingleObject(threadHandle,1000);

	CloseHandle(threadHandle);

    return 0;
}