# YOLOv3 Object Detection C++ with OpenCV
#### Udacity C++ Nanodegree Capstone Project (option 1) - Deployment of Neural Network
![dog](https://github.com/jbell303/yolov3-object-detection-cpp/blob/master/images/sf_processed.PNG)

## About
This is a C++ version of [this](https://www.pyimagesearch.com/2018/11/12/yolo-object-detection-with-opencv/) blog post by [PyImageSearch](https://pyimagesearch.com).
YOLO (You only look once) is an object detection algorithm by [Joseph Redmon](https://pjreddie.com) and Ali Faradhi.

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

## Rubric Points
Loops.1 - `helper.h` all functions  
Loops.2 - `object_detection.cpp` lines 89, 103  
Loops.3 - `object_detection.cpp` line 19  
OOP.1 - `helper.h`, `message_queue.h`  
OOP.2 - `message_queue.h` lines 13, 53  
OOP.3 - `helper.h` lines 19, 31, 44, 99  
OOP.4 - `helper.h`
OOP.9 - `message_queue.h` line 10  
Memory.1 - `helper.h` lines 19, 31, 44, 99  
Memory.3 - `object_detection.cpp` lines 142, 163, 164 
Memory.5 - `message_queue.cpp` lines 25, 37 `object_detection` lines 150, 175, 176  
Memory.6 - `object_detection.cpp` lines 142, 163, 164  
Concurrency.1 - `object_detection` lines 143, 165  
Concurrency.3 - `message_queue` lines 19, 34, 48 (not used)  
Concurrency.4 - `message_queue` lines 22, 38  

## References
* [YOLOv3: An Incremental Improvement](https://arxiv.org/abs/1804.02767) by Joseph Redmon and Ali Faradhi
* [OpenCV DNN Object Detection C++](https://github.com/opencv/opencv/blob/master/samples/dnn/object_detection.cpp)
* [learnopencv](https://www.learnopencv.com/deep-learning-based-object-detection-using-yolov3-with-opencv-python-c/) Deep Learning Based Object Detection Using YOLOv3 with OpenCV - Python, C++.
* [YOLOv4](https://github.com/AlexeyAB/darknet) - Neural Networks for Object Detection (Windows and Linux version of Darknet)  

Notably, this project attempts to use the async protocol from the OpenCV tutorial, however, I could not get async to process video files properly, so I changed the `Message Queue` class to that of the Concurrency module in the Udacity course.


