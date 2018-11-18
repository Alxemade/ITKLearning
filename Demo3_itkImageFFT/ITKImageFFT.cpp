#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkForwardFFTImageFilter.h"
#include "itkComplexToModulusImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include "itkFFTShiftImageFilter.h"
 
int main( int argc, char* argv[] )
{
	// 首先设置读取和写入文件的路径
	const char * inputFileName = "E:\\XC\\itk\\Examples\\Demo3_itkImageFFT\\bin\\lena.jpg"; //argv[1];
	const char * outputFileName = "E:\\XC\\itk\\Examples\\Demo3_itkImageFFT\\bin\\lena.tif"; //argv[2];
 
	const unsigned int Dimension = 2;  // 设置我们输出的维度，这里是二维图像，所以是二维
 
	//typedef unsigned char                              PixelType;
	typedef float                              PixelType;//定义像素类型。
	typedef itk::Image< PixelType, Dimension > RealImageType;  // 定义图像的类型
 
	typedef itk::ImageFileReader< RealImageType >  ReaderType;  // 定义图像读取的类型
	ReaderType::Pointer reader = ReaderType::New();  //定义一个图像读取的指针
	reader->SetFileName( inputFileName );  // 开始读取一幅图像
 
	// 首先定义一个正向的傅里叶变换类型，注意ForwardFFTImageFilter是一个抽象基类
	typedef itk::ForwardFFTImageFilter< RealImageType > ForwardFFTFilterType;  // brief Base class for forward Fast Fourier Transform.
	typedef ForwardFFTFilterType::OutputImageType ComplexImageType;  // 设置图像输出的数据类型，这里将其定义为复数数据类型
	ForwardFFTFilterType::Pointer forwardFFTFilter = ForwardFFTFilterType::New();  // 定义一个FFT正变换的指针变量
	forwardFFTFilter->SetInput( reader->GetOutput() );  // itk管线连接
	// ForwardFFTImageFilter is an abstract base class: the actual implementation is provided by the best child class available on the
														//计算复图像的系数。
	typedef itk::ComplexToModulusImageFilter< ComplexImageType, RealImageType >  // 这里输出为什么是RealImageType图像了？
		ComplexToModulusFilterType;  // \brief Computes pixel-wise the Modulus of a complex image.
	ComplexToModulusFilterType::Pointer complexToModulusFilter
		= ComplexToModulusFilterType::New();  // 建立一个智能指针
	complexToModulusFilter->SetInput( forwardFFTFilter->GetOutput() ); // 开始计算复图像的系数？
 
	// Window and shift the output for visualization.
	typedef unsigned char                           OutputPixelType;//8位
	//typedef unsigned short                           OutputPixelType;//16位
	//在指定的范围内进行线性变换，低于该区域的映射为某一常数，高于高区域的映射为某一常数
	// 这个是为了显示才有的，要不然我们如何显示复数的系数了
	typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
	typedef itk::IntensityWindowingImageFilter< RealImageType, OutputImageType >
		WindowingFilterType;
	WindowingFilterType::Pointer windowingFilter
		= WindowingFilterType::New();
	windowingFilter->SetInput( complexToModulusFilter->GetOutput() );
	windowingFilter->SetWindowMinimum( 0 );
	windowingFilter->SetWindowMaximum( 20000 );  // 设置窗口的最大值和最小值
 
	//将四角的频率成分移动中心位置
	typedef itk::FFTShiftImageFilter< OutputImageType, OutputImageType > FFTShiftFilterType;
	FFTShiftFilterType::Pointer fftShiftFilter = FFTShiftFilterType::New();  // Shift the zero-frequency components of a Fourier transform to the center of the image
	fftShiftFilter->SetInput( windowingFilter->GetOutput() );
 
	typedef itk::ImageFileWriter< OutputImageType > WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( outputFileName );
	writer->SetInput( fftShiftFilter->GetOutput() );   // 写入图像
	try
	{
		writer->Update();  // 如果没有出现异常那么直接写入
	}
	catch( itk::ExceptionObject & error )
	{
		std::cerr << "Error: " << error << std::endl;  // 如果出现异常则抛出异常
		return EXIT_FAILURE;
	}
 
	return EXIT_SUCCESS;
}