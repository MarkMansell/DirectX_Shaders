
Texture2D ObjTexture : register(t0);

SamplerState samLinear
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct SurfaceInfo
{
	float4 AmbientMtrl;
	float4 DiffuseMtrl;
	float4 SpecularMtrl;
};

struct Light
{
	float4 AmbientLight;
	float4 DiffuseLight;
	float4 SpecularLight;

	float SpecularPower;
	float3 LightVecW;
};

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;

	SurfaceInfo surface;
	Light light;

	float3 EyePosW;
	float HasTexture;
}

struct VS_INPUT
{
	float2 PosL : POSITION;
	float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float2 texCoord1 : TEXCOORD1;
	float2 texCoord2 : TEXCOORD2;
	float2 texCoord3 : TEXCOORD3;
	float2 texCoord4 : TEXCOORD4;
	float2 texCoord5 : TEXCOORD5;
	float2 texCoord6 : TEXCOORD6;
	float2 texCoord7 : TEXCOORD7;
	float2 texCoord8 : TEXCOORD8;
	float2 texCoord9 : TEXCOORD9;

};

VS_OUTPUT VS(VS_INPUT input)
{
	//VS_OUTPUT output = (VS_OUTPUT)0;

	//output.PosH = mul(float4(input.PosL, 0.0f, 1.0f), World);

	//output.Tex = input.Tex;

	//return output;


	VS_OUTPUT output = (VS_OUTPUT)0;
	float texelSize;

	// Change the position vector to be 4 units for proper matrix calculations.
	//input.PosL.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	//output.PosH = mul(input.PosL, World);
	//output.PosH = mul(output.PosH, View);
	//output.PosH = mul(output.PosH, Projection);

	output.PosH = mul(float4(input.PosL, 0.0f, 1.0f), World);

	// Store the texture coordinates for the pixel shader.
	output.Tex = input.Tex;

	// Determine the floating point size of a texel for a screen with this specific width.
	texelSize = 1.0f / 1920; // sort out

							 // Create UV coordinates for the pixel and its four horizontal neighbors on either side.
	output.texCoord1 = input.Tex + float2(texelSize * -4.0f, 0.0f);
	output.texCoord2 = input.Tex + float2(texelSize * -3.0f, 0.0f);
	output.texCoord3 = input.Tex + float2(texelSize * -2.0f, 0.0f);
	output.texCoord4 = input.Tex + float2(texelSize * -1.0f, 0.0f);
	output.texCoord5 = input.Tex + float2(texelSize *  0.0f, 0.0f);
	output.texCoord6 = input.Tex + float2(texelSize *  1.0f, 0.0f);
	output.texCoord7 = input.Tex + float2(texelSize *  2.0f, 0.0f);
	output.texCoord8 = input.Tex + float2(texelSize *  3.0f, 0.0f);
	output.texCoord9 = input.Tex + float2(texelSize *  4.0f, 0.0f);

	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float weight0, weight1, weight2, weight3, weight4;
	float normalization;
	float4 color;

	// Create the weights that each neighbor pixel will contribute to the blur.
	weight0 = 1.0f;
	weight1 = 0.9f;
	weight2 = 0.55f;
	weight3 = 0.18f;
	weight4 = 0.1f;

	// Create a normalized value to average the weights out a bit.
	normalization = (weight0 + 2.0f * (weight1 + weight2 + weight3 + weight4));

	// Normalize the weights.
	weight0 = weight0 / normalization;
	weight1 = weight1 / normalization;
	weight2 = weight2 / normalization;
	weight3 = weight3 / normalization;
	weight4 = weight4 / normalization;

	// Initialize the color to black.
	color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	// Add the nine horizontal pixels to the color by the specific weight of each.
	color += ObjTexture.Sample(samLinear, input.texCoord1) * weight4;
	color += ObjTexture.Sample(samLinear, input.texCoord2) * weight3;
	color += ObjTexture.Sample(samLinear, input.texCoord3) * weight2;
	color += ObjTexture.Sample(samLinear, input.texCoord4) * weight1;
	color += ObjTexture.Sample(samLinear, input.texCoord5) * weight0;
	color += ObjTexture.Sample(samLinear, input.texCoord6) * weight1;
	color += ObjTexture.Sample(samLinear, input.texCoord7) * weight2;
	color += ObjTexture.Sample(samLinear, input.texCoord8) * weight3;
	color += ObjTexture.Sample(samLinear, input.texCoord9) * weight4;

	// Set the alpha channel to one.
	color.a = 1.0f;

	return color;
}
