#include <iostream>
#include "detect_num.h"

using namespace std;

int main()
{
    cv::Mat image_sc;
    std::vector<Vec2i> coordination;
    std::vector<cv::Mat> image_all;
    float height;
    coordination = detect_num(image_sc, image_all, height);

    cout << "Detection complete!" << endl;
    return 0;
}

