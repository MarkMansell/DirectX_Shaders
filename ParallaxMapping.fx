//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);
Texture2D txDisplacement : register(t2);

SamplerState samLinear
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

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
	float3 PosL : POSITION;
	float3 NormL : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 TanL : TANGENT;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
	float3 ToEyeT : EYEVECTOR;
	float3 LightVecT : LIGHTVECTOR;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	float4 posW = mul(float4(input.PosL, 1.0f), World);
	output.PosW = posW.xyz;

	output.PosH = mul(posW, View);
	output.PosH = mul(output.PosH, Projection);

	output.Tex = input.Tex;

	float3x3 tbn;
	tbn[0] = normalize(mul(float4(input.TanL, 0.0f), World).xyz);
	tbn[1] = normalize(mul(float4(cross(input.NormL, input.TanL), 0.0f), World).xyz);
	tbn[2] = normalize(mul(float4(input.NormL, 0.0f), World).xyz);

	float3 eyevect =   EyePosW - posW.xyz;

	float3 lightvect = normalize(light.LightVecW);

	output.LightVecT = normalize(mul(tbn, lightvect));
	output.ToEyeT =    normalize(mul(tbn, eyevect));
	
	float3 normal = mul(input.NormL, tbn);
	output.NormW = normalize(normal);
	
	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{

	int nMinSamples = 70;
	int nMaxSamples = 80;

	float fParallaxLimit = -length(input.ToEyeT.xy) / input.ToEyeT.z;

	// Scale the parallax limit according to heightmap scale.
	fParallaxLimit *= 0.0013f + -0.02f;
	//0.8f
	// Calculate the parallax offset vector direction and maximum offset.
	float2 vOffsetDir = normalize(input.ToEyeT.xy);
	float2 vMaxOffset = vOffsetDir * fParallaxLimit;

	// Calculate the geometric surface normal vector, the vector from
	// the viewer to the fragment, and the vector from the fragment
	// to the light.
	float3 N = normalize(input.NormW);
	float3 E = normalize(input.ToEyeT);
	float3 L = normalize(input.LightVecT);

	// Calculate how many samples should be taken along the view ray
	// to find the surface intersection.  This is based on the angle
	// between the surface normal and the view vector.
	int nNumSamples = (int)lerp(nMaxSamples, nMinSamples, dot(E, N));

	// Specify the view ray step size.  Each sample will shift the current
	// view ray by this amount.
	float fStepSize = 1.0 / (float)nNumSamples;

	// Calculate the texture coordinate partial derivatives in screen
	// space for the tex2Dgrad texture sampling instruction.
	float2 dx = ddx(input.Tex);
	float2 dy = ddy(input.Tex);

	// Initialize the starting view ray height and the texture offsets.
	float fCurrRayHeight = 1.0;
	float2 vCurrOffset = float2(0, 0);
	float2 vLastOffset = float2(0, 0);

	float fLastSampledHeight = 1;
	float fCurrSampledHeight = 1;

	int nCurrSample = 0;

	while (nCurrSample < nNumSamples)
	{
		// Sample the heightmap at the current texcoord offset.  The heightmap 
		// is stored in the alpha channel of the height/normal map.
		//fCurrSampledHeight = tex2Dgrad( NH_Sampler, IN.texcoord + vCurrOffset, dx, dy ).a;
		fCurrSampledHeight = txDisplacement.SampleGrad(samLinear, input.Tex + vCurrOffset, dx, dy).r;

		// Test if the view ray has intersected the surface.
		if (fCurrSampledHeight > fCurrRayHeight)
		{
			// Find the relative height delta before and after the intersection.
			// This provides a measure of how close the intersection is to 
			// the final sample location.
			float delta1 = fCurrSampledHeight - fCurrRayHeight;
			float delta2 = (fCurrRayHeight + fStepSize) - fLastSampledHeight;
			float ratio = delta1 / (delta1 + delta2);

			// Interpolate between the final two segments to 
			// find the true intersection point offset.
			vCurrOffset = (ratio)* vLastOffset + (1.0 - ratio) * vCurrOffset;

			// Force the exit of the while loop
			nCurrSample = nNumSamples + 1;
		}
		else
		{
			// The intersection was not found.  Now set up the loop for the next
			// iteration by incrementing the sample count,
			nCurrSample++;

			// take the next view ray height step,
			fCurrRayHeight -= fStepSize;

			// save the current texture coordinate offset and increment
			// to the next sample location, 
			vLastOffset = vCurrOffset;
			vCurrOffset += fStepSize * vMaxOffset;

			// and finally save the current heightmap height.
			fLastSampledHeight = fCurrSampledHeight;
		}
	}

	// Calculate the final texture coordinate at the intersection point.
	float2 vFinalCoords = input.Tex + vCurrOffset;

	// Use the final texture coordinates to get the normal vector, then 
	// expand it from [0,1] to [-1,1] range.
	float4 vFinalNormal = txNormal.Sample(samLinear, vFinalCoords); //.a;

																			   // Sample the colormap at the final intersection point.
	float4 vFinalColor = txDiffuse.Sample(samLinear, vFinalCoords);

	// Expand the final normal vector from [0,1] to [-1,1] range.
	vFinalNormal = vFinalNormal * 2.0f - 1.0f;

	// Shade the fragment based on light direction and normal.
	float3 vAmbient = vFinalColor.rgb * 0.1f;
	float3 vDiffuse = vFinalColor.rgb * max(0.0f, dot(L, vFinalNormal.xyz)) * 0.5f;

	vFinalColor.rgb = vAmbient + vDiffuse;

	float4 color = vFinalColor;

	//float2 vGridCoords = frac(vFinalCoords * 10.0f);

	//if ((vGridCoords.x < 0.025f) || (vGridCoords.x > 0.975f))
	//	color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	//if ((vGridCoords.y < 0.025f) || (vGridCoords.y > 0.975f))
	//	color = float4(1.0f, 1.0f, 1.0f, 1.0f);

	return color;

}
