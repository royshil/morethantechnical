// This is the main DLL file.
#include "stdafx.h"

#include "sys/types.h"
#include "sys/stat.h"

#include "VirtualSurgeon_CSWrapper.h"

namespace VirtualSurgeon_CSWrapper {
	void MarshalString ( System::String ^ s, string& os ) {
		using namespace Runtime::InteropServices;
		const char* chars = (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
		os = chars;
		Marshal::FreeHGlobal(IntPtr((void*)chars));
	}

	System::String^ VirtualSurgeonWrapper::GetFilename() {
		using namespace Runtime::InteropServices;
		return Marshal::PtrToStringAuto((IntPtr)((void*)(m_params->filename.c_str())));
	}

	void VirtualSurgeonWrapper::Initialize(System::String^ filename) {
		if(m_params != NULL) delete m_params;
		if(orig_im != NULL) delete orig_im;
		if(headMask != NULL) delete headMask;
		if(complete != NULL) delete complete;

		m_params = new VIRTUAL_SURGEON_PARAMS();
		m_params->InitializeDefault();

		//InitializeDefault(m_params);

		MarshalString(filename,m_params->filename);

		orig_im = new Mat();
		m_params->FaceDotComDetection(*orig_im);

		System::String^ s = filename->Substring(0, filename->LastIndexOf('.')) + ".mask.bin";
		std::string _s;
		MarshalString(s,_s);

		struct stat f__stat;
		headMask = new Mat();
		if(stat(_s.c_str(),&f__stat)==0) {
			//(imread(_s,0)).convertTo(*headMask,CV_32FC1,1.0/255.0);
			System::IO::FileStream^ fs = gcnew System::IO::FileStream(s,System::IO::FileMode::Open);
			int size = orig_im->rows * orig_im->cols * 4;
			cli::array<byte>^ barr = gcnew cli::array<byte>(size);
			fs->Read(barr,0,size);
			headMask->create(orig_im->size(),CV_32FC1);
			System::Runtime::InteropServices::Marshal::Copy(barr,0,System::IntPtr::IntPtr(headMask->data),size);
			fs->Close();
		}

		m_params->no_gui = true;
	}

	VirtualSurgeon_ParamsWrapper^ VirtualSurgeonWrapper::getParamsWrapper() {
		VirtualSurgeon_ParamsWrapper^ m_params_wrapper = gcnew VirtualSurgeon_ParamsWrapper();
		m_params_wrapper->p = m_params;
		return m_params_wrapper;
	}

	void VirtualSurgeonWrapper::ExtractHead() {
		HeadExtractor he(*m_params);
		Mat _tmp; orig_im->copyTo(_tmp);
		(*headMask) = he.ExtractHead(_tmp);
	}

	void VirtualSurgeonWrapper::Recolor() {
		Recoloring r(*m_params);
		
		//get face mask
		Mat faceMask(orig_im->size(),CV_8UC1,Scalar(0));
		double li_ri = norm(m_params->li - m_params->ri) / (double)(faceMask.cols);
		Point2d faceEllipse(
			((double)(m_params->li.x+m_params->ri.x))/2.0 - li_ri * m_params->yaw * 10.0,
			((double)(m_params->li.y+m_params->ri.y))/2.0 + (int)(li_ri * (double)faceMask.cols * 1.0)
		);
		ellipse(faceMask,
			faceEllipse,
			//midp,
			Size((int)floor((double)(faceMask.cols) * li_ri * 1 + li_ri * m_params->yaw * 1),(int)floor(((double)faceMask.cols) * li_ri * 1.58)),
			-m_params->roll,	//angle
			0.0,	//start angle
			360.0,	//end angle
			Scalar(255),CV_FILLED);

		model_recolored = new Mat();
		model_nohead_im->copyTo(*model_recolored);

		r.Recolor(*orig_im,faceMask,*model_recolored,*model_skin_mask);
	}

	cli::array<VirtualSurgeonPoint^>^ VirtualSurgeonWrapper::FindNeck() {
		NeckFinder nf(*m_params);
		Mat _tmp; orig_im->copyTo(_tmp);
		
		nf.FindNeck(_tmp);

		vector<Point> neck = nf.getNeckPoints();

		cli::array<VirtualSurgeonPoint^>^ points = gcnew cli::array<VirtualSurgeonPoint^>(neck.size());
		for(int i=0;i<neck.size();i++) {
			points[i] = gcnew VirtualSurgeonPoint();
			points[i]->x = neck[i].x;
			points[i]->y = neck[i].y;
		}

		return points;
	}

	VirtualSurgeonImage^ VirtualSurgeonWrapper::getVSImage(Mat* im) {
		if(im == NULL) return nullptr;
		if(im->rows == 0 || im->cols == 0) return nullptr;

		VirtualSurgeonImage^ i = gcnew VirtualSurgeonImage();
		
		i->buf = gcnew cli::array<byte>(im->rows * im->step);
		System::Runtime::InteropServices::Marshal::Copy(System::IntPtr::IntPtr(im->data),i->buf,0,im->rows * im->step);

		i->width = im->cols;
		i->height = im->rows;
		i->row_stride = im->step;
		i->channels = im->channels();
		i->buf_size = im->rows * im->step;

		return i;
	}

	void VirtualSurgeonWrapper::InitializeModel(System::String^ filename) {
		if(model_recolored != NULL) delete model_recolored;
		if(model_im != NULL) delete model_im;
		if(model_nohead_im != NULL) delete model_nohead_im;
		if(model_skin_mask != NULL) delete model_skin_mask;
		if(model_warped != NULL) delete model_warped;
		if(m_model_data != NULL) delete m_model_data;

		m_model_data = new VIRTUAL_SURGEON_PARAMS();

		MarshalString(filename,m_model_data->filename);

		model_im = new Mat();
		m_model_data->FaceDotComDetection(*model_im);

		System::String^ s = filename->Substring(0, filename->LastIndexOf('.')) + ".model.png";
		std::string _s; MarshalString(s,_s);
		model_nohead_im = new Mat();
		*model_nohead_im = imread(_s);

		model_skin_mask = new Mat();
		s = filename->Substring(0, filename->LastIndexOf('.')) + ".skin_mask.png";
		MarshalString(s,_s);
		*model_skin_mask = imread(_s,0);
	}

	void VirtualSurgeonWrapper::Warp(cli::array<VirtualSurgeonPoint^>^ face_points_cli,
									cli::array<VirtualSurgeonPoint^>^ model_points_cli) {
		VirtualSurgeon_Warp warp(*m_params);

		assert(orig_im != NULL && orig_im->rows > 0 && model_im != NULL && model_im->rows > 0);

		Mat im; orig_im->copyTo(im);
		Mat im_mask; headMask->copyTo(im_mask);
		Mat _model_im; 
		if(model_recolored != NULL && model_recolored->rows > 0) {
			model_recolored->copyTo(_model_im);
		} else {
			model_nohead_im->copyTo(_model_im);
		}
		Mat model_mask = Mat::ones(model_im->size(),CV_8UC1) * 255;

		vector<Point2d> face_points;
		vector<Point2d> model_points;
		for(int i=0;i<face_points_cli->Length;i++) {
			face_points.push_back(Point2d(face_points_cli[i]->x,face_points_cli[i]->y));
			model_points.push_back(Point2d(model_points_cli[i]->x,model_points_cli[i]->y));
		}

		warp.doWarp(*m_model_data,
					model_points,
					face_points,
					_model_im,
					model_mask,
					im,
					im_mask);

		model_warped = new Mat();
		_model_im.copyTo(*model_warped);
	}

	void FindBoundingRect(Rect& faceRect, Mat* headMask) {	//get bounding rect (TODO: move to utils..)
			Mat arow(1,headMask->cols,CV_32SC1);
			for(int i=0;i<headMask->cols;i++) {
				((int*)arow.data)[i] = i;
			}
			Mat allrows; repeat(arow,headMask->rows,1,allrows);
			Mat rows; allrows.copyTo(rows,(*headMask > 0));
			rows.setTo(Scalar(headMask->cols/2),(*headMask == 0));
			double minv,maxv;
			minMaxLoc(rows,&minv,&maxv);

			faceRect.x = minv;
			faceRect.width = maxv - minv;

			Mat acol(headMask->rows,1,CV_32SC1);
			for(int i=0;i<headMask->rows;i++) {
				((int*)acol.data)[i] = i;
			}
			Mat allcols; repeat(acol,1,headMask->cols,allcols);
			Mat cols; allcols.copyTo(cols,(*headMask > 0));
			cols.setTo(Scalar(headMask->rows/2),(*headMask == 0));
			minMaxLoc(cols,&minv,&maxv);

			faceRect.y = minv;
			faceRect.height = maxv - minv;
		}

	void VirtualSurgeonWrapper::MakeModel(VirtualSurgeonPoint^ face_loc, VirtualSurgeonPoint^ model_loc) {
		if(headMask == NULL || headMask->rows == 0 || headMask->cols==0 ||
			orig_im == NULL || orig_im->rows == 0 || orig_im->cols==0 ||
			model_warped == NULL || model_warped->rows == 0 || model_warped->cols==0) 
			return;

		Rect faceRect;
		FindBoundingRect(faceRect,headMask);

		double model_dist = norm(m_model_data->li - m_model_data->ri);
		model_dist /= cos(m_model_data->yaw / 180.0 * CV_PI);
		double face_dist = norm(m_params->li - m_params->ri);
		face_dist /= cos(m_params->yaw / 180.0 * CV_PI);

		double scaleFromFaceToBack = model_dist / face_dist;

		//double scaleFromFaceToBack = norm(m_model_data->li - m_model_data->ri) / 
		//					norm(m_params->li - m_params->ri);
		//double scaleFromFaceToBack = 1.0;

		Mat _tmp_o; (*orig_im)(faceRect).convertTo(_tmp_o,CV_32F);
		{
			Mat __tmp;
			resize(_tmp_o,__tmp,Size(),scaleFromFaceToBack,scaleFromFaceToBack);
			__tmp.copyTo(_tmp_o);
		}

		Mat mask; //(*headMask)(faceRect).copyTo(mask);
		resize((*headMask)(faceRect),mask,Size(),scaleFromFaceToBack,scaleFromFaceToBack);
		Mat antiMask = -mask + 1.0;

		//TODO: use face_loc & model_loc
		Point face_p(face_loc->x,face_loc->y);
		Point face_p1((int)((double)face_loc->x * scaleFromFaceToBack), 
						(int)((double)face_loc->y * scaleFromFaceToBack));

		Rect modelRect(faceRect);
		modelRect.x *= scaleFromFaceToBack;
		modelRect.y *= scaleFromFaceToBack;
		//modelRect.x = face_p
		modelRect.x += model_loc->x - face_p1.x;
		//modelRect.x += faceRect.width * scaleFromFaceToBack;
		modelRect.y += model_loc->y - face_p1.y;
		//modelRect.y += faceRect.height * scaleFromFaceToBack;
		modelRect.width = _tmp_o.cols;
		modelRect.height = _tmp_o.rows;
		Mat _tmp_m; (*model_warped)(modelRect).convertTo(_tmp_m,CV_32F);

		if(!m_params->no_gui) {
			namedWindow("tmp");
			imshow("tmp",mask);
			waitKey();
			imshow("tmp",_tmp_m / 255.0);
			waitKey();
		}

		vector<Mat> ov; split(_tmp_o,ov);
		vector<Mat> mv; split(_tmp_m,mv);
		for(int i=0;i<ov.size();i++) {
			ov[i] = ov[i].mul(mask) + mv[i].mul(antiMask);
		}
		Mat out;
		merge(ov,out);

		complete = new Mat();

		model_warped->copyTo(*complete);
		out.convertTo((*complete)(modelRect),CV_8UC3);

		//circle(*complete,Point(model_loc->x,model_loc->y),5,Scalar(255,255),3);
		//circle(*complete,face_p,5,Scalar(255,0,255),3);
		//circle(*complete,face_p1,5,Scalar(0,0,255),3);

		
		//imshow("tmp",*complete);
		//waitKey(0);

		//Mat _tmp = (*orig_im).mul(*headMask);
		//_tmp += Mat((*model_nohead_im) * (-Mat(*headMask) + Scalar(1.0)));

		//_tmp.copyTo(*complete);
	}

	VirtualSurgeonWrapper::~VirtualSurgeonWrapper() {
		if(m_params != NULL) delete m_params;
		if(orig_im != NULL) delete orig_im;
		if(headMask != NULL) delete headMask;
		if(complete != NULL) delete complete;
		if(model_recolored != NULL) delete model_recolored;
		if(model_im != NULL) delete model_im;
		if(model_nohead_im != NULL) delete model_nohead_im;
		if(model_skin_mask != NULL) delete model_skin_mask;
		if(model_warped != NULL) delete model_warped;
		if(m_model_data != NULL) delete m_model_data;
	}

}//ns