#pragma once

public ref class OpenCVImageWrapper
{
public:
	 int width;

        int height;

        cli::array<byte>^ buf;

        int buf_size;

        int row_stride;

        int channels;

	OpenCVImageWrapper(void);

	static OpenCVImageWrapper^ getFromOpenCV();
};
