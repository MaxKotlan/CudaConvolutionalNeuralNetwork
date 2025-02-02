#include <iostream>
#include <vector>
#include  <iomanip>
#include "idx-cuda.h"
#include "neuralnetwork.h"
#include <thrust/device_vector.h>

int main(int argc, char** argv){
    
    IDX::ImageDatabase t10k("data/t10k-images.idx3-ubyte");
    IDX::LabelDatabase t10klab("data/t10k-labels.idx1-ubyte");
    IDX::ImageDatabase t10ktrain("data/train-images.idx3-ubyte");
    IDX::LabelDatabase t10ktrainlab("data/train-labels.idx1-ubyte");

    srand(132);
    NeuralNetwork mynn(t10k.x()*t10k.y(), 16, 2, 10, 1.0);

    uint32_t imageindex;
    std::cout << " Enter Image Index: ";
    std::cin >> imageindex;

    auto image_raw = t10ktrain.GetImage(imageindex);
    auto image = image_raw.Normalize();
    uint32_t label = t10ktrainlab.GetLabel(imageindex);    

    //IDX::ImageDatabase     t10ktrain_reg("../data/train-images.idx3-ubyte");
    //auto image_raw_host = t10k.GetImage(imageindex);
    /*for(int i = 0; i < image_raw.size(); i++){
        if (i%28 == 0) std::cout << std::endl;
        uint32_t el = (uint32_t)image_raw[i];
        if (el != 0)
        std::cout << std::setfill('0') << std::setw(2) << std::hex  << el;
        else
            std::cout << "  ";
    }
    std::cout << std::endl;*/

    std::cout << std::fixed << std::setprecision(2);

    std::cout << std::dec;
    uint32_t pollingrate = 10;
    uint32_t count = 0;
    while (true){

        image_raw = t10ktrain.GetImage(count%t10ktrain.size());
        image = image_raw.Normalize();
        label = t10ktrainlab.GetLabel(count%t10ktrain.size());    


        if (count%pollingrate == 0){
            auto device_result = mynn.ForwardPropagate(image);
            std::vector<float> result(device_result.size());
            thrust::copy(device_result.begin(), device_result.end(), result.begin());
            std::cout << "training iteration: " << count << " should be: " << label << " Probabilities: ";
            for (auto e : result)
                std::cout << std::fixed << std::setprecision(2) << e << " ";

            float total = 0;
            for (uint32_t i = 0; i < result.size(); i++){
                float difference = 0.0;
                if (i == label) difference = 1.0;
                total += ((result[i] - difference) * (result[i] - difference) );
            }
            total = total / 10.0;
            std::cout << std::fixed << std::setprecision(6) << " error: " << total;
            std::cout << std::endl;
        }
        for (int i = 0; i < 100; i++)
            mynn.TrainSingle(image, label);
        count++;
    }
}