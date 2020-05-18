# YOLOv3 Object Detection C++
#### Udacity C++ Nanodegree Capstone Project (option 1) - Clone of PyImageSearch OpenCV Object Detection Tutorial
![dog](https://github.com/jbell303/yolov3-object-detection-cpp/blob/master/images/dog_processed.PNG)

## About
This is a C++ version of [this](https://www.pyimagesearch.com/2018/11/12/yolo-object-detection-with-opencv/) blog post by [PyImageSearch](https://pyimagesearch.com).
YOLO (You only look once) is an object detection algorithm by [Joseph Redmon](https://pjreddie.com).

## Requirements
**CMake** >=2.8  
**OpenCV** >=3.3 You will need download [OpenCV](https://opencv.org) to build this project. Pre-built binaries can be found [here](opencv.org/releases). Version 3.3 or higher is required to use the DNN module.

**YOLOv3 weights** You can download the YOLOv3 weights file [here](https://pjreddie.com/media/files/yolov3.weights)

## Installation
Clone this repo `git clone https://github.com/jbell303/yolov3-object-detection-cpp.git`  
Place the `yolov3.weights` file downloaded earlier into the `yolo-coco` folder.  

### Linux / Mac
`cd` into the project folder
```
mkdir build && cd build  
cmake .. && make
```

### Windows
`cd` into the project folder
```
mkdir build && cd build
cmake -G "Visual Studio 16 2019" ..
cmake --build . --config Release --target INSTALL
```

## Usage
`./object detector -i<path/to/image> -d <path/to/yolo/directory>`  
You can run object detection on a video with `-v <path/to/video> -o <path/to/output>` instead of `-i`

GPU - OpenCV will try to use the DNN backend with CUDA support if the DNN module was build with CUDA. Otherwise it will revert to CPU.

## Summary
This is a basic script with two helper classes `helper.h` and `message_queue.h`. The `Helper` class abstracts some of the pre and post-processing functions. `MessageQueue` allows for asynchronous execution of image pre-processing which is marginally helpful during video detection. A possible improvement would be to use the `net.forwardAsync()` for forward passes through the network, but the backend of my OpenCV build did not support it. 

## References
This project is similar to the OpenCV Object Detection Tutorial [here](https://github.com/opencv/opencv/blob/master/samples/dnn/object_detection.py) and [this](https://www.learnopencv.com/deep-learning-based-object-detection-using-yolov3-with-opencv-python-c/) tutorial.

Notably, this project attempts to use the async protocol from the OpenCV tutorial, however, I could not get async to process video files properly, so I changed the `Message Queue` class to that of the Concurrency module in the Udacity course.


