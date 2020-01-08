#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <Windows.h>
#include <time.h>
using namespace std;

#ifdef _WIN32
#pragma once
#include <opencv2/core/version.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define CV_VERSION_ID CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) \
  CVAUX_STR(CV_SUBMINOR_VERSION)

#ifdef _DEBUG
#define cvLIB(name) "opencv_" name CV_VERSION_ID "d"
#else
#define cvLIB(name) "opencv_" name CV_VERSION_ID
#endif //_DEBUG

//#pragma comment( lib, cvLIB("core") )
//#pragma comment( lib, cvLIB("imgproc") )
//#pragma comment( lib, cvLIB("highgui") )

#endif //_WIN32

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "face_identification.h"
#include "recognizer.h"
#include "face_detection.h"
#include "face_alignment.h"
#include "math_functions.h"

using namespace seeta;

/* Whether th e file [name] exist*/
bool exists(const char * name) {
	struct stat buffer;
	return (stat(name, &buffer) == 0);
}

/* Returns a list of files in a directory (except the ones that begin with a dot) */
void GetFilesInDirectory(std::vector<string> &out, const string &directory)
{
	HANDLE dir;
	WIN32_FIND_DATA file_data;

	if ((dir = FindFirstFile((directory + "/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		return; /* No files found */

	do {
		const string file_name = file_data.cFileName;
		const string full_file_name = directory + "\\\\" + file_name;
		const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (file_name[0] == '.')
			continue;

		if (is_directory)
			continue;

		out.push_back(full_file_name);
	} while (FindNextFile(dir, &file_data));

	FindClose(dir);
} // GetFilesInDirectory

int main(int argc, char* argv[]) {
	//check arguments
	/*exe
	**dataset_dir
	**standard_image
	**min_similarity
	**result_dir.
	D:\\sheldon D:\\standard\\58.jpg 0.6 D:\\sheldon_after_filtering
	D:\\Dataset_pb_crawl_from_internet\\1 D:\\Dataset_pb_crawl_from_internet\\standard\\1.jpg 0.6 D:\\Dataset_pb_crawl_from_internet\\1_after_filtering
	*/
	if (argc < 5) {
		cout << "Usage: " << argv[0] << " dataset_dir standard_image min_similarity result_dir" << endl;
		system("pause");
		return 0;
	}
	//Check whether Detection & Align & Identify model existed
	if (!exists("seeta_fd_frontal_v1.0.bin")) {
		cout << "SeetaFace Detection Model(seeta_fd_frontal_v1.0.bin) do not existed in current directory!" << endl;
		system("pause");
		return 0;
	}
	if (!exists("seeta_fa_v1.1.bin")) {
		cout << "SeetaFace Alignment Model(seeta_fa_v1.1.bin) do not existed in current directory!" << endl;
		system("pause");
		return 0;
	}
	if (!exists("seeta_fr_v1.0.bin")) {
		cout << "SeetaFace Identification Model(seeta_fr_v1.0.bin) do not existed in current directory!" << endl;
		system("pause");
		return 0;
	}
	// Initialize face detection model
	seeta::FaceDetection detector("seeta_fd_frontal_v1.0.bin");
	detector.SetMinFaceSize(40);
	detector.SetScoreThresh(2.f);
	detector.SetImagePyramidScaleFactor(0.8f);
	detector.SetWindowStep(4, 4);

	// Initialize face alignment model 
	seeta::FaceAlignment point_detector("seeta_fa_v1.1.bin");

	// Initialize face Identification model 
	FaceIdentification face_recognizer("seeta_fr_v1.0.bin");

	bool have_done = false;
	float gallery_fea[2048];

	if (exists("standard_image_info_feature_2.txt")) {
		ifstream in_st_fea;
		in_st_fea.open("standard_image_info_feature_2.txt", ios::in);
		string tmp_str;
		in_st_fea >> tmp_str;
		if (tmp_str == argv[2]) {
			have_done = true;
			for (int i = 0; i < 2048; i++) {
				in_st_fea >> gallery_fea[i];
			}
		}
		in_st_fea.close();
	}
	if (!have_done) {
		//load standard image
		cv::Mat gallery_img_color = cv::imread(argv[2], 1);
		cv::Mat gallery_img_gray;
		cv::cvtColor(gallery_img_color, gallery_img_gray, CV_BGR2GRAY);

		ImageData gallery_img_data_color(gallery_img_color.cols, gallery_img_color.rows, gallery_img_color.channels());
		gallery_img_data_color.data = gallery_img_color.data;

		ImageData gallery_img_data_gray(gallery_img_gray.cols, gallery_img_gray.rows, gallery_img_gray.channels());
		gallery_img_data_gray.data = gallery_img_gray.data;

		// Detect faces
		std::vector<seeta::FaceInfo> gallery_faces = detector.Detect(gallery_img_data_gray);
		int32_t gallery_face_num = static_cast<int32_t>(gallery_faces.size());

		if (gallery_face_num <= 0) {
			std::cout << "In standard image, Faces are not detected." << endl;
			return 0;
		}

		if (gallery_face_num > 1) {
			std::cout << "In standard image, Faces deteced number is more than one." << endl;
			return 0;
		}

		// Detect 5 facial landmarks
		seeta::FacialLandmark gallery_points[5];
		point_detector.PointDetectLandmarks(gallery_img_data_gray, gallery_faces[0], gallery_points);

		// Extract face identity feature

		face_recognizer.ExtractFeatureWithCrop(gallery_img_data_color, gallery_points, gallery_fea);

		ofstream save_st_fea;
		save_st_fea.open("standard_image_info_feature_2.txt", ios::out);
		save_st_fea << argv[2] << endl;
		for (int i = 0; i < 2048; i++) {
			save_st_fea << gallery_fea[i] << " ";
		}
		save_st_fea.close();
	}

	const string data_dir = argv[1];
	vector<string> data_set;
	GetFilesInDirectory(data_set, data_dir);

	string result_dir = argv[4];
	if (result_dir.at(result_dir.length() - 1) != '\\') {
		result_dir = result_dir.append("\\\\");
	}
	CreateDirectory(result_dir.c_str(), NULL);
	CreateDirectory((result_dir + "src\\").c_str(), NULL);
	CreateDirectory((result_dir + "src_crop\\").c_str(), NULL);
	double min_similarity = atof(argv[3]);

	int i = 0;
	if (exists("log_of_2.txt")) {
		ifstream ifile;
		ifile.open("log_of_2.txt", ios::in);
		ifile >> i;
		ifile.close();
	}

	cout << "start from " + i << endl;

	ofstream ofile;
	ofile.open("log_of_2.txt", ios::out);

	for (; i < data_set.size(); i++) {
		string filename = data_set[i].substr(data_set[i].rfind('\\') + 1);
		if (exists((result_dir + "src_crop\\" + filename).c_str())) {
			cout << filename << " have done before" << endl;
			continue;
		}
		cout << data_set[i] << endl;
		//load example image
		cv::Mat probe_img_color = cv::imread(data_set[i].c_str(), 1);
		cv::Mat probe_img_gray;
		try {
			cv::cvtColor(probe_img_color, probe_img_gray, CV_BGR2GRAY);
		}
		catch (exception e) {
			cout << data_set[i] + " " + e.what() << endl;
			continue;
		}

		ImageData probe_img_data_color(probe_img_color.cols, probe_img_color.rows, probe_img_color.channels());
		probe_img_data_color.data = probe_img_color.data;

		ImageData probe_img_data_gray(probe_img_gray.cols, probe_img_gray.rows, probe_img_gray.channels());
		probe_img_data_gray.data = probe_img_gray.data;

		//detect faces
		std::vector<seeta::FaceInfo> probe_faces = detector.Detect(probe_img_data_gray);
		int32_t probe_face_num = static_cast<int32_t>(probe_faces.size());

		if (probe_face_num < 1) {
			continue;
		}
		seeta::FacialLandmark probe_points[5];
		point_detector.PointDetectLandmarks(probe_img_data_gray, probe_faces[0], probe_points);

		// Create a image to store crop face.
		cv::Mat dst_img(face_recognizer.crop_height(),
			face_recognizer.crop_width(),
			CV_8UC(face_recognizer.crop_channels()));
		ImageData dst_img_data(dst_img.cols, dst_img.rows, dst_img.channels());
		dst_img_data.data = dst_img.data;
		face_recognizer.CropFace(probe_img_data_color, probe_points, dst_img_data);

		// Extract face identity feature
		float probe_fea[2048];
		face_recognizer.ExtractFeature(dst_img_data, probe_fea);

		// Caculate similarity of two faces
		float sim = face_recognizer.CalcSimilarity(gallery_fea, probe_fea);

		if (sim > min_similarity) {
			clock_t t = clock();
			cout << t / CLOCKS_PER_SEC << "s: " << data_set[i] << endl;
			CopyFile(data_set[i].c_str(), (result_dir + "src\\" + filename).c_str(), false);
			cv::imwrite((result_dir + "src_crop\\" + filename).c_str(), dst_img);
		}

		ofile.clear();
		ofile.seekp(0, ofile.beg);
		ofile << i << endl;

	}
	ofile.close();
	return 0;
}