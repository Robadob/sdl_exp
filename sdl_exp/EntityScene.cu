#include "EntityScene.h"

__global__ void moveKernel(float *data, float i, cudaTextureObject_t texObj)
{
    unsigned int id = threadIdx.x;
    float id2 = (id*3.6)+i;
    id2 = (id2 >= 360) ? id2-360 : id2;
    data[(id * 4) + 0] = 100 * (float)sinf(id2*0.0174533);
    data[(id * 4) + 2] = 100 * (float)cosf(id2*0.0174533);
}
void EntityScene::cuInit()
{
    float *tempData = (float*)malloc(sizeof(float) * 4 * 100);
    for (int i = 0; i < 100; i++)
    {
        tempData[(i * 4) + 0] = 100 * sin(i*3.6);
        tempData[(i * 4) + 1] = -50.0f;
        tempData[(i * 4) + 2] = 100 * cos(i*3.6);
    }
    cudaMemcpy(this->cuTexBuf->d_mappedPointer, tempData, sizeof(float) * 4 * 100, cudaMemcpyHostToDevice);
    cudaError_t status = cudaGetLastError();
    if (status != CUDA_SUCCESS || (status = cudaGetLastError()) != CUDA_SUCCESS)
    {
        if (status == cudaErrorUnknown)
        {
            printf("An Unknown CUDA Error Occurred :(\n");
            printf("Perhaps performing the same operation under the CUDA debugger with Memory Checker enabled could help!\n");
            printf("If this error only occurs outside of NSight debugging sessions, or causes the system to lock up. It may be caused by not passing the required amount of shared memory to a kernal launch that uses runtime sized shared memory.\n");
            printf("Also possible you have forgotten to allocate texture memory you are trying to read\n");
            printf("Passing a buffer to 'cudaGraphicsSubResourceGetMappedArray' or a texture to 'cudaGraphicsResourceGetMappedPointer'.\n");
            getchar();
            exit(1);
        }
        printf(" CUDA Error Occurred;\n%s\n", cudaGetErrorString(status));
        getchar();
        exit(1);
    }
    free(tempData);
}

void EntityScene::cuUpdate()
{
    static float i = 0;
    i-=0.05f;
    i = (i < 0) ? 359 : i;
    moveKernel << <1, 100 >> >(this->cuTexBuf->d_mappedPointer, i, this->cuTexBuf->cuTextureObj); 
    cudaDeviceSynchronize(); 
    cudaError_t status = cudaGetLastError();
    if (status != CUDA_SUCCESS || (status = cudaGetLastError()) != CUDA_SUCCESS)
    {
        if (status == cudaErrorUnknown)
        {
            printf("An Unknown CUDA Error Occurred :(\n");
            printf("Perhaps performing the same operation under the CUDA debugger with Memory Checker enabled could help!\n");
            printf("If this error only occurs outside of NSight debugging sessions, or causes the system to lock up. It may be caused by not passing the required amount of shared memory to a kernal launch that uses runtime sized shared memory.\n");
            printf("Also possible you have forgotten to allocate texture memory you are trying to read\n");
            printf("Passing a buffer to 'cudaGraphicsSubResourceGetMappedArray' or a texture to 'cudaGraphicsResourceGetMappedPointer'.\n");
            getchar();
            exit(1);
        }
        printf(" CUDA Error Occurred;\n%s\n",  cudaGetErrorString(status));
        getchar();
        exit(1);
    }
}
