Texture2D gDiffuseMap : DiffuseMap;
Texture2D gTransparencyMap : TransparencyMap;
float4x4 gWorldMatrix : WORLD;
float4x4 gWorldViewProj :ViewProjection;

//-------------------------------------------------------------
//	Rasterizer Sructs
//-------------------------------------------------------------
RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = true;
};

//-------------------------------------------------------------
//	Blend State
//-------------------------------------------------------------
BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

//-------------------------------------------------------------
//	Depth Stencil State
//-------------------------------------------------------------
DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;
	
	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;
	
	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;
	
	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;
	
	FrontFaceStencilPass = keep;
	BackFaceStencilPass= keep;
	
	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};



//-------------------------------------------------------------
//	Sampling Sructs
//-------------------------------------------------------------
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

//-------------------------------------------------------------
//	Vertex Shader
//-------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4( input.Position.x, input.Position.y, -input.Position.z, 1.f ), mul(gWorldMatrix, gWorldViewProj));
	output.TexCoord = input.TexCoord;

	return output;
}

//-------------------------------------------------------------
//	Pixel Shader
//-------------------------------------------------------------
float4 AnisotropicSample(VS_OUTPUT input) : SV_TARGET
{
	float4 finalColor = gDiffuseMap.Sample(samAnisotropic, input.TexCoord);
	
	return saturate(finalColor);
}

//-------------------------------------------------------------
//	Technique
//-------------------------------------------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
		SetVertexShader(CompileShader( vs_5_0, VS() ));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader( ps_5_0, AnisotropicSample() ));
	}
}
