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

    // generate random colors for class labels
    static std::vector<cv::Scalar> getColors(std::vector<std::string>& labels)
    {
        std::vector<cv::Scalar> colors; // vector of colors for our class labels
        // select random colors for our class labels
        for (int i = 0; i < labels.size(); i++)
        {
            colors.emplace_back(randomColor());
        }
        return colors;
    }

    // get output layer names
    static std::vector<std::string> getOutputLayerNames(const cv::dnn::Net& net)
    {
        std::vector<std::string> layerNames = net.getLayerNames();
        std::vector<std::string> outputLayers;
        for (auto i : net.getUnconnectedOutLayers())
        {
            outputLayers.push_back(layerNames[i - 1]);
        }
        return outputLayers;
    }

    // postprocessing - get the bounding boxes, confidence scores and class IDs
    // from the model output
    static void postProcess(std::vector<cv::Mat> &layerOutputs, cv::Mat &image, float conf, float thresh, 
        std::vector<std::string> &labels, std::vector<cv::Scalar> &colors)
    {
        // initialize our lists of detected bounding boxes, confidences and
        // class IDs, respectively
        std::vector<cv::Rect> boxes;
        std::vector<float> confidences;
        std::vector<int> classIds;
        
        // loop over each of the layer outputs
        // https://github.com/opencv/opencv/blob/master/samples/dnn/object_detection.cpp lines 368-391
        for (size_t i = 0; i < layerOutputs.size(); ++i)
        {
            float* data = (float*)layerOutputs[i].data;
            for (int j = 0; j < layerOutputs[i].rows; ++j)
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
                data += layerOutputs[i].cols;
            }
        }

        // apply non-maxima suppression to suppress weak, overlapping bounding
        // boxes
        std::vector<int> idxs;
        cv::dnn::NMSBoxes(boxes, confidences, conf,
            thresh, idxs); 

        // ensure at least one detection exists
        if (idxs.size() > 0)
        {
            // loop over the indexes we are keeping
            for (int i : idxs)
            {
                int classId = classIds[i];
                drawPrediction(image, boxes[i], colors[classId], labels[classId], confidences[i]);
            }
        }
    }

    static void drawPrediction(cv::Mat &image, cv::Rect box, cv::Scalar color, std::string label,
        float confidence)
    {
        // draw a bounding box rectangle and label on the image
        cv::rectangle(image, cv::Rect(box.x, box.y, box.width, box.height), color, 2);
        std::ostringstream stringstream;
        stringstream << label << ": " << cv::format("%.3f", confidence);
        std::string text = stringstream.str();
        cv::putText(image, text, cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX,
            0.5, color, 2);
    }
};
#endif // !HELPER

