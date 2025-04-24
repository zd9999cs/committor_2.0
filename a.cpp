#include <torch/torch.h>
#include <iostream>

int main() {
  if (torch::cuda::is_available()) {
    std::cout << "CUDA is available. Device count: " 
              << torch::cuda::device_count() << std::endl;
  } else {
    std::cout << "CUDA is not available." << std::endl;
  }
  return 0;
}