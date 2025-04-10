#include <ocean/base/Frame.h>
#include <iostream>

using namespace Ocean;

int main() {
  FrameType frameType(1280u, 720u, FrameType::FORMAT_RGB24, FrameType::ORIGIN_UPPER_LEFT);
  Frame frameOwningTheMemory(frameType);

  std::cout<< " Successfully created ocean frame. " << std::endl;
}
