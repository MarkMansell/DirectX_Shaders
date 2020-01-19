
Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);

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
	float3 T: TANGENT;
	float3 B: BINORMAL;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	float4 posW = mul(float4(input.PosL, 1.0f), World);
	output.PosW = posW.xyz;

	output.PosH = mul(posW, View);
	output.PosH = mul(output.PosH, Projection);

	output.Tex = input.Tex;

	float4 worldPosition = mul(float4(input.PosL, 1.0f), World);

	float3x3 tbn;
	tbn[0] = normalize(mul(float4(input.TanL, 0.0f), World).xyz);
	tbn[1] = normalize(mul(float4(cross(input.NormL, input.TanL), 0.0f), World).xyz);
	tbn[2] = normalize(mul(float4(input.NormL, 0.0f), World).xyz);

	output.T = tbn[0];
	output.B = tbn[1];
	output.NormW = tbn[2];

//	float3 lightvect = light.LightVecW - posW.xyz;
	float3 eyevect =   EyePosW - posW.xyz;

	output.LightVecT = normalize(mul(tbn, light.LightVecW));//lightvect));
	output.ToEyeT =    normalize(mul(tbn, eyevect));

	//float3 normalW = mul(float4(input.NormL, 0.0f), World).xyz;
	//output.NormW = normalize(normalW);

	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{

	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);
	float4 textureNormal = txNormal.Sample(samLinear, input.Tex);

	textureNormal = (2.0f * textureNormal) - 1.0f;
	//textureNormal = normalize(textureNormal);
//	float3x3 TBN = float3x3(normalize(input.T), normalize(input.B), normalize(input.NormW)); 

//	TBN = transpose(TBN);

	//float3 worldNormal = textureNormal.xyz;//mul(TBN, textureNormal.xyz); 

	float3 lightDir = normalize(input.LightVecT);
	float3 diffuse = saturate(dot(textureNormal.xyz, lightDir));

	float3 specular = float3(0.0f, 0.0f, 0.0f);

	diffuse = light.DiffuseLight.xyz * textureColour.rgb * diffuse;

	//float3 specular = 0;

	if (diffuse.x > 0)
	{
		float3 reflection = reflect(lightDir, textureNormal.xyz);
		float3 viewDir = normalize(input.ToEyeT);

		float3 r = reflect(-lightDir, textureNormal);
		float specularAmount = pow(max(dot(r, input.ToEyeT), 0.0f), light.SpecularPower);
		specular += specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb;

		//specular = saturate(dot(reflection, -viewDir));
		//specular = pow(specular, 20.0f);

		//specular *= light.SpecularPower * light.DiffuseLight.xyz;
	}

	float3 ambient = float3(0.1f, 0.1f, 0.1f) * textureColour;

	return float4(ambient + diffuse + specular, 1);
	//return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
