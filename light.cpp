#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// 定义检测模式
enum LightState { RED, GREEN, NOT };

// 检测红绿灯模式
LightState detectLightState(const Mat& frame) {
    Mat hsv, maskRed, maskGreen;

    // 转换到 HSV 色彩空间
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    // 红色范围 (分两部分：低红和高红)
    Scalar lowRed1(0, 100, 140), highRed1(10, 255, 255);
    Scalar lowRed2(170, 100, 140), highRed2(180, 255, 255);
    inRange(hsv, lowRed1, highRed1, maskRed);
    Mat maskRed2;
    inRange(hsv, lowRed2, highRed2, maskRed2);
    maskRed |= maskRed2;

    // 绿色范围
    Scalar lowGreen(30, 50, 50), highGreen(89, 255, 255);
    inRange(hsv, lowGreen, highGreen, maskGreen);

    // 使用 HoughCircles 检测圆形
    vector<Vec3f> circlesRed, circlesGreen;
    HoughCircles(maskRed, circlesRed, HOUGH_GRADIENT, 1, 20, 100, 20, 3, 70);  // 参数需调整
    HoughCircles(maskGreen, circlesGreen, HOUGH_GRADIENT, 1, 20, 100, 20, 3, 70);

    // 判断结果
    if (!circlesGreen.empty()) {
        return GREEN; // 检测到绿色圆形
    } else if (!circlesRed.empty()) {
        return RED;   // 检测到红色圆形
    } else {
        return NOT;   // 无红绿灯
    }
}

// 显示文字和标注圆形区域
void displayResult(Mat& frame, LightState state, const vector<Vec3f>& circles) {
    // 状态文字
    string status;
    Scalar color;
    switch (state) {
        case RED:
            status = "RED";
            color = Scalar(0, 0, 255); // 红色
            break;
        case GREEN:
            status = "GREEN";
            color = Scalar(0, 255, 0); // 绿色
            break;
        case NOT:
            status = "NOT";
            color = Scalar(255, 255, 255); // 白色
            break;
    }

    // 显示状态文字
    putText(frame, status, Point(10, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);

    // 标注检测到的圆形
    for (size_t i = 0; i < circles.size(); i++) {
        Vec3f c = circles[i];
        Point center(cvRound(c[0]), cvRound(c[1]));
        int radius = cvRound(c[2]);
        // 绘制圆形
        circle(frame, center, radius, color, 2);
        // 绘制圆心
        circle(frame, center, 3, Scalar(255, 255, 255), -1);
    }
}

int main(int argc, char** argv) {
    // 打开视频文件或摄像头
    VideoCapture cap("/home/xybuser/桌面/color-main/video/move_green.avi"); // 替换为你的 AVI 文件路径
    //VideoCapture cap("/home/xybuser/桌面/color-main/video/move_red.avi"); 
    //VideoCapture cap("/home/xybuser/桌面/color-main/video/move_red_yellow.avi"); 
    //VideoCapture cap("/home/xybuser/桌面/color-main/video/random.avi");
    //VideoCapture cap("/home/xybuser/桌面/color-main/video/red_10s.avi");

    //VideoCapture cap(0);                  // 使用USB摄像头（替换上面一行）
 

    if (!cap.isOpened()) {
        cout << "Error: Cannot open video file." << endl;
        return -1;
    }

    Mat frame;
    while (cap.read(frame)) {
        // 检测红绿灯状态
        LightState state = detectLightState(frame);

        // 显示结果
        vector<Vec3f> circles;
        if (state == RED || state == GREEN) {
            // 再次检测圆形 (这里假设重用 detectLightState 的结果)
            Mat mask;
            Mat hsv;
            cvtColor(frame, hsv, COLOR_BGR2HSV);
            if (state == RED) {
                Scalar lowRed1(0, 100, 140), highRed1(10, 255, 255);
                Scalar lowRed2(170, 100, 140), highRed2(180, 255, 255);
                Mat maskRed1, maskRed2;
                inRange(hsv, lowRed1, highRed1, maskRed1);
                inRange(hsv, lowRed2, highRed2, maskRed2);
                mask = maskRed1 | maskRed2;
            } else { // GREEN
                Scalar lowGreen(30, 50, 50), highGreen(89, 255, 255);
                inRange(hsv, lowGreen, highGreen, mask);
            }
            HoughCircles(mask, circles, HOUGH_GRADIENT, 1, 20, 100, 20, 3, 70); // 再次检测圆形
        }
        displayResult(frame, state, circles);

        // 显示画面
        imshow("Traffic Light Detection", frame);

        // 按下 ESC 键退出
        if (waitKey(30) == 27) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
