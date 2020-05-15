// pyimagesearch-YOLO-ObjectDetection.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// parse the command line arguments
const std::string keys =
"{help h usage ? || usage: object_detection.exe --image <path_to_image> --yolo <path_to_yolo_directory> --confidence 0.5 --threshold 0.3}"
"{@image i || path to image file}"
"{@yolo d || path to yolo model configuration directory}"
"{confidence c |0.5| minimum probability to filter weak detections}"
"{threshold t |0.3| threshold when applying non-maximum suppression}";


// random color generator
static cv::Scalar randomColor() {
    return cv::Scalar(rand() % 255, rand() % 255, rand() % 255);
}

// get output names
static std::vector<std::string> getOutputLayerNames(const cv::dnn::Net& net)
{
    std::vector<std::string> layerNames = net.getLayerNames();
    std::vector<std::string> outputLayers;
    for ( auto i : net.getUnconnectedOutLayers())
    {
        outputLayers.push_back(layerNames[(double)i - 1]);
    }
    return outputLayers;
}

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
    std::vector<cv::Scalar> colors; // vector of colors for our class labels
    
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

    // select random colors for our class labels
    for (int i = 0; i < labels.size(); i++)
    {
        colors.emplace_back(randomColor());
    }

    // derive that paths to the weights and model configuration
    weightsPath = yoloPath + "yolov3.weights";
    configPath = yoloPath + "yolov3.cfg";

    // load our YOLO object detector train on COCO dataset (80 classes)
    std::cout << "loading YOLO from disk.." << std::endl;
    cv::dnn::Net net = cv::dnn::readNetFromDarknet(configPath, weightsPath);

    // load our input image and grab its spatial dimensions
    if (!parser.has("@image"))
    {
        parser.printMessage();
        return 0;
    }

    std::string imagePath = parser.get<std::string>("@image");
    cv::Mat image;
    try
    {
        image = cv::imread(imagePath);
    }
    catch (...)
    {
        std::cout << "unable to load image at: " << imagePath << std::endl;
    }

    auto height = image.size().height;
    auto width = image.size().width;

    // determine only the *output* layer nanes that we need from YOLO
    std::vector<std::string> ln = getOutputLayerNames(net);

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

    // initialize our lists of detected bounding boxes, confidences and
    // class IDs, respectively
    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;
    std::vector<int> classIDs;

    // loop over each of the layer outputs
    for (size_t i = 0; i < layerOutputs.size(); ++i)
    {
        float* data = (float*)layerOutputs[i].data;
        for (int j = 0; j < layerOutputs[i].rows; ++j, data += layerOutputs[i].cols)
        {
            cv::Mat scores = layerOutputs[i].row(j).colRange(5, layerOutputs[i].cols);
            cv::Point classIdPoint;
            double confidence;
            // get the value and location of the maximum score
            cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > parser.get<float>("confidence"))
            {
                int centerX = (int)(data[0] * image.cols);
                int centerY = (int)(data[1] * image.rows);
                int width = (int)(data[2] * image.cols);
                int height = (int)(data[3] * image.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIDs.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    }

    // apply non-maxima suppression to suppress weak, overlapping bounding
    // boxes
    std::vector<int> idxs;
    cv::dnn::NMSBoxes(boxes, confidences, parser.get<float>("confidence"),
        parser.get<float>("threshold"), idxs);

    // ensure at least one detection exists
    if (idxs.size() > 0)
    {
        // loop over the indexes we are keeping
        for (int i : idxs)
        {
            // extract the bounding box coordinates
            auto x = boxes[i].x;
            auto y = boxes[i].y;
            auto w = boxes[i].width;
            auto h = boxes[i].height;

            // draw a bounding box rectangle and label on the image
            cv::Scalar color = colors[classIDs[i]];
            cv::rectangle(image, cv::Rect(x, y, w, h), color, 2);
            std::ostringstream stringstream;
            stringstream << labels[classIDs[i]] << ": " << cv::format("%.3f", confidences[i]);
            std::string text = stringstream.str();
            cv::putText(image, text, cv::Point(x, y - 5), cv::FONT_HERSHEY_SIMPLEX,
                0.5, color, 2);
        }
    }

    // show the image output
    cv::imshow("Image", image);
    cv::waitKey(0);

    return 0;
}