#include "itkImage.h"
#include "itkIdentityTransform.h"
#include "itkImageFileReader.h"
#include "itkVectorResampleImageFilter.h"
#include "itkVectorNearestNeighborInterpolateImageFunction.h"
#include "itkRGBPixel.h"
#include <typeinfo>
#include "QuickView.h"
 
typedef itk::Image<itk::RGBPixel<unsigned char>, 2> ImageType;  // 定义一个图像类型
 
static void CreateImage(ImageType::Pointer image);  // 静态成员函数声明，只需要在声明的时候加上static就可以了，定义的地方不需要加
// 注意这里的ImageType::Pointer相当于是SmartPointer<ImageType> 类型的数据，智能指针
int main(int argc, char *argv[])
{
	double factor = 2.0;  // 采样率，默认的采样率是2
 
	// Create input image
	ImageType::Pointer input;
	if(argc < 2)  // 如果仅仅输入一个参数，比如仅仅输入一个ITKImageResample.cpp的话，那么就是显示空白的图像
	{
		input = ImageType::New();  // 创建一个ImageType类型的数据，然后返回一个SmartPointer<ImageType>类型的指针
		CreateImage(input);
	}
	else  // 这里表示输入的参数大于1个，也就是可能会输入路径或者或者输入第三个参数：采样率
	{
		typedef itk::ImageFileReader<ImageType> ReaderType;  // 定义图像读取类型
		ReaderType::Pointer reader = ReaderType::New();  // 定义一个指针类型
		reader->SetFileName(argv[1]);  // 获取图像的路径，然后读取图像
		reader->Update();  // brief Bring this filter up-to-date,也就是将这个filter变成实时的
		input = reader->GetOutput();  // 获取输入
		if (argc > 2)  // 如果输入的参数大于2个，就表示此时已经输入了采样率了
		{
			factor = atof(argv[2]);  // atof是将string类型转成float类型
		}
	}
	// GetLargestPossibleRegion: Get the region object that defines the size and starting index for the largest possible region this image could represent.
	// GetSize: Get the size of the region
	// 也就是说这里代码的含义就是首先读取一幅图像，然后我们读取他的尺寸，然后才给他分配空间
	ImageType::SizeType inputSize = input->GetLargestPossibleRegion().GetSize();
	// 这里为什么加入我们只输入一个参数也是输出200,200？这是因为我们在CreateImage()已经指定了图像默认大小是200,200
	std::cout << "Input size: " << inputSize << std::endl;  // 这里输出我们输入图像的大小

	// Resize， 重新调整图像大小
	ImageType::SizeType outputSize;
	outputSize[0] = inputSize[0] * factor;  // 这两句代码保存的是输入的图像乘以factor大小
	outputSize[1] = inputSize[1] * factor;
 
	ImageType::SpacingType outputSpacing;  // Spacing holds the size of a pixel.
	// 这里的intputSize[0]和outputSize[0]都是unsigned_64类型，所以需要将其变成double类型
	// 这里使用outputSpacing的原因也是很简单的，因为我们经过上面运算，输出图像大小已经发生了变化，那么space就是使得输出图像和之前的保持一样的大小
	outputSpacing[0] = input->GetSpacing()[0] * (static_cast<double>(inputSize[0]) / static_cast<double>(outputSize[0]));
	outputSpacing[1] = input->GetSpacing()[1] * (static_cast<double>(inputSize[1]) / static_cast<double>(outputSize[1]));
	//std::cout << "Input image type: " << typeid(inputSize[0]).name() << std::endl;
	//std::cout << "Output image type: " << typeid(outputSize[0]).name() << std::endl;
	//std::cout << "Input image space type: " << typeid(input->GetSpacing()[0]).name() << std::endl;
	//std::cout << "Input image space: " << input->GetSpacing()[0] << std::endl;
	//std::cout << "Output image space: " << outputSpacing[0] << std::endl;
	
	typedef itk::IdentityTransform<double, 2> TransformType;  // 设置一个变换类型，默认的变换维度是三维，这里需要将其变成2维
	typedef itk::VectorResampleImageFilter<ImageType, ImageType> ResampleImageFilterType; // brief Resample an image via a coordinate transform
	// first kind
	ResampleImageFilterType::Pointer resample = ResampleImageFilterType::New();  // 设置一个指针类型
	resample->SetInput(input);  // 设置输入
	resample->SetSize(outputSize);  // 设置输出的尺寸
	resample->SetOutputSpacing(outputSpacing);  // Set the output image spacing
	resample->SetTransform(TransformType::New());  //  Set/Get the coordinate transformation
	resample->UpdateLargestPossibleRegion();  // brief Sets the output requested region to the largest possible region and updates.
 
	typedef itk::VectorNearestNeighborInterpolateImageFunction<
		ImageType, double >  NearestInterpolatorType;  // Nearest neighbor interpolate a vector image at specified positions.
	NearestInterpolatorType::Pointer nnInterpolator =
		NearestInterpolatorType::New();  // 建立一个插值？
	// second kind
	ResampleImageFilterType::Pointer resampleNN =
		ResampleImageFilterType::New();
	resampleNN->SetInput(input);
	resampleNN->SetSize(outputSize);
	resampleNN->SetOutputSpacing(outputSpacing);
	resampleNN->SetTransform(TransformType::New());
	resampleNN->SetInterpolator(nnInterpolator);//相比较于，多了最近邻插值,Set the interpolator function.
	resampleNN->UpdateLargestPossibleRegion();
 
	ImageType::Pointer output = resample->GetOutput();  // 定义一个输出指针
 
	std::cout << "Output size: " << output->GetLargestPossibleRegion().GetSize() << std::endl;  // 输出图像大小
 
	QuickView viewer;  //  A convenient class to render itk images with vtk
	// 然后
	viewer.AddRGBImage(input.GetPointer(),
		true,
		"Original");  // 添加第一幅图像
	viewer.AddRGBImage(output.GetPointer(),
		true,
		"Resampled");  // 添加第二幅图像，直接0填充
 
	viewer.AddRGBImage(resampleNN->GetOutput(),
		true,
		"Resampled NN"); // 使用最近邻插值方法
 
	viewer.Visualize(); // Render the images. If interact is tru, start a vtk Interactor.
 
	return EXIT_SUCCESS;
}
// 创建一幅图像
void CreateImage(ImageType::Pointer image)
{
	// Create a black image with 2 white regions
 
	ImageType::IndexType start;  // Index typedef support. An index is used to access pixel values.
	start.Fill(0);  // Set one value for the index in all dimensions
 
	ImageType::SizeType size;
	size.Fill(200);  // 这个是填充图像的尺寸大小
 
	ImageType::RegionType region(start,size); // A region is used to specify a subset of an image.
	image->SetRegions(region);  // 设置区域的大小
	image->Allocate();  // 开始分配内存空间
	image->FillBuffer( itk::NumericTraits< ImageType::PixelType >::Zero);  // Define numeric traits for RGBPixel, 使用0进行填充
 
	ImageType::PixelType pixel;
	pixel.SetRed(200);  // Set the Red component.
	pixel.SetGreen(50);  // Set the Green component
	pixel.SetBlue(50);  // Set the Green component
 
	// Make a square,在背景创建一个矩形框
	for(unsigned int r = 20; r < 80; r++)  // 长60
	{
		for(unsigned int c = 30; c < 100; c++)  // 宽70
		{
			ImageType::IndexType pixelIndex;
			pixelIndex[0] = r;
			pixelIndex[1] = c;
 
			image->SetPixel(pixelIndex, pixel);  // 也就是在对应的位置pixelIndex设置颜色pixel
 
		}
	}
 
}