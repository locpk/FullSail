#include "Samplers.hlsli"
#include "InputLayout.hlsli"
#include "../ConstantBuffer.h"

float4 main(GSOUT_Particle psInput) : SV_TARGET{
	float4 sampleColor = gTexArray.Sample(trilinearWrap, float3(psInput.TexCoord, 1));
	return (sampleColor*psInput.Color);
}