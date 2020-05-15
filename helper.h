// Helper.h
// helper functions for object_detection.cpp

#ifndef HELPER
#define HELPER

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

class Helper {
public:
    // random color generator
    static cv::Scalar randomColor() {
        return cv::Scalar(rand() % 255, rand() % 255, rand() % 255);
    }

    // get output layer names
    static std::vector<std::string> getOutputLayerNames(const cv::dnn::Net& net)
    {
        std::vector<std::string> layerNames = net.getLayerNames();
        std::vector<std::string> outputLayers;
        for (auto i : net.getUnconnectedOutLayers())
        {
            outputLayers.push_back(layerNames[(double)i - 1]);
        }
        return outputLayers;
    }

    // postprocessing - get the bounding boxes, confidence scores and class IDs
    // from the model output
    static void postProcess(std::vector<cv::Mat>& layerOutputs, cv::Mat& image, std::vector<cv::Rect>& boxes,
        std::vector<float>& confidences, std::vector<int>& classIds, float conf)
    {
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
                if (confidence > conf)
                {
                    int centerX = (int)(data[0] * image.cols);
                    int centerY = (int)(data[1] * image.rows);
                    int width = (int)(data[2] * image.cols);
                    int height = (int)(data[3] * image.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float)confidence);
                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
        }
    }
};
#endif // !HELPER

