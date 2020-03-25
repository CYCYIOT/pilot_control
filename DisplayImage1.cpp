#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <thread>
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include "udp.h"
using namespace ATL;
#define BL 0
#define WH 255
using namespace std;
using namespace cv;
//using namespace ATL;
Mat frame, roi, temp;
Mat BinOriImg; 
int flag=0;
int flag_t=1;
int number = 0;
//int g=0;
int numRecognizer1();

void fun1(int a)
{
	while (flag_t) {
		Sleep(3000);
	}
// namedWindow("数字识别1",0);
 while(flag){ 
  //waitKey(20);
  numRecognizer1(); 
  //namedWindow("数字识别1",0);
}
//destroyWindow("数字识别1");
}


void fun2(int a)
{
	main_control();
}

int main()
{
	  // VideoCapture cap(0);
         VideoCapture cap;
         namedWindow("video",0);
         resizeWindow("video",640,400);
        
      
       std::thread temp1(fun1,3);
        temp1.detach();

		std::thread temp2(fun2,3);
		temp2.detach();
#if 0
		Mat img = imread("1.jpg");  //读一张图片
	//	namedWindow("原画");  //创建一个“原画”窗口
		imshow("kkkk", img);   //在窗口中显示图片
		waitKey(0);  //等待6000ms关闭窗口 
#endif
		char str[10];
		int count1 = 0;
#if 1
lope:
        cap.open("rtsp://192.168.100.1:7070/H264VideoSMS");
        // cap.open("rtsp://192.168.1.4:8554/testStream"); 
	//temp = imread("blue1.jpg", IMREAD_COLOR);  // 模板，原图0~9
	//imshow("3",temp);
        int run=0;
        flag=1;
       // imshow("数字识别",temp);
       // waitKey(0);
	//	cout << "jjj" << endl;
	while (cap.isOpened())
        {
        // cap.open("rtsp://192.168.100.1:7070/H264VideoSMS");
		cap >> frame;
		//roi = frame(Rect(20, 80, 600, 100)).clone();
		//blur(frame, frame, Size(15, 15));
		//roi.copyTo(frame(Rect(20, 80, 600, 100)));
		//rectangle(frame, Point(20, 80), Point(620, 180), Scalar(0, 200, 0), 2);
		sprintf(str,"NUMBER:%d",number);
		putText(frame, str, Point(50, 60), FONT_HERSHEY_COMPLEX, 2, Scalar(0, 255, 0), 4, 8);
		imshow("video", frame);
		int key = waitKey(20);
		if (frame.empty() || key == 27)
			break;
		//if (key == (int)'0')
                 // waitKey(0);
	   // numRecognizer1();
	  //printf("run = %d\n",run++);
            flag_t=0;
			if (run++ >= 700)
				//printf("count = %d",count1++);
				cout << "11" << endl;
                 goto lope;
			
           }
#endif
     // numRecognizer1();
     flag=0;
     printf("end\n");
	// system("pause");
     cap.release();//释放资源
     destroyAllWindows();//关闭所有窗口
     //waitKey(0);
	
	
    return 0;
}
 
int getColSum(Mat&src,int i)
{
 Mat col=src.col(i).clone();
 //imshow("ff",col);
// waitKey(0);
 int sum=0;
 for(int j=0;j<src.rows;j++)
  {
  // printf("col = %d ",col.data[j]);
   sum+=col.data[j];
  } 
 return sum;
}

int getRowSum(Mat&src, int i)
{
Mat row=src.row(i);
//imshow("ff",row);
int sum=0;
for(int j=0;j<src.cols;j++)
sum+=row.data[j];
//waitKey(0);
return sum;
}

void cutTop(Mat& src, Mat& dstImg)//上下切割  
{  
    int top, bottom;  
    top = 0;  
    bottom = src.rows;  
  
    int i;  
    for (i = 0; i < src.rows; i++)  
    {  
        int colValue = getRowSum(src, i);  
        //cout <<i<<" th "<< colValue << endl;  
        if (colValue>0)  
        {  
            top = i;  
            break;  
        }  
    }  
    for (; i < src.rows; i++)  
    {  
        int colValue = getRowSum(src, i);  
        //cout << i << " th " << colValue << endl;  
        if (colValue == 0)  
        {  
            bottom = i;  
            break;  
        }  
    }  
  
    int height = bottom - top;  
    Rect rect(0, top, src.cols, height);  
    dstImg = src(rect).clone();  
}  

int cutLeft(Mat& src, Mat& leftImg, Mat& rightImg)//左右切割  
{  
    int left, right;  
    left = 0;  
    right = src.cols;  
  
    int i;  
   // printf("cols = %d rows = %d \n",src.cols,src.rows);
    for (i = 1; i < src.cols; i++)  
    {  
        //getRowSum(src, i);
        int colValue = getColSum(src, i);  
       // cout <<i<<" th "<< colValue << endl;  
        if (colValue>0)  
        {  
            left = i;  
            break;  
        }  
    }  
    if (left == 0)  
    {  
        return 1;  
    }  
  
  
    for (; i < src.cols; i++)  
    {  
        int colValue = getColSum(src, i);  
        //cout << i << " th " << colValue << endl;  
        if (colValue == 0)  
        {  
            right = i;  
            break;  
        }  
    }  
   
    int width = right - left;  
    Rect rect(left, 0, width, src.rows);  
    leftImg = src(rect).clone();  
    Rect rectRight(right, 0, src.cols - right, src.rows);  
    rightImg = src(rectRight).clone();  
    cutTop(leftImg, leftImg);  
    return 0;  
}
 
void getPXSum(Mat &src, int &a)//获取所有像素点和  
{   
    threshold(src, src, 150, 255, CV_THRESH_BINARY);  
      a = 0;  
    for (int i = 0; i < src.rows;i++)  
    {  
        for (int j = 0; j < src.cols; j++)  
        {  
            a += src.at <uchar>(i, j);  
        }  
    }  
}  

int sum_test(Mat& src)
{
int colValue=0;
for (int i = 1; i < src.cols; i++)  
    {  
        //getRowSum(src, i);
         colValue += getColSum(src, i);  
     }
// printf("colv = %d\n",colValue);
return colValue;
}

int  getSubtract(Mat &src, int TemplateNum) //两张图片相减  
{  
    Mat img_result;  
    int min = 1000000;  
    int serieNum = 0; 
    int diff; 

     if( sum_test(src)  < 50000){
      // printf("error colValue \n");
	   number = 0;
       return -1;
     }

    for (int i = 0; i < TemplateNum; i++){  
        char name[20];  
        sprintf_s(name, "./%dLeft.jpg", i);  
        Mat Template = imread(name, CV_LOAD_IMAGE_GRAYSCALE);  
       // threshold(Template, Template, 100, 255, CV_THRESH_BINARY);  
       // threshold(src, src, 100, 255, CV_THRESH_BINARY);  
        resize(src, src, Size(32, 48), 0, 0, CV_INTER_LINEAR);  
        resize(Template, Template, Size(32, 48), 0, 0, CV_INTER_LINEAR);//调整尺寸        
        //imshow(name, Template);  
        absdiff(Template, src, img_result);//  
        getPXSum(img_result, diff);  
        if (diff < min)  
        {  
            min = diff;  
            serieNum = i;  
        }  
    } 
   
   // cout <<g++;
	//cout << "num:" <<serieNum<< endl;
	number = serieNum;
   // printf_s("最小距离是%d ",min);  
   // printf_s("匹配到第%d个模板匹配的数字是%d\n",serieNum,serieNum);
     
    
    return serieNum;  
}  
  
int chepai()
{
	unsigned char pixelB, pixelG, pixelR;  //记录各通道值
	unsigned char DifMax = 50;             //基于颜色区分的阈值设置
	unsigned char B = 138, G = 63, R = 23; //各通道的阈值设定，针对与蓝色车牌
        // namedWindow("形态学处理后huidu",0);
        // resizeWindow("形态学处理后huidu",500,500);
        Mat ResizeImg=frame.clone(); 
	if (ResizeImg.cols > 640)
	    resize(ResizeImg, ResizeImg, Size(640, 640 * ResizeImg.rows / ResizeImg.cols));
	//imshow("形态学处理后huidu", ResizeImg);
        
	
	Mat BinRGBImg = ResizeImg.clone();  //二值化之后的图像
	//int i = 0, j = 0;
	for (int i = 0; i < ResizeImg.rows; i++)   //通过颜色分量将图片进行二值化处理
	{
		for (int j = 0; j < ResizeImg.cols; j++)
		{
			pixelB = ResizeImg.at<Vec3b>(i, j)[0]; //获取图片各个通道的值
			pixelG = ResizeImg.at<Vec3b>(i, j)[1];
			pixelR = ResizeImg.at<Vec3b>(i, j)[2];
 
			if (abs(pixelB - B) < DifMax && abs(pixelG - G) < DifMax && abs(pixelR - R) < DifMax)
			{                                              //将各个通道的值和各个通道阈值进行比较
				if(i==0 || j==0 || i == (ResizeImg.rows-1) || j == (ResizeImg.cols-1))
                                   return 0;
                                  
                                BinRGBImg.at<Vec3b>(i, j)[0] = 255;     //符合颜色阈值范围内的设置成白色
				BinRGBImg.at<Vec3b>(i, j)[1] = 255;
				BinRGBImg.at<Vec3b>(i, j)[2] = 255;
			}
			else
			{
				BinRGBImg.at<Vec3b>(i, j)[0] = 0;        //不符合颜色阈值范围内的设置为黑色
				BinRGBImg.at<Vec3b>(i, j)[1] = 0;
				BinRGBImg.at<Vec3b>(i, j)[2] = 0;
			}
		}
	}
       // waitKey(0);
        Mat pFrame1;
	//imshow("基于颜色信息二值化", BinRGBImg);        //显示二值化处理之后的图像
    // equalizeHist(BinRGBImg,BinRGBImg);


	//Mat BinOriImg;     //形态学处理结果图像
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3)); //设置形态学处理窗的大小
	dilate(BinRGBImg, BinOriImg, element);     //进行多次膨胀操作
	dilate(BinOriImg, BinOriImg, element);
	dilate(BinOriImg, BinOriImg, element);
 
	erode(BinOriImg, BinOriImg, element);      //进行多次腐蚀操作
	erode(BinOriImg, BinOriImg, element);
	erode(BinOriImg, BinOriImg, element);
	//imshow("形态学处理后", BinOriImg);        //显示形态学处理之后的图像

        cvtColor(BinOriImg, BinOriImg, CV_BGR2GRAY);   //将形态学处理之后的图像转化为灰度图像
	threshold(BinOriImg, BinOriImg, 100, 255, THRESH_BINARY); //灰度图像二值化
       // imshow("形态学处理后huidu", BinOriImg); 
      //  Mat edge;
     //   Canny(BinOriImg, BinOriImg, 30, 200, 3);
      //  imshow("形态学处理后canny", BinOriImg); 
        return 0;
}

int check_circles()
{
        Mat srcImage = frame.clone();
	Mat midImage,dstImage;//临时变量和目标图的定义
 
	//【2】显示原始图
	//imshow("【原始图】", srcImage);  
 
	//【3】转为灰度图，进行图像平滑
	cvtColor(srcImage,midImage, CV_BGR2GRAY);//转化边缘检测后的图为灰度图
	GaussianBlur( midImage, midImage, Size(9, 9), 2, 2 );
 
	//【4】进行霍夫圆变换
	vector<Vec3f> circles;
	HoughCircles( midImage, circles, CV_HOUGH_GRADIENT,1.5, 10, 200, 100, 0, 0 );
        if(!circles.size())
         {
       //cout<<"NO ciecles"<<endl;
         return -1;
          }
   // cout<<"have"<<circles.size()<<" ciecles"<<endl;
return 0;
}
int numRecognizer1()  
{  

	if (check_circles()) {
		number = 0;
		return -1;
	}
       if(chepai()){
        // cout<<"图片不完整"<<endl;
		   number = 0;
         return -1;
              }
 
    

        Mat src=BinOriImg.clone();
	//cvtColor(roi, src, CV_BGR2GRAY);
	//threshold(src, src, 100, 255, THRESH_BINARY_INV); 
       // imshow("origin", src);   


    Mat leftImg,rightImg;  
    int res = cutLeft(src, leftImg, rightImg);    
    int i = 0;  
  //printf("res = %d\n",res); 
   // imshow("tets",leftImg);
  if (res == 1)
	  number = 0;
    while (res == 0)  
    {         
        char nameLeft[10];  
        sprintf_s(nameLeft, "%dLeft", i);  
        char nameRight[10];  
        sprintf_s(nameRight, "%dRight", i);  
        i++;  
#if 0
        stringstream ss;  //制作模板
        ss << nameLeft;  
        imwrite("./" + ss.str() + ".jpg", leftImg);  
        ss >> nameLeft;  
#endif
        Mat srcTmp = rightImg;  
        getSubtract(leftImg, 10); //匹配模板  
        res = cutLeft(srcTmp, leftImg, rightImg);     
    }  
   // printf("end .....\n");    
   // waitKey(0);  
    return 0;  
} 


