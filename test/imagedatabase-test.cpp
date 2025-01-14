#include "database-test.h"
#include <idx.h>
#include <iostream>
#include <algorithm> 
#include <vector>
#include <gsl/gsl>

void TestSingle();    
void TestAll();

void TestImageDatabase(){
    std::cout << "STARTING Database Tests" << std::endl;
    TestSingle();
    TestAll(); 
    std::cout << "Finished Database Tests" << std::endl; 
    std::vector<int> myvec{1,2,3,4,5,6,7,8,9};
    auto my_subspan = gsl::make_span(myvec).subspan(3, 5);
}

void TestSingle(){
    std::cout << "Testing Image Database:" << std::endl;
    IDX::ImageDatabase t10k("data/t10k-images.idx3-ubyte");
    Image test_image = t10k.GetImage(0);
    assert(test_image.x()    == t10k.x());
    assert(test_image.y()    == t10k.y());
    assert(test_image.size() == t10k.x()*t10k.y());

    std::vector<unsigned char> firstImage{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,84,185,159,151,60,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,222,254,254,254,254,241,198,198,198,198,198,198,198,198,170,52,0,0,0,0,0,0,0,0,0,0,0,0,67,114,72,114,163,227,254,225,254,254,254,250,229,254,254,140,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,66,14,67,67,67,59,21,236,254,106,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,83,253,209,18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,233,255,83,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,129,254,238,44,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,59,249,254,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,133,254,187,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,205,248,58,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,126,254,182,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,75,251,240,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19,221,254,166,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,203,254,219,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,254,254,77,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,224,254,115,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,133,254,254,52,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,61,242,254,254,52,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,121,254,254,219,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,121,254,207,18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    assert( std::equal(firstImage.begin(), firstImage.end(), test_image.begin) );

}

void TestAll(){
    std::cout << "Checking For Null Images" << std::endl;

    std::cout << "TESTING data/t10k-images.idx3-ubyte" << std::endl;

    IDX::ImageDatabase t10k("data/t10k-images.idx3-ubyte");
    for (int i = 0; i < t10k.size(); i++){
        Image img = t10k.GetImage(i);
        uint64_t checksum = 0;
        for (auto it = img.begin; it <= img.begin + img.x()*img.y(); it++){
            checksum += (uint64_t)*it;
        }
        assert(checksum != 0);
    }

    std::cout << "TESTING data/train-images.idx3-ubyte" << std::endl;

    IDX::ImageDatabase t10ktraining("data/train-images.idx3-ubyte");
    for (int i = 0; i < t10ktraining.size(); i++){
        Image img = t10ktraining.GetImage(i);
        uint64_t checksum = 0;
        for (auto it = img.begin; it <= img.begin + img.x()*img.y(); it++){
            checksum += (uint64_t)*it;
        }
        if (checksum == 0)
        std::cout << "FAILED AT " << i << std::endl;
        //assert(checksum != 0);
    }


}