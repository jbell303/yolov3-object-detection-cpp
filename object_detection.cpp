// pyimagesearch-YOLO-ObjectDetection.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// include helper file
#include "helper.h"

// parse the command line arguments
const std::string keys =
"{help h usage ? || usage: object_detection.exe --image <path_to_image> --yolo <path_to_yolo_directory> --confidence 0.5 --threshold 0.3}"
"{@image i || path to image file}"
"{@video v || path to input video}"
"{@output o || path to output video}"
"{@yolo d || path to yolo model configuration directory}"
"{confidence c |0.5| minimum probability to filter weak detections}"
"{threshold t |0.3| threshold when applying non-maximum suppression}";

int main(int argc, char** argv)
{
    // parse the command line arguments
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("Use this script to run object detection using YOLOv3 in OpenCV.");
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
 
    std::string yoloPath; // path to the YOLO directory
    std::string labelsPath; // path to the class labels
    std::string weightsPath; // path to the weights file
    std::string configPath; // path to the model configuration file
    std::vector<std::string> labels; // class labels for model output
    
    // quit if the yolo directory command arg was not provided
    if (!parser.has("@yolo"))
    {
        parser.printMessage();
        return 0;
    }
    
    // read the labels from the coco.names file
    yoloPath = parser.get<std::string>("@yolo") + "/";
    labelsPath = yoloPath + "coco.names";
    std::ifstream fstream(labelsPath.c_str());
    std::string line;
    while (std::getline(fstream, line))
        labels.emplace_back(line);

    

    // derive that paths to the weights and model configuration
    weightsPath = yoloPath + "yolov3.weights";
    configPath = yoloPath + "yolov3.cfg";

    // load our YOLO object detector train on COCO dataset (80 classes)
    std::cout << "[INFO] loading YOLO from disk.." << std::endl;
    cv::dnn::Net net = cv::dnn::readNetFromDarknet(configPath, weightsPath);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA); // use GPU if available, reverts to CPU
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

    // load our input image and grab its spatial dimensions
    if (!parser.has("@image") && !parser.has("@video"))
    {
        parser.printMessage();
        return 0;
    }

    cv::Mat image;

    if (parser.has("@image"))
    {
        std::string imagePath = parser.get<std::string>("@image");
        try
        {
            image = cv::imread(imagePath);
        }
        catch (...)
        {
            std::cout << "unable to load image at: " << imagePath << std::endl;
        }
    }

    auto height = image.size().height;
    auto width = image.size().width;

    // determine only the *output* layer nanes that we need from YOLO
    std::vector<std::string> ln = Helper::getOutputLayerNames(net);

    /* construct a blob from the input image and then perform a forward
    // pass of the YOLO object detector, giving us our bounding boxes and 
    // associated probabilities */

    cv::Mat blob = cv::dnn::blobFromImage(image, 1 / 255.0, cv::Size(416, 416), cv::Scalar(0, 0, 0), true, false);
    net.setInput(blob);
    auto start = std::chrono::steady_clock::now();
    std::vector<cv::Mat> layerOutputs;
    net.forward(layerOutputs, ln);
    auto end = std::chrono::steady_clock::now();

    // show timing information on YOLO
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "YOLO took " << ms / 1000.0 << " seconds" << std::endl;

    // get the minimum confidence and NMS threshold
    float conf = parser.get<float>("confidence");
    float thresh = parser.get<float>("threshold");

    Helper::postProcess(layerOutputs, image, conf, thresh, labels);

    // show the image output
    cv::imshow("Image", image);
    cv::waitKey(0);

    return 0;
}

