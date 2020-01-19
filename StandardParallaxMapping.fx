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
	float height = txDisplacement.Sample(samLinear, input.Tex);

	float3 toeyetangent = normalize(input.ToEyeT);

	float2 offset = toeyetangent.xy * (height * 0.04f);
	input.Tex += offset;

	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);
	float4 textureNormal = txNormal.Sample(samLinear, input.Tex);

	textureNormal = (2.0f * textureNormal) - 1.0f;

	float3 lightDir = normalize(input.LightVecT);
	float3 diffuse = saturate(dot(textureNormal.xyz, lightDir));

	float3 specular = float3(0.0f, 0.0f, 0.0f);

	diffuse = light.DiffuseLight.xyz * textureColour.rgb * diffuse;

	if (diffuse.x > 0)
	{
		float3 reflection = reflect(lightDir, textureNormal.xyz);
		float3 viewDir = normalize(input.ToEyeT);

		float3 r = reflect(-lightDir, textureNormal);
		float specularAmount = pow(max(dot(r, input.ToEyeT), 0.0f), light.SpecularPower);
		specular += specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb;
	}

	float3 ambient = float3(0.1f, 0.1f, 0.1f) * textureColour;

	return float4(ambient + diffuse + specular, 1);
}