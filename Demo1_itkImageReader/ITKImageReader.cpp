#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h> 
 
int main(int argc, char* argv[]) 
{ 
	const unsigned int Dimension = 2;                                          //定义图像维数 
	//typedef unsigned char                                     PixelType;      //定义像素类型 
	typedef itk::RGBPixel< unsigned char >   PixelType;  // RGBPixel相当于是类模板的继承，这里的PiexlType相当于是一种数据类型
	typedef itk::Image< PixelType, 2 >       ImageType;  // 定义一个图像类型
	
	typedef itk::ImageFileReader< ImageType >  ReaderType;  // 定义一个读取类型
	typedef itk::ImageFileWriter< ImageType >  WriterType;  // 定义一个写入图像类型
	ReaderType::Pointer reader = ReaderType::New();  // 创建一个文件读指针
	WriterType::Pointer writer = WriterType::New();  // 创建一个文件写指针
 
	reader->SetFileName("E:\\XC\\itk\\Examples\\Demo1_itkImageReader\\bin\\hua.jpg");   // 文件读路径
	writer->SetFileName( "E:\\XC\\itk\\Examples\\Demo1_itkImageReader\\bin\\hua2.jpg");  // 文件写路径
 
	ImageType::Pointer image = reader->GetOutput();  // 将数据读取进来
	writer->SetInput( image );  // 写入读取的对象

	writer->Update();  //  Aliased to the Write() method to be consistent with the rest of the pipeline.
 
	return 0; 
}