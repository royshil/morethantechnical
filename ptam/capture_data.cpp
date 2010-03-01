#include "stdafx.h"
#include "utils.h"


int capture_data(int, char**)
{
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat gray,prev;
    Mat frame;
	Mat image;
	Mat frames[9];
	vector<Point2f> framePoints[9];
	int captured_frames = 0;

    namedWindow("aaa",1);
	vector<Point2f> points1,points2;
	vector<uchar> status;
	vector<float> err;
	bool needToInit = true,firstAcquired = false;
	//uchar counter;
	vector<int> whoIsBad;
	vector<Point2f> tmp_pts;

    for(;;)
    {
        cap >> frame; // get a new frame from camera
		frame.copyTo(image);
		cvtColor(frame,gray,CV_BGR2GRAY);

		if(needToInit) {
			goodFeaturesToTrack(gray,points1,200,0.01f,2.0);
			whoIsBad.clear();
			points2.clear();

			needToInit = false;
		} else {
			//if(!firstAcquired) {
				calcOpticalFlowPyrLK(prev,gray,points1,points2,status,err);

				unsigned int k=0,bad=0;
				whoIsBad.clear();
				for(unsigned int i=0;i<points2.size();i++)
				{
					//compact points
					if(status[i]) {
						//points1[k++] = points2[i];
						k++;
						points1[i] = points2[i];
						circle(image,points2[i],2,CV_RGB(0,255,0),CV_FILLED);
					} else {
						points1[i].x = -1.0f;
						points1[i].y = -1.0f;
						bad++;
						whoIsBad.push_back(i);
					}
				}

				if(!firstAcquired && bad > 20) {
					//too many bads, find some nicer points and add them to the vector
					goodFeaturesToTrack(gray,tmp_pts,whoIsBad.size(),0.01f,2.0);
					for(unsigned int i=0;i<whoIsBad.size();i++) {
						points1[whoIsBad[i]] = tmp_pts[i];
					}
				}

				//points1.resize(k);
				printf("%d good points, %d bad points\n",k,bad);
				imshow("aaa",image);
			//} //else {
			//	//insert 
			//}

			//counter++;
		}

		gray.copyTo(prev);

		char c = waitKey(30);
		if(c == 27) {
			break;
		} else if(c > '0' && c < '9') {
			int i = c-'1';
			frame.copyTo(frames[i]);
			framePoints[i] = points1;
			printf("saved first frame\n");
			firstAcquired = true;
			captured_frames++;
		} else if(c=='r') {
			needToInit = true;
			printf("re-init\n");
		}
    }

	if(firstAcquired) {
		for(int in=0;in<captured_frames;in++) {
			std::stringstream st;
			st << "frame" << (in+1) << ".png";
			imwrite(st.str(),frames[in]);
			std::stringstream st1;
			st1 << "frame" << (in+1) << ".points";
			FILE* f = fopen(st1.str().c_str(),"w");
			for(unsigned int i=0;i<framePoints[in].size();i++)
			{
				if(status[i]) {
					fprintf(f,"%.3f %.3f ",framePoints[in][i].x,framePoints[in][i].y);
				}
			}
			fclose(f);
		}
	}

    return 0;
}