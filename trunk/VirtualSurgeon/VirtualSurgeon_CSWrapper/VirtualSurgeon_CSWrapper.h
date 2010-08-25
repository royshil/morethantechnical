// VirtualSurgeon_CSWrapper.h
#pragma once

using namespace System;

#include "VirtualSurgeon_ParamsWrapper.h"

namespace VirtualSurgeon_CSWrapper {
	public ref class VirtualSurgeonImage {
	public:
		int width;
		int height;
		cli::array<byte>^ buf;
		int buf_size;
		int row_stride;
		int channels;
	};

	public ref struct VirtualSurgeonPoint {
		int x,y;
	};

	public ref class VirtualSurgeonWrapper
	{
	private:
		VirtualSurgeon::HeadExtractor* m_he;
		VirtualSurgeonParams* m_params;
		VirtualSurgeonFaceData* m_model_data;

		Mat* orig_im;
		Mat* headMask;

		Mat* model_im;
		Mat* model_nohead_im;
		Mat* model_warped;
		Mat* model_recolored;
		Mat* model_skin_mask;
		Mat* model_body_mask;

		Mat* complete;

		VirtualSurgeonImage^ getVSImage(Mat* im);

	public: 
		VirtualSurgeonWrapper() {};
		~VirtualSurgeonWrapper();
		void Initialize(System::String^ filename);
		void InitializeModel(System::String^ filename);
		System::String^ GetFilename();

		void ExtractHead();
		cli::array<VirtualSurgeonPoint^>^ FindNeck();
		void Warp(cli::array<VirtualSurgeonPoint^>^ face_points_cli,
					cli::array<VirtualSurgeonPoint^>^ model_points_cli);
		void MakeModel(VirtualSurgeonPoint^ face_loc, VirtualSurgeonPoint^ model_loc);
		void Recolor();
		
		VirtualSurgeon_ParamsWrapper^ getParamsWrapper();
		VirtualSurgeonImage^ getHeadMask() {return getVSImage(headMask);};
		VirtualSurgeonImage^ getOrigImage() {return getVSImage(orig_im);};
		VirtualSurgeonImage^ getModelImage() {return getVSImage(model_im);};
		VirtualSurgeonImage^ getCompleteModel() {return getVSImage(complete);};
		VirtualSurgeonImage^ getWarpedModel() {return getVSImage(model_warped);};
		VirtualSurgeonImage^ getModelNoHead() {return getVSImage(model_nohead_im);};
		VirtualSurgeonImage^ getModelRecolored() {return getVSImage(model_recolored);};
	};

} //ns
