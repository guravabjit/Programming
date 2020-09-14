__global__ void sinwave_vbo_kernel(float4 *pos, unsigned int width, unsigned int height, float animTime)
{
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
	
	float u = x / (float)width;
	float v = y / (float)height;
	
	u = (u*2.0)-1.0;
	v = (v*2.0)-1.0;
	
	float frequency = 4;
	float w = sinf(frequency*u+animTime) * cosf(frequency*v+animTime) * 0.5;
	
	pos[y*width+x] = make_float4(u,w,v,1.0);
	
	return;
}

void launchCudaKernel(float4* pos, unsigned int width, unsigned int height, float animTime)
{
	dim3 block(8, 8, 1);
	dim3 grid(width/block.x, height/block.y, 1.0);
	sinwave_vbo_kernel <<< grid, block >>> (pos, width, height, animTime);
}
