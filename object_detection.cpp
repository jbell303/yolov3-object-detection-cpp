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

// include message queue
#include "message_queue.h"

// parse the command line arguments
const std::string keys =
"{help h usage ? || usage: object_detection.exe --image <path_to_image> --yolo <path_to_yolo_directory> --confidence 0.5 --threshold 0.3}"
"{@image i || path to image file}"
"{@video v || path to input video}"
"{@output o || path to output video}"
"{@yolo d || path to yolo model configuration directory}"
"{confidence c |0.5| minimum probability to filter weak detections}"
"{threshold t |0.3| threshold when applying non-maximum suppression}"
"{async a |true| use async protocol}";

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

    // generate bounding box colors for each class
    std::vector<cv::Scalar> colors = Helper::getColors(labels);

    // derive that paths to the weights and model configuration
    weightsPath = yoloPath + "yolov3.weights";
    configPath = yoloPath + "yolov3.cfg";

    // load our YOLO object detector train on COCO dataset (80 classes)
    std::cout << "[INFO] loading YOLO from disk.." << std::endl;
    cv::dnn::Net net = cv::dnn::readNetFromDarknet(configPath, weightsPath);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA); // use GPU if available, reverts to CPU
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

    // determine only the *output* layer nanes that we need from YOLO
    std::vector<std::string> ln = Helper::getOutputLayerNames(net);

    // initialize image, video capture and video writer
    cv::Mat image;
    cv::VideoCapture vs;
    cv::VideoWriter writer;
    std::string output;
    int totalFrames;

    if (parser.has("@image"))
    {
        std::string imagePath = parser.get<std::string>("@image");
        try
        {
            image = cv::imread(imagePath);
            totalFrames = 1;
        }
        catch (...)
        {
            std::cout << "unable to load image at: " << imagePath << std::endl;
            return 0;
        }
    }
    else if (parser.has("@video") && parser.has("@output"))
    {
        std::string videoPath = parser.get<std::string>("@video");
        try
        {
            vs = cv::VideoCapture(videoPath);
            totalFrames = vs.get(cv::CAP_PROP_FRAME_COUNT);
        }
        catch (...)
        {
            std::cout << "unable to load video at: " << videoPath << std::endl;
            return 0;
        }
        auto fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
        output = parser.get<std::string>("@output");
        try
        {
            writer = cv::VideoWriter(output, fourcc, 30,
                cv::Size(vs.get(cv::CAP_PROP_FRAME_WIDTH), vs.get(cv::CAP_PROP_FRAME_HEIGHT)));
        }
        catch (...)
        {
            std::cout << "unable to create video at: " << output << std::endl;
            return 0;
        }
        std::cout << "[INFO] processing video file: " << output << std::endl;
    }
    else
    {
        parser.printMessage();
        return 0;
    }

    int frameCnt = 0;
    auto startTime = std::chrono::steady_clock::now();
    bool async = parser.get<bool>("async");

    // async protocol from OpenCV sample, though some implemenation varies
    // https://github.com/opencv/opencv/blob/master/samples/dnn/object_detection.cpp
    if (async)
    {
        bool process = true;

        // frames capture thread
        std::shared_ptr<MessageQueue<cv::Mat>> framesQueue(new MessageQueue<cv::Mat>);
        std::thread framesThread([&]() {
            cv::Mat frame;
            while (process)
            {
                // grab the image parsed earlier or grab a frame from video capture
                if (parser.has("@image"))
                    frame = image;
                else
                    vs >> frame;

                if (!frame.empty())
                {
                    framesQueue->send(std::move(frame));
                }
                else
                    break;
            }
        });

        // frames processing thread
        std::shared_ptr<MessageQueue<cv::Mat>> processedFramesQueue(new MessageQueue<cv::Mat>);
        std::shared_ptr<MessageQueue<std::vector<cv::Mat>>> predictionsQueue(new MessageQueue<std::vector<cv::Mat>>);
        std::thread processingThread([&]() {
            while (process)
            {
                // get next frame
                cv::Mat frame;
                if (frameCnt < totalFrames)
                    frame = framesQueue->receive();

                // process the frame
                if (!frame.empty())
                {
                    cv::Mat blob = cv::dnn::blobFromImage(frame, 1 / 255.0, cv::Size(416, 416), cv::Scalar(0, 0, 0), true, false);
                    net.setInput(blob);
                    std::vector<cv::Mat> layerOutputs;
                    net.forward(layerOutputs, ln);
                    predictionsQueue->send(std::move(layerOutputs));
                    processedFramesQueue->send(std::move(frame));
                    frame.release();
                    blob.release();
                }
            }
        });

        // postprocessing and rendering loop
        while (cv::waitKey(1) < 0)
        {
            frameCnt++;

            // if we have processed all the frames, break
            if (frameCnt > totalFrames)
            {
                auto endTime = std::chrono::steady_clock::now();
                float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
                std::cout << "[INFO] Done processing: " << totalFrames << " frames in " << elapsed / 1000 << " seconds" << std::endl;
                std::cout << "[INFO] Video created: " << output << std::endl;
                break;
            }

            std::cout << "processing frame: " << frameCnt << " of: " << totalFrames << std::endl;

            // get current frame and predictions
            std::vector<cv::Mat> outs = predictionsQueue->receive();
            cv::Mat frame = processedFramesQueue->receive();

            // get the minimum confidence and NMS threshold
            float conf = parser.get<float>("confidence");
            float thresh = parser.get<float>("threshold");

            Helper::postProcess(outs, frame, conf, thresh, labels, colors);

            // if single image was provided, exit the loop after first pass
            if (parser.has("@image"))
            {
                auto endTime = std::chrono::steady_clock::now();
                float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
                std::cout << "[INFO] Done processing in: " << elapsed / 1000 << " seconds" << std::endl;
                break;
            }


            // write image to output file
            cv::Mat convertedFrame;
            frame.convertTo(convertedFrame, CV_8U);
            writer.write(convertedFrame);
            cv::imshow("Image", frame);
        }

        process = false;
        framesThread.join();
        processingThread.join();

    }
    else
    {
        while (cv::waitKey(1) < 0)
        {
            frameCnt++;

            // capture the image
            if (parser.has("@video"))
                vs >> image;

            if (image.empty())
            {
                std::cout << "[INFO] Done processing video: " << output << std::endl;
                break;
            }
            // construct a blob from the input image and then perform a forward
            // pass of the YOLO object detector, giving us our bounding boxes and
            // associated probabilities

            cv::Mat blob = cv::dnn::blobFromImage(image, 1 / 255.0, cv::Size(416, 416), cv::Scalar(0, 0, 0), true, false);
            net.setInput(blob);
            auto start = std::chrono::steady_clock::now();
            std::vector<cv::Mat> layerOutputs;
            net.forward(layerOutputs, ln);
            auto end = std::chrono::steady_clock::now();

            // show timing information on YOLO
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "processed frame: " << frameCnt << " of " << totalFrames << " in " << ms / 1000.0 << " seconds" << std::endl;

            // get the minimum confidence and NMS threshold
            float conf = parser.get<float>("confidence");
            float thresh = parser.get<float>("threshold");

            Helper::postProcess(layerOutputs, image, conf, thresh, labels, colors);

            // if single image was provided, exit the loop after first pass
            if (parser.has("@image"))
                break;

            // write image to output file
            cv::Mat frame;
            image.convertTo(frame, CV_8U);
            writer.write(frame);
            cv::imshow("Image", image);
        }
    }

    // if input was an image, display the image
    if (parser.has("@image"))
    {
        // show the image output
        cv::imshow("Image", image);
        cv::waitKey(0);
    } 
    else if (parser.has("@video"))
    {
        auto endTime = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        std::cout << "[INFO] Done processing: " << totalFrames << " frames in " << elapsed / 1000 << " seconds" << std::endl;
        std::cout << "[INFO] cleaning up..." << std::endl;
        writer.release();
        vs.release();
    }
    return 0;
}

