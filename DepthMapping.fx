cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexInputType
{
	float3 position : POSITION;
	float3 NormL : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 TanL : TANGENT;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType DepthVertexShader(VertexInputType input)
{
	PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	//input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	float4 posW = mul(float4(input.position, 1.0f), worldMatrix);
	output.position = mul(posW, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the position value in a second input value for depth value calculations.
	output.depthPosition = output.position;

	return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 DepthPixelShader(PixelInputType input) : SV_TARGET
{
	float depthValue;
	float4 color;

	// Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
	//depthValue = 1.0f - (input.depthPosition.z / input.depthPosition.w);
	depthValue = input.depthPosition.z / input.depthPosition.w;

	// First 10% of the depth buffer color red.
	if (depthValue < 0.9f)
	{
		color = float4(1.0, 0.0f, 0.0f, 1.0f);
	}

	// The next 0.025% portion of the depth buffer color green.
	if (depthValue > 0.9f)
	{
		color = float4(0.0, 1.0f, 0.0f, 1.0f);
	}

	// The remainder of the depth buffer color blue.
	if (depthValue > 0.925f)
	{
		color = float4(0.0, 0.0f, 1.0f, 1.0f);
	}
	color = float4(depthValue, depthValue, depthValue, 1.0f);
	return color;
}