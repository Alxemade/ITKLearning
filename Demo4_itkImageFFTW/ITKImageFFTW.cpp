//#include "tiffio.h"
#include  "complex.h"
#include "fftw3.h"
#include <stdio.h>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkWrapPadImageFilter.h"
#include "itkForwardFFTImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkComplexToModulusImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include "itkFFTShiftImageFilter.h"
 
#pragma comment(lib, "libfftw3-3.lib")
#pragma comment(lib, "libfftw3f-3.lib")
#pragma comment(lib, "libfftw3l-3.lib")
#define N 8
typedef itk:: Image<float , 2> FloatImageType;    //  
typedef itk ::Image< unsigned char , 2> UnsignedCharImageType;
typedef float MatrixType;
 
void CopyMatrix(FloatImageType:: Pointer image , MatrixType **mask);
void CopyImage(FloatImageType:: Pointer image , fftw_complex *mask);
void CopyRealImage (FloatImageType:: Pointer image , float ** mask);//仅仅拷贝实部
void MyCastFloatToUnsichar(FloatImageType::Pointer image, UnsignedCharImageType::Pointer imageNew );//将float转为unsigned char型
int main()
{
	//读取图像
	 FloatImageType::Pointer image;
	
	 const char * inputFileName = "E:\\XC\\itk\\Examples\\Demo4_itkImageFFTW\\bin\\lena.jpg" ; //argv[1];
	 const char * outputFileName = "E:\\XC\\itk\\Examples\\Demo4_itkImageFFTW\\bin\\lena.tif" ; //argv[2];
 
	 typedef itk ::ImageFileReader< FloatImageType> ReaderType ;
	 ReaderType::Pointer reader = ReaderType::New ();
	 reader->SetFileName (inputFileName);
	 reader->Update ();
	 image = reader ->GetOutput();
 
	//遍历图像像素，赋值到一个矩阵
	 MatrixType **pixelColors = new MatrixType*[512]();
	 for (int i=0;i<512;i++)
	 {
		 pixelColors[i] = new MatrixType[512]; //为每个指针都分配8个char元素空间。
	 }
	 CopyMatrix(image ,  pixelColors);
 
	int width =512;
	int height=512;
 
	fftw_plan planR;
	fftw_complex *inR,  *outR, *resultR;
 
	//Allocate arrays.
	inR = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * width);
	outR = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * width);
	resultR = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * width * width);
 
	//Fill in arrays with the pixelcolors.
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			//int currentIndex = ((y * width) + (x)) * 3;
			int currentIndex = (y * width) + x;
			inR[y * width + x][0] = (double)pixelColors[y][x];
			inR[y * width + x][1] = 0.0;	
		}
	}
 
	//Forward plans.
	planR = fftw_plan_dft_2d(width, width, inR, outR, FFTW_FORWARD, FFTW_MEASURE);
	
	//Forward FFT.
	fftw_execute(planR);
	
	//Backward plans.
	fftw_plan planRR = fftw_plan_dft_2d(width, width, outR, resultR, FFTW_BACKWARD, FFTW_MEASURE);
	
	//Backward fft
	fftw_execute(planRR);
 
	//// Have to scale the output values to get back to the original.
	int size=height*width;
	for(int i = 0; i < size; i++) {
		resultR[i][0] = resultR[i][0] / size;
		resultR[i][1] = resultR[i][1] / size;
	}
 
	//Overwrite the pixelcolors with the result.
	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++)
		{
			//int currentIndex = ((y * width) + (x)) * 3;
			pixelColors[y][x] =resultR[y * width + x][0];//
			//std::cout<<pixelColors[y][x]<<'\t';
		}
	}
	
	//将图像写出来
	CopyRealImage(image , pixelColors);//将pixelColors中的像素值复制到image中
	//调整亮度
	typedef itk ::RescaleIntensityImageFilter< FloatImageType , FloatImageType > RescaleFilterType;
	RescaleFilterType::Pointer imaginaryRescaleFilter = RescaleFilterType ::New();
	imaginaryRescaleFilter->SetInput (image);
	imaginaryRescaleFilter->SetOutputMinimum (0);
	imaginaryRescaleFilter->SetOutputMaximum (255);
	imaginaryRescaleFilter->Update ();
	//转为8bit
	//类型转换，float转为8bit的。
	typedef itk ::CastImageFilter< FloatImageType, UnsignedCharImageType > CastFilterType;
	CastFilterType::Pointer castFilter = CastFilterType::New ();
	castFilter->SetInput (imaginaryRescaleFilter->GetOutput());
	castFilter->Update ();
	//将图像写出来
	typedef itk ::ImageFileWriter< UnsignedCharImageType > WriterType ;
	WriterType::Pointer writer = WriterType::New ();
	writer->SetFileName ( outputFileName );
	writer->SetInput (castFilter->GetOutput());
	try
	{
		writer->Update ();
	}
	catch( itk ::ExceptionObject & error )
	{
		std::cerr << "Error: " << error << std ::endl;
		return EXIT_FAILURE ;
	}
	delete []pixelColors;
	fftw_free(inR);
	fftw_free(outR);
	fftw_free(resultR);
 
	return 0;
}
void CopyMatrix(FloatImageType:: Pointer image , MatrixType **mask)
{
	FloatImageType::RegionType region = image->GetLargestPossibleRegion ();
	FloatImageType::SizeType regionSize = region.GetSize ();
 
	for (int x=0; x<regionSize [0]; x++)
		for(int y=0; y<regionSize [1]; y++)
		{	//3、定义像素索引
			FloatImageType::IndexType index;
			index[0] = x ;
			index[1] = y ;
			mask[x][y] =(MatrixType) image->GetPixel(index);//获取该位置的像素值
		}
 
}
 
void CopyImage(FloatImageType:: Pointer image , fftw_complex *mask)
{
	FloatImageType::RegionType region = image->GetLargestPossibleRegion ();
	FloatImageType::SizeType regionSize = region.GetSize ();
 
	for (int x=0; x<regionSize [0]; x++)
		for(int y=0; y<regionSize [1]; y++)
		{
			double re = mask[x*regionSize[1]+y][0];
			double im = mask[x*regionSize[1]+y][1];
			double mag = sqrt (re*re + im*im);
			//3、定义像素索引
			FloatImageType::IndexType index;
			index[0] = x ;
			index[1] = y ;
			image->SetPixel(index, mag);//获取该位置的像素值
		}
 
}
//仅仅将mask中的值，赋给image中！
void CopyRealImage (FloatImageType:: Pointer image , MatrixType ** mask)
{
	FloatImageType::RegionType region = image->GetLargestPossibleRegion ();
	FloatImageType::SizeType regionSize = region.GetSize ();
	int Imgsize = regionSize[0]*regionSize[1];
 
	for (int x=0; x<regionSize [0]; x++)
		for(int y=0; y<regionSize [1]; y++)
		{	//3、定义像素索引
			FloatImageType::IndexType index;
			index[0] = x ;
			index[1] = y ;
			
			image->SetPixel (index, mask[x ][y]); 
			//获取该位置的像素值
			//std::cout<<image->GetPixel(index);
		}
 
}
void MyCastFloatToUnsichar(FloatImageType::Pointer image, UnsignedCharImageType::Pointer imageNew )//将float转为unsigned char型
{
	FloatImageType::RegionType region = image->GetLargestPossibleRegion ();
	FloatImageType::SizeType regionSize = region.GetSize ();
	
	for (int x=0; x<regionSize [0]; x++)
		for(int y=0; y<regionSize [1]; y++)
		{	//3、定义像素索引
			FloatImageType::IndexType index;
			index[0] = x ;
			index[1] = y ;
			//std::cout<<image->GetPixel(index)<<'\t';
			float temp = static_cast<unsigned char>(image->GetPixel(index));
			//std::cout<<temp<<'\t';
			imageNew->SetPixel (index, temp); 
			int tt=imageNew->GetPixel(index);
			////获取该位置的像素值
			//std::cout<<tt<<'\t';
		}
 
}
 