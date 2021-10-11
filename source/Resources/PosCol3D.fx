Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;
float4x4 gWorldMatrix : WORLD;
float4x4 gViewInverse : VIEWINVERSE;
float4x4 gWorldViewProj : WorldViewProjection;
float3 gLightDirection = { -0.577f, 0.577f, -0.577f };

float3 gLightColor = { 0.5f, .5f, .5f };
float gLightIntensity = { 7.0f };
float gShininess = {25.f};
uint gRasterIdx = { 0 };

//-------------------------------------------------------------
//	Rasterizer Sructs
//-------------------------------------------------------------
RasterizerState gRasterizerState[3]
{
	{
		CullMode = none;
		FrontCounterClockwise = true;
	},
	{
		CullMode = back;
		FrontCounterClockwise = true;
	},
	{
		CullMode = front;
		FrontCounterClockwise = true;
	}
};
//-------------------------------------------------------------
//	Blend State
//-------------------------------------------------------------
BlendState gBlendState
{
};

//-------------------------------------------------------------
//	Depth Stencil State
//-------------------------------------------------------------
DepthStencilState gDepthStencilState
{
};


//-------------------------------------------------------------
//	Sampling Sructs
//-------------------------------------------------------------
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Border; // or Mirrot or clamp or border
	AddressV = Clamp; // or Mirrot or clamp or border
	BorderColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border; // or Mirrot or clamp or border
	AddressV = Clamp; // or Mirrot or clamp or border
	BorderColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC; 
	AddressU = Border; // or Mirrot or clamp or border
	AddressV = Clamp; // or Mirrot or clamp or border
	BorderColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );
};

//-------------------------------------------------------------
//	Input/Output Sructs
//-------------------------------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 WorldPosition : COLOR;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : COLOR;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 ViewDirection : VIEWDIRECTION;
	float2 TexCoord : TEXCOORD;
	
};


float4 Phong(VS_OUTPUT input, SamplerState sampleMode)
{
	float3 reflect = gLightDirection - ( 2.f * dot(input.Normal, gLightDirection) * input.Normal );

	float cosA = abs(dot(reflect, input.ViewDirection));
	
	float4 specularColor = float4( gSpecularMap.Sample(sampleMode, input.TexCoord).xyz, 1.f );
	float glossiness = gGlossinessMap.Sample(sampleMode, input.TexCoord).r * gShininess;
	
	float phongSpecularReflection = pow(cosA, glossiness);
	
	specularColor *= phongSpecularReflection;
	
	return specularColor;
}

float4 Normal(VS_OUTPUT input, SamplerState sampleMode)
{

	// calculateNormal
	float3 normal = input.Normal;
	float3 tangent = input.Tangent;

	normal = normalize(normal);
	tangent = normalize(tangent);
	float3 binormal = normalize(cross(tangent, normal));

	float3x3 tangentSpaceAxis = float3x3( tangent, binormal, normal );

	float4 sampleValue = float4(gNormalMap.Sample(sampleMode, input.TexCoord).xyz, 1.f);
	normal.x = 2.f * sampleValue.r - 1.f;
	normal.y = 2.f * sampleValue.g - 1.f;
	normal.z = 2.f * sampleValue.b - 1.f;

	normal = mul(normal, tangentSpaceAxis);

	float observedArea = (float)0;
	float3 pixelColor = (float3)0;

	if ( dot(-normal, gLightDirection) <= 0 )
	{
		observedArea = dot(normal, gLightDirection);
		pixelColor = gLightColor * gLightIntensity * observedArea;
	}
	return float4(pixelColor, 1.f);
}


//-------------------------------------------------------------
//	Vertex Shader
//-------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4( input.Position.x, input.Position.y, -input.Position.z, 1.f ), mul(gWorldMatrix, gWorldViewProj));

	output.WorldPosition =float4( input.WorldPosition.x, input.WorldPosition.y, -input.WorldPosition.z, 1.f);
	output.Normal = mul(input.Normal, -gWorldMatrix);
	output.Tangent = mul(input.Tangent, -gWorldMatrix);
	output.TexCoord = input.TexCoord;
	output.ViewDirection = mul(normalize(input.WorldPosition.xyz - gViewInverse[3].xyz), gWorldMatrix);
	output.ViewDirection.z *= -1.f;

	return output;
}

//-------------------------------------------------------------
//	Pixel Shader
//-------------------------------------------------------------
//float4 CS(VS_OUTPUT input) : SV_TARGET
//{
//	return input.WorldPosition;
//}

float4 ColorSample(VS_OUTPUT input) : SV_TARGET
{
	return float4(input.WorldPosition.xy, -input.WorldPosition.z, 1.f);
}

float4 PointSample(VS_OUTPUT input) : SV_TARGET
{
	float4 finalColor = float4( gDiffuseMap.Sample(samPoint, input.TexCoord).xyz, 1.f );
	finalColor += Phong(input, samPoint);
	finalColor *= Normal(input, samPoint);
	
	return saturate(finalColor);
}

float4 LinearSample(VS_OUTPUT input) : SV_TARGET
{ 
	float4 finalColor = float4( gDiffuseMap.Sample(samLinear, input.TexCoord).xyz, 1.f );
	finalColor += Phong(input, samLinear);

	finalColor *= Normal(input, samLinear);
	
	return saturate(finalColor);
}

float4 AnisotropicSample(VS_OUTPUT input) : SV_TARGET
{
	float4 finalColor = float4( gDiffuseMap.Sample(samAnisotropic, input.TexCoord).xyz, 1.f );
	finalColor += Phong(input, samAnisotropic);
	finalColor *= Normal(input, samAnisotropic);
	
	return saturate(finalColor);
}

//-------------------------------------------------------------
//	Technique
//-------------------------------------------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState[gRasterIdx]);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
		SetVertexShader(CompileShader( vs_5_0, VS() ));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader( ps_5_0, ColorSample() ));
	}
}
//
technique11 Point
{
	pass P0
	{
		SetRasterizerState(gRasterizerState[gRasterIdx]);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
		SetVertexShader(CompileShader( vs_5_0, VS() ));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader( ps_5_0, PointSample() ));
	}
}

technique11 Linear
{
	pass P0
	{
		SetRasterizerState(gRasterizerState[gRasterIdx]);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
		SetVertexShader(CompileShader( vs_5_0, VS() ));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader( ps_5_0, LinearSample() ));
	}
}

technique11 Anisotropic
{
	pass P0
	{
		SetRasterizerState(gRasterizerState[gRasterIdx]);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
		SetVertexShader(CompileShader( vs_5_0, VS() ));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader( ps_5_0, AnisotropicSample() ));
	}
}
