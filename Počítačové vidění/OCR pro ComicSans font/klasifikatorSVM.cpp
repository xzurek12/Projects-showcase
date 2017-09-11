#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/nonfree/features2d.hpp"
#include <opencv/ml.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <stdio.h>
#include <dirent.h>
#include <string.h>


using namespace std;
using namespace cv;

#define DATASET_LOCATION "dataset/chars/no noise/"
#define COMMA 44
#define DASH 45
#define PERIOD 46
#define QUESTIONMARK 63

//---------------------------------------------------------------------------------------------
/**
*	Trida reprezentujici KNN algoritmus
**/
class KNN_OCR
{
	public:
		KNN_OCR();
		void get_data();
		void prepare_KNN();
		float classify(Mat input_image);
		void train();
		void test();
		Mat image_preprocess(Mat input_image);
		Mat image_deskew(Mat input_image);
		Mat get_hog_descriptor(Mat input_image);

		// for SVM
		int SVMclassify(Mat input_image);

	private:
		int K;
		Mat trainData, trainClasses;
		KNearest knn;
		vector<string> images;
		vector<int> classes;

		// for SVM
		CvSVMParams params; 
		CvSVM svm;
};
//---------------------------------------------------------------------------------------------
/**
*	Konstruktor tridy KNN_OCR
**/
KNN_OCR::KNN_OCR()
{
	get_data();
	prepare_KNN();
	train();

	printf("----------------------- Train READY -------------------\n");
	printf(" ------------------------------------------------------\n");
}
//---------------------------------------------------------------------------------------------
/**
*	Funkce image_deskew vyrovna sklon(skew) vstupniho obrazku
**/
Mat KNN_OCR::image_deskew(Mat input_image)
{
	Mat thr;
	threshold(input_image, thr, 200, 255, THRESH_BINARY_INV);

	vector<Point> points;
	Mat_<uchar>::iterator it = thr.begin<uchar>();
	Mat_<uchar>::iterator end = thr.end<uchar>();

	for (; it != end; ++it)
	{
		if (*it) points.push_back(it.pos());
	}
		
	RotatedRect box = minAreaRect(Mat(points));
	Mat rot_mat = getRotationMatrix2D(box.center, box.angle, 1);

	Mat rotated;
	warpAffine(input_image, rotated, rot_mat, input_image.size(), INTER_CUBIC);
	
	return rotated;
}
//---------------------------------------------------------------------------------------------
/**
*	Funkce get_hog_descriptor extrahuje priznakovy vektor z predzpracovaneho obrazku pomoci HOG
**/
Mat KNN_OCR::get_hog_descriptor(Mat preprocesed_image)
{
	//extract HOG feature
	Mat Hogfeat;
	HOGDescriptor hogDescriptor(Size(32, 16), Size(8, 8), Size(4, 4), Size(4, 4), 9);

	vector<float> descriptorsValues;
	vector<Point> locations;

	hogDescriptor.compute(preprocesed_image, descriptorsValues, Size(32, 32), Size(0, 0), locations);
	Hogfeat.create(descriptorsValues.size(), 1, CV_32FC1);

	for (unsigned int j = 0; j < descriptorsValues.size(); j++)
	{
		Hogfeat.at<float>(j, 0) = descriptorsValues.at(j);
	}

	return Hogfeat.reshape(1, 1);
}
//---------------------------------------------------------------------------------------------
/**
*	Funkce image_preprocess predzpracuje vstupni obrazek pro naslednou klasifikaci
**/
Mat KNN_OCR::image_preprocess(Mat input_image)
{
	Mat preprocessed_image;

	// Zmena velikosti obrazku
	Mat resized;
	resize(input_image, resized, Size(64, 48));

	// Pro lepsi vysledky zmena sklonu 
	Mat deskewed = image_deskew(resized);

	// Pro lepsi vysledky prevod do grayscale
	Mat gray_image;
	cvtColor(deskewed, gray_image, CV_BGR2GRAY);

	// Pro lepsi vysledky vyhlazeni gaussianem
	Mat gaussian_image;
	GaussianBlur(gray_image, gaussian_image, Size(3,3), 0, 0);

	// Pro lepsi vysledky aplikace adaptivniho prahovani
	//Mat adap_thres_image;
	//adaptiveThreshold(gaussian_image, adap_thres_image, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 2);

	preprocessed_image = gaussian_image;

	return preprocessed_image;
}
//---------------------------------------------------------------------------------------------
/**
*	Funkce get_data z adresare a podadresaru nacte testovaci obrazky a vytvori klasifikacni tridy 
**/
void KNN_OCR::get_data()
{
	DIR *dp, *fp;
	struct dirent *dirp, *firp;
	char dir_path[255] = "";
	char file_path[255] = "";

	// Nacteni adresaru
	dp = opendir(DATASET_LOCATION);
	while ((dirp = readdir(dp)) != NULL)
	{
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) continue;

		strcat(dir_path, DATASET_LOCATION);
		strcat(dir_path, dirp->d_name);
		strcat(dir_path, "/");

		// Nacteni obrazku v danem adresari
		fp = opendir(dir_path);
		while ((firp = readdir(fp)) != NULL)
		{
			if (!strcmp(firp->d_name, ".") || !strcmp(firp->d_name, "..")) continue;

			// Ziskani jednotlivych trid, trida je reprezentovana ASCII hodnotou znaku
			if (strlen(dirp->d_name) == 1)
			{
				classes.push_back((int)dirp->d_name[0]);
			}
			else
			{
				if (strcmp(dirp->d_name, "comma")) classes.push_back(COMMA);
				else if (strcmp(dirp->d_name, "dash")) classes.push_back(DASH);
				else if (strcmp(dirp->d_name, "period")) classes.push_back(PERIOD);
				else if (strcmp(dirp->d_name, "questionmark")) classes.push_back(QUESTIONMARK);
			}

			strcat(file_path, dir_path);
			strcat(file_path, firp->d_name);
			string str(file_path);
			images.push_back(str);

			file_path[0] = '\0';
		}
		closedir(fp);
		dir_path[0] = '\0';
	}

	closedir(dp);
}
//---------------------------------------------------------------------------------------------
/**
*	Funkce prepare_KNN ziska pro kazdy obrazek HOG priznak a vlozi jej do trenovacich dat
**/
void KNN_OCR::prepare_KNN()
{
	Mat src_image, preprocessed_image, hogDescriptor;

	// Naplneni trid(labels) pro obrazky
	trainClasses = Mat(classes).reshape(0, classes.size());

	for (unsigned int i = 0; i < images.size(); i++)
	{
		src_image = imread(images.at(i), CV_LOAD_IMAGE_COLOR);

		// Preprocess file
		preprocessed_image = image_preprocess(src_image);

		// Get HOG feature vector
		hogDescriptor = get_hog_descriptor(preprocessed_image);

		trainData.push_back(hogDescriptor);
	}

	// for SVM - inicializace SVM klasifikatoru
	params.svm_type = CvSVM::C_SVC;
	params.kernel_type = CvSVM::LINEAR;
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);

}
//---------------------------------------------------------------------------------------------
/**
*	Funkce train natrenuje KNN klasifikator na trenovacich datech a klasifikacnich tridach
**/
void KNN_OCR::train()
{
	knn.train(trainData, trainClasses);
	K = knn.get_max_k() / 2;

	// for SVM
	svm.train(trainData, trainClasses, Mat(), Mat(), params);
}
//---------------------------------------------------------------------------------------------
/**
*	Funkce classify klasifikuje vstupni obrazek pomoci KNN a ziskaneho HOG vectoru
**/
float KNN_OCR::classify(Mat input_image)
{
	Mat inputImageDescriptor, hogDescriptor;

	Mat preprocessed_image;
	preprocessed_image = image_preprocess(input_image);

	// Get HOG feature vector
	hogDescriptor = get_hog_descriptor(preprocessed_image);

	inputImageDescriptor.push_back(hogDescriptor);

	return knn.find_nearest(inputImageDescriptor, K);;
}
//---------------------------------------------------------------------------------------------
/**
*	for SVM
**/
int KNN_OCR::SVMclassify(Mat input_image)
{
	Mat inputImageDescriptor, hogDescriptor;

	Mat preprocessed_image;
	preprocessed_image = image_preprocess(input_image);

	// Get HOG feature vector
	hogDescriptor = get_hog_descriptor(preprocessed_image);

	inputImageDescriptor.push_back(hogDescriptor);

	return svm.predict(inputImageDescriptor);
}


//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	char* imageName = argv[1];
	float result;
	int svmResult;

	Mat image;
	image = imread(imageName, 1);

	KNN_OCR ocr;

	result = ocr.classify(image);
	printf("KNN says: %d == '%c'\n", (int)result, (int)result);
	
	// for SVM
	svmResult = ocr.SVMclassify(image);
	printf("SVM says: %d == '%c'\n", (int)svmResult, (int)svmResult);

	return EXIT_SUCCESS;
}
