#include "layer.h"
#include "debug.h"
#include "activation.h"
#include "neuralnetwork.h"
#include "matrix.h"
#include <cublas_v2.h>
#include <curand.h>


LayerConnector::LayerConnector(uint32_t inputsize, uint32_t outputsize, NeuralNetwork* network=nullptr):
    inputsize(inputsize),
    outputsize(outputsize),
    d_input(inputsize),
    d_output_ref(nullptr),
    d_weights(inputsize*outputsize),
    d_biases(outputsize),
    _nextLayer(nullptr),
    _neuralnetwork(network),
    d_delta_weights(outputsize*inputsize),
    d_delta_biases(outputsize),
    d_activation_delta(outputsize)
{
    if (network == nullptr){
        std::cerr << " \n\
        ERROR. You must set a reference to the neural network, \
        so that the layer can retrive global parameters such as the learning rate. \
        ";
        exit(-1);
    }
    InitalizeWithRandomValues();
    ResetDeltaVectors();
    cudaDeviceSynchronize();
};

void LayerConnector::InitalizeWithRandomValues(){

    curandGenerator_t generator = nullptr;
    if(generator == nullptr) { 
        curandCreateGenerator(&generator, CURAND_RNG_PSEUDO_DEFAULT);    
        curandSetPseudoRandomGeneratorSeed(generator, 0);
    }
    curandGenerateUniform(generator, thrust::raw_pointer_cast(d_weights.data()), d_weights.size());
    curandGenerateUniform(generator, thrust::raw_pointer_cast(d_biases.data()),  d_biases.size());

}

void LayerConnector::ResetDeltaVectors(){
    thrust::fill(d_delta_weights.begin(), d_delta_weights.end(), 0);
    thrust::fill(d_delta_biases.begin(),  d_delta_biases.end(),  0);
}

thrust::device_vector<float> LayerConnector::operator()(thrust::device_vector<float> &d_input){
    return std::move(CalculateOutputNeurons(d_input));
}

thrust::device_vector<float> LayerConnector::CalculateOutputNeurons(thrust::device_vector<float>& d_input_new){
    d_input = std::move(d_input_new);

    thrust::device_vector<float> d_output(outputsize);
    thrust::copy(d_biases.begin(), d_biases.end(), d_output.begin());

    MatrixMultiply(
        d_output.size(), d_input.size(), 1,
        1.0,1.0,
        d_weights, d_input, d_output
    );
    cudaDeviceSynchronize();
    thrust::transform(d_output.begin(), d_output.end(), d_output.begin(), Activation::Sigmoid());
    cudaDeviceSynchronize();
    return std::move(d_output);
}

void LayerConnector::CalculateGradient(thrust::device_vector<float>& d_cost){
    if (d_output_ref == nullptr){
        std::cerr << " \n\
        ERROR. You must set a reference to the input of the next layer to calculate the gradient.\n \
        Please do this by calling SetOutputReference( [pointer to device output vector] )\n \
        before you call CalculateGradient().\n \
        ";
        exit(-1);
    }

    d_activation_delta = GenerateActivationDelta(*d_output_ref);
    if (_nextLayer == nullptr) {
        /*If output layer use cost vector*/
        thrust::transform(d_activation_delta.begin(), d_activation_delta.end(), d_cost.begin(), d_activation_delta.begin(), thrust::multiplies<float>());
        cudaDeviceSynchronize();
    } else {
        /*otherwise use previous layer*/
        thrust::device_vector<float> previous_layer_delta = CalculatePreviousLayerDelta();
        cudaDeviceSynchronize();
        thrust::transform(d_activation_delta.begin(), d_activation_delta.end(), previous_layer_delta.begin(), d_activation_delta.begin(), thrust::multiplies<float>());
        cudaDeviceSynchronize();
    }

    MatrixMultiply(
        d_input.size(), 1, d_activation_delta.size(),
        1.0, 1.0, 
        d_input, d_activation_delta, d_delta_weights
    );
    cudaDeviceSynchronize();
    thrust::device_vector<float> newdeltabias(d_delta_biases.size()); cudaDeviceSynchronize();
    thrust::transform(d_activation_delta.begin(), d_activation_delta.end(), d_biases.begin(), newdeltabias.begin(), thrust::multiplies<float>());
    cudaDeviceSynchronize();
    thrust::transform(newdeltabias.begin(), newdeltabias.end(), d_delta_biases.begin(), d_delta_biases.begin(), thrust::plus<float>());
    cudaDeviceSynchronize();

}

thrust::device_vector<float> LayerConnector::CalculatePreviousLayerDelta(){
    thrust::device_vector<float> previous_layer_delta(outputsize);

    MatrixMultiply(
        _nextLayer->inputsize, _nextLayer->outputsize, 1,
        1.0, 0.0, 
        _nextLayer->d_weights, _nextLayer->d_activation_delta, previous_layer_delta
    );
    cudaDeviceSynchronize();

    return std::move(previous_layer_delta);
}

thrust::device_vector<float> LayerConnector::GenerateActivationDelta(const thrust::device_vector<float>& output_layer){
    thrust::device_vector<float> d_activation_delta = output_layer;
    thrust::transform(d_activation_delta.begin(), d_activation_delta.end(), d_activation_delta.begin(), Activation::SigmoidDerivative());
    cudaDeviceSynchronize();
    return std::move(d_activation_delta);
}

void LayerConnector::ApplyDeltas(){
    /*Multiply Deltas by the learning rate*/
    thrust::transform(d_delta_weights.begin(), d_delta_weights.end(), thrust::make_constant_iterator(_neuralnetwork->getLearningRate()), d_delta_weights.begin(), thrust::multiplies<float>());
    cudaDeviceSynchronize();
    thrust::transform(d_delta_biases.begin(), d_delta_biases.end(), thrust::make_constant_iterator(_neuralnetwork->getLearningRate()), d_delta_biases.begin(), thrust::multiplies<float>());
    cudaDeviceSynchronize();

    thrust::transform(d_delta_weights.begin(), d_delta_weights.end(), thrust::make_constant_iterator((float)(_neuralnetwork->getTrainingCount())), d_delta_weights.begin(), thrust::divides<float>());
    cudaDeviceSynchronize();
    thrust::transform(d_weights.begin(), d_weights.end(), d_delta_weights.begin(), d_weights.begin(), thrust::minus<float>());
    cudaDeviceSynchronize();

    thrust::transform(d_delta_biases.begin(), d_delta_biases.end(), thrust::make_constant_iterator((float)(_neuralnetwork->getTrainingCount())), d_delta_biases.begin(), thrust::divides<float>());
    cudaDeviceSynchronize();
    thrust::transform(d_biases.begin(), d_biases.end(), d_delta_biases.begin(), d_biases.begin(), thrust::minus<float>());
    cudaDeviceSynchronize();

    ResetDeltaVectors();
    cudaDeviceSynchronize();
}