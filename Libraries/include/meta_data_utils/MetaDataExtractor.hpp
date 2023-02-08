//
// Created by serkan on 04.02.2023.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <algorithm>


/*
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
*/
#include <opencv2/opencv.hpp>

#include <dlib/dnn.h>
#include <dlib/data_io.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/opencv/cv_image.h>

#ifndef SMARTIR_YFTS_TEST_METADATAEXTRACTOR_HPP
#define SMARTIR_YFTS_TEST_METADATAEXTRACTOR_HPP

typedef struct {
    int start_x;
    int start_y;
    int end_x;
    int end_y;
} FaceInfo;


class MetaDataExtractor {
public:

    MetaDataExtractor(const std::string &prototxt_path, const std::string &model_path, const std::string &shape_path,
                      float conf_th);

    void write_vector_to_file(const std::vector<double>& myVector, const std::string &filename);

    int extract_metadata(const std::vector<cv::Mat> &frame_buffer, double sampling_rate, const std::string &save_path,
                         const std::string &filename);

private:
    int detect_face(const cv::Mat &frame, FaceInfo &face_info);

    int detect_landmarks(const cv::Mat &frame, FaceInfo &face_info,
                         dlib::full_object_detection &landmark_out);

    int extract_roi(const cv::Mat &frame, std::vector<cv::Mat> &roi_out);

    double calculate_roi_green_mean(const cv::Mat &roi);

    cv::dnn::Net face_detector;
    dlib::shape_predictor shape_predictor;
    float conf_th;
    float face_bbox_enhancement_scale_factor = 0.05f;

};


#endif //SMARTIR_YFTS_TEST_METADATAEXTRACTOR_HPP
