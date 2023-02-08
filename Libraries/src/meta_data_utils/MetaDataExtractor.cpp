//
// Created by serkan on 04.02.2023.
//

#include "MetaDataExtractor.hpp"

MetaDataExtractor::MetaDataExtractor(const std::string &prototxt_path,
                                     const std::string &model_path, const std::string &shape_path,
                                     float conf_th) {
    face_detector = cv::dnn::readNetFromCaffe(prototxt_path, model_path);
    if (face_detector.empty()) {
        std::cerr << "Can't load face detector model by using the following files: " << std::endl;
        std::cerr << "prototxt:   " << prototxt_path << std::endl;
        std::cerr << "caffemodel: " << model_path << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        dlib::deserialize(shape_path) >> shape_predictor;
    }
    catch (dlib::serialization_error e) {
        std::cerr << "Can't load landmark localization model by using the following files: " << std::endl;
        std::cerr << "landmark localization model path: " << shape_path << std::endl;
        exit(EXIT_FAILURE);
    }
    this->conf_th = conf_th;
}

int MetaDataExtractor::detect_face(const cv::Mat &frame,
                                   FaceInfo &face_info) {
    if (frame.empty()) return -1;

    cv::Mat resized_img;
    cv::resize(frame, resized_img, cv::Size(300, 300));

    cv::Mat input_blob = cv::dnn::blobFromImage(resized_img, 1, cv::Size(300, 300), cv::Scalar(104, 177, 123), false);
    face_detector.setInput(input_blob);

    cv::Mat detection = face_detector.forward();
    cv::Mat detection_mat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

    float best_conf = 0, best_conf_idx = 0;
    for (int i = 0; i < detection_mat.rows; ++i) {
        float conf = detection_mat.at<float>(i, 2);
        if (conf > best_conf) {
            best_conf = conf;
            best_conf_idx = i;
        }
    }

    if (best_conf >= conf_th) {
        face_info.start_x = static_cast<int>(detection_mat.at<float>(best_conf_idx, 3) * frame.size[1]);
        face_info.start_y = static_cast<int>(detection_mat.at<float>(best_conf_idx, 4) * frame.size[0]);
        face_info.end_x = static_cast<int>(detection_mat.at<float>(best_conf_idx, 5) * frame.size[1]);
        face_info.end_y = static_cast<int>(detection_mat.at<float>(best_conf_idx, 6) * frame.size[0]);
        cv::rectangle(frame, cv::Point(face_info.start_x, face_info.start_y), cv::Point(face_info.end_x, face_info.end_y), cv::Scalar(255, 255 ,255));

        return 1;
    }
    return -1;
}

int
MetaDataExtractor::detect_landmarks(const cv::Mat &frame,
                                    FaceInfo &face_info,
                                    dlib::full_object_detection &landmark_out) {
    try {
        dlib::cv_image<dlib::bgr_pixel> dlib_img(frame);
        dlib::rectangle face_rect(face_info.start_x, face_info.start_y, face_info.end_x, face_info.end_y);
        landmark_out = shape_predictor(dlib_img, face_rect);
    }
    catch (std::exception &exception) {
        std::cout << "Exception thrown at detecting facial landmarks!\n" << exception.what() << "\n";
        return -1;
    }
    return 1;
}


int
MetaDataExtractor::extract_roi(const cv::Mat &frame,
                               std::vector<cv::Mat> &roi_out) {
    if (frame.empty()) return -1;

    FaceInfo face_info;
    if (detect_face(frame, face_info) == -1) {
        std::cout << "Unable to detect face, terminating metadata generating operations!\n";
        return -1;
    }

    dlib::full_object_detection landmarks;
    if (detect_landmarks(frame, face_info, landmarks) == -1) {
        std::cout << "Unable to detect facial landmarks, terminating metadata generating operations!\n";
        return -1;
    }
    try {
        roi_out.clear();
        roi_out.reserve(3);

        int face_height = abs(face_info.end_y - face_info.start_y);
        int added_height = static_cast<int>(face_height * face_bbox_enhancement_scale_factor);

        cv::Rect ROI1Coords(landmarks.part(54).x(),
                            landmarks.part(29).y() + added_height,
                            landmarks.part(12).x() - landmarks.part(54).x(),
                            landmarks.part(33).y() - landmarks.part(29).y());
        roi_out.emplace_back(frame(ROI1Coords));

        cv::Rect ROI2Coords(landmarks.part(4).x(),
                            landmarks.part(29).y() + added_height,
                            landmarks.part(48).x() - landmarks.part(4).x(),
                            landmarks.part(33).y() - landmarks.part(29).y());
        roi_out.emplace_back(frame(ROI2Coords));

        int roi3_height = static_cast<int>((landmarks.part(25).y() - face_info.start_y) * 0.8);
        cv::Rect ROI3Coords(landmarks.part(18).x(),
                            face_info.start_y + roi3_height * 0.2,
                            landmarks.part(25).x() - landmarks.part(18).x(),
                            roi3_height);

        cv::rectangle(frame, ROI1Coords, cv::Scalar(255, 255 ,255));
        cv::rectangle(frame, ROI2Coords, cv::Scalar(255, 255 ,255));
        cv::rectangle(frame, ROI3Coords, cv::Scalar(255, 255 ,255));

        roi_out.emplace_back(frame(ROI3Coords));
    }
    catch (std::exception &exception) {
        std::cout << "Error occurred during roi extraction, terminating metadata generating operations!\n";
        return -1;
    }
    return 1;
}

double
MetaDataExtractor::calculate_roi_green_mean(const cv::Mat &roi) {
    std::vector<cv::Mat> channels;
    cv::split(roi, channels);
    return cv::mean(channels[1])[0];
}

void
MetaDataExtractor::write_vector_to_file(const std::vector<double>& myVector,
                                        const std::string &filename)
{

    std::ofstream output_file(filename);
    if (!output_file.is_open()){
        std::cout << "Failed to open/write file: " << filename << "\n";
        return;
    }
    std::ostream_iterator<double> output_iterator(output_file, "\n");
    std::copy(std::begin(myVector), std::end(myVector), output_iterator);
    output_file.close();
}

int
MetaDataExtractor::extract_metadata(const std::vector<cv::Mat> &frame_buffer,
                                    double sampling_rate,
                                    const std::string &save_path,
                                    const std::string &file_name)
{
    int frame_buffer_size = frame_buffer.size();

    std::vector<double> raw_roi1(frame_buffer_size);
    std::vector<double> raw_roi2(frame_buffer_size);
    std::vector<double> raw_roi3(frame_buffer_size);

    std::vector<cv::Mat> roi_out;
    for (int i = 0; i < frame_buffer_size; ++i) {
        if (extract_roi(frame_buffer[i], roi_out) == -1) {
            return -1;
        }
        raw_roi1[i] = calculate_roi_green_mean(roi_out[0]);
        raw_roi2[i] = calculate_roi_green_mean(roi_out[1]);
        raw_roi3[i] = calculate_roi_green_mean(roi_out[2]);

        cv::imshow("Meta Data Extraction", frame_buffer[i]);
        if (cv::waitKey(1) == 27) {
            return -1;
        }
    }
    cv::destroyAllWindows();

    write_vector_to_file(raw_roi1, save_path + file_name + "_" + std::to_string(static_cast<int>(sampling_rate)) + "roi1.txt");
    write_vector_to_file(raw_roi2, save_path + file_name + "_" + std::to_string(static_cast<int>(sampling_rate)) + "roi2.txt");
    write_vector_to_file(raw_roi3, save_path + file_name + "_" + std::to_string(static_cast<int>(sampling_rate)) + "roi3.txt");

    return 1;
}


