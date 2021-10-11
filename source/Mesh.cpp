#include "pch.h"
#include "Mesh.h"
#include <sstream>
#include <fstream>
#include <regex>

Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex_Input>& vertexData, const std::vector<uint32_t>& indexData, const std::wstring& effectPath, bool transFlag, const Elite::FPoint3& position)
	: Object(position)
	, m_VertexBuffer{ vertexData }
	, m_IndexBuffer{ indexData }
{

	SetTransformationMatrix();
	SetTransformedVertexBuffer(m_TransformedVertexBuffer, m_VertexBuffer);


	EffectSetup(pDevice, effectPath, transFlag);


}

Mesh::Mesh(ID3D11Device* pDevice, const char* filePath, const std::wstring& effectPath, bool transFlag, const Elite::FPoint3& position, bool flipY)
	: Object(position)
{
	ParseFile(filePath, flipY);
	SetTransformationMatrix();
	SetTransformedVertexBuffer(m_TransformedVertexBuffer, m_VertexBuffer);

	EffectSetup(pDevice, effectPath, transFlag);

}

void Mesh::EffectSetup(ID3D11Device* pDevice, const std::wstring& effectPath, bool transflag)
{
	if ( !transflag )
	{
		m_pEffect = new Effect(pDevice, effectPath);
	} else
	{
		m_pEffect = new FlatEffect(pDevice, effectPath);
	}

	//Create Vertex Layout
	HRESULT result = S_OK;
	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 36;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TEXCOORD";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 48;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	//Create the input layout
	D3DX11_PASS_DESC passDesc;
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pVertexLayout);

	if ( FAILED(result) )
	{
		printf("[WARNING] InputLayout creation: FAILED");
		return;
	}

	//Create Vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_Input) * (uint32_t) m_VertexBuffer.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = m_VertexBuffer.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);

	if ( FAILED(result) )
	{
		printf("[WARNING] Vertex Buffer creation: FAILED");
		return;
	}

	//Create index buffer
	m_AmountIndices = (uint32_t) m_IndexBuffer.size();
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = m_IndexBuffer.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if ( FAILED(result) )
	{
		printf("[WARNING] Index Buffer creation: FAILED");
		return;
	}
}

Mesh::~Mesh()
{
	if ( m_pEffect )
	{
		delete m_pEffect;
		m_pEffect = nullptr;
	}

	if ( m_pVertexLayout )
	{
		m_pVertexLayout->Release();
	}

	if ( m_pIndexBuffer )
	{
		m_pIndexBuffer->Release();
	}

	if ( m_pVertexBuffer )
	{
		m_pVertexBuffer->Release();
	}
}

void Mesh::SetTransformationMatrix(const Elite::FVector3& worldUp)
{
	m_U = Elite::Cross(worldUp, m_W);
	m_V = Elite::Cross(m_W, m_U);
	m_W = Elite::Cross(m_U, m_V);

	Elite::Normalize(m_U);
	Elite::Normalize(m_V);
	Elite::Normalize(m_W);

	m_TransformationMatrix = Elite::FMatrix4(m_U, m_V, m_W, Elite::FVector4(m_Position.x, m_Position.y, m_Position.z, 1));
}

void Mesh::SetTransformedVertexBuffer(std::vector<Vertex_Output>& transformedVertexBuffer, const std::vector<Vertex_Input>& vertexBuffer)
{
	transformedVertexBuffer.resize(vertexBuffer.size());
	for ( uint32_t i{}; i < vertexBuffer.size(); i++ )
	{
		transformedVertexBuffer[i].Position = vertexBuffer[i].Position;
		transformedVertexBuffer[i].WorldPosition = vertexBuffer[i].WorldPosition;
		transformedVertexBuffer[i].Uv = vertexBuffer[i].Uv;
		transformedVertexBuffer[i].Normal = vertexBuffer[i].Normal;
	}
}

bool Mesh::GetColor(const std::vector<uint32_t>& idx, const Elite::FPoint2& pixel, Elite::RGBColor& pixelColor, float& depth)
{
	//frustrum culling
	for ( size_t i{}; i < idx.size(); ++i )
	{
		if ( m_TransformedVertexBuffer[idx[i]].Position.w < 0.1f || m_TransformedVertexBuffer[idx[i]].Position.w > 1000.f ) return false;
	}

	int j = 0;
	int k = 0;
	for ( size_t i{}; i < idx.size(); ++i )
	{
		// culling
		if ( Elite::Cross(pixel - static_cast<Elite::FPoint2>( m_TransformedVertexBuffer[idx[( i + 2 ) % 3]].Position ), static_cast<Elite::FVector2>( ( m_TransformedVertexBuffer[idx[i]].Position - m_TransformedVertexBuffer[idx[( i + 2 ) % 3]].Position ) )) < 0 )
		{
			j++;
		} else
		{
			k++;
		}
	}

	if ( j != 3 && k != 3 )
	{
		return false;
	}

	switch ( m_pEffect->GetCullMode() )
	{
		case CullMode::NONE:
			break;
		case CullMode::BACK:
			if ( j == 3 )
			{
				return false;
			}
			break;
		case CullMode::FRONT:
			if ( k == 3 )
			{
				return false;
			}
			break;
		default:
			break;
	}

	//Rasterization Stage
	//float triangleSurface{};
	float weights[3]{};
	float finalWeight{};
	float finalDepth{};

	float signedAreaParalellogramTriangle;
	float totalSurface = Elite::Cross(m_TransformedVertexBuffer[idx[2]].Position.xy - m_TransformedVertexBuffer[idx[0]].Position.xy, m_TransformedVertexBuffer[idx[1]].Position.xy - m_TransformedVertexBuffer[idx[0]].Position.xy);
	Elite::FVector2 side{};
	Elite::FPoint2 oposingVertex{};

	Elite::FVector4 interpolateColor{};
	Elite::FVector2 interpelatedUV{};
	Elite::FVector3 interpelatedNormal{};
	Elite::FVector3 interpelatedTangent{};
	Elite::FVector3 interpelatedViewDirection{};

	for ( size_t i{}; i < idx.size(); ++i )
	{
		oposingVertex = m_TransformedVertexBuffer[idx[( ( i + 1 ) % 3 )]].Position.xy;
		side = m_TransformedVertexBuffer[idx[( ( i + 2 ) % 3 )]].Position.xy - oposingVertex;
		signedAreaParalellogramTriangle = Elite::Cross(pixel - oposingVertex, side);

		weights[i] = signedAreaParalellogramTriangle / totalSurface;

		finalDepth += weights[i] * ( 1.0f / m_TransformedVertexBuffer[idx[i]].Position.z );
		finalWeight += weights[i] * ( 1.0f / m_TransformedVertexBuffer[idx[i]].Position.w );

		interpolateColor += (Elite::FVector3(m_TransformedVertexBuffer[idx[i]].WorldPosition.xyz) / m_TransformedVertexBuffer[idx[i]].Position.w ) * weights[i];
		interpelatedUV += ( m_TransformedVertexBuffer[idx[i]].Uv / m_TransformedVertexBuffer[idx[i]].Position.w ) * weights[i];
		interpelatedNormal += ( m_TransformedVertexBuffer[idx[i]].Normal / m_TransformedVertexBuffer[idx[i]].Position.w ) * weights[i];
		interpelatedTangent += ( m_TransformedVertexBuffer[idx[i]].Tangent / m_TransformedVertexBuffer[idx[i]].Position.w ) * weights[i];
		interpelatedViewDirection += ( m_TransformedVertexBuffer[idx[i]].ViewDirection / m_TransformedVertexBuffer[idx[i]].Position.w ) * weights[i];

	}
	finalWeight = 1.0f / finalWeight;
	finalDepth = 1.0f / finalDepth;

	if ( finalDepth > depth )
	{
		return false;
	}
	depth = finalDepth;

	interpolateColor *= finalWeight;
	interpelatedUV *= finalWeight;
	interpelatedNormal *= finalWeight;
	interpelatedTangent *= finalWeight;
	interpelatedViewDirection *= finalWeight;

	Elite::FPoint4 position = Elite::FPoint4(pixel.x, pixel.y, finalWeight, finalDepth);
	Vertex_Output oVertex = Vertex_Output{ position, interpolateColor,interpelatedNormal, interpelatedTangent, interpelatedUV, interpelatedViewDirection};
	bool skipPixel = false;
	PixelShading(oVertex, pixelColor, skipPixel);
	return !skipPixel;
	//Rasterization Stage
}

void Mesh::PixelShading(const Vertex_Output& outputVertex, Elite::RGBColor& pixelColor, bool& skipPixel)
{
	// normals
	float observedArea{0};
	float lightIntensity = 7.0f;
	Elite::FVector3 lightDirection{ 0.577f, -0.577f, 0.577f };
	Elite::RGBColor lightColor{ 0.50f, .50f, .50f };
	bool hasDiffuse{ m_pDiffuseTexture->IsSet() };
	bool hasNormal{ m_pNormalTexture->IsSet() };
	bool hasGlossAndSpec{ m_pGlossinessTexture->IsSet() && m_pSpecularTexture->IsSet() };

	if ( !hasGlossAndSpec && !hasDiffuse && !hasNormal )
	{
		pixelColor = Elite::RGBColor(outputVertex.WorldPosition.x, outputVertex.WorldPosition.y, outputVertex.WorldPosition.z);
		return;
	}
	bool skip = false;
	skip;
	if ( !hasGlossAndSpec )
	{
		if ( !hasDiffuse && !hasNormal )
		{
			if ( Elite::Dot(-outputVertex.Normal, lightDirection) <= 0 )
			{
				observedArea = Elite::Dot(outputVertex.Normal, lightDirection);
				Elite::RGBColor tempColor{ outputVertex.WorldPosition.x, outputVertex.WorldPosition.y, outputVertex.WorldPosition.z };
				pixelColor = tempColor * lightColor * lightIntensity * observedArea * outputVertex.Position.w;
				return;
			}
		} else if ( hasDiffuse && !hasNormal )
		{
			if ( outputVertex.Normal == Elite::FVector3(0, 0, 0) )
			{
				pixelColor = m_pDiffuseTexture->Sample(outputVertex.Uv);
				return;
			}

			if ( Elite::Dot(-outputVertex.Normal, lightDirection) <= 0 )
			{
				observedArea = Elite::Dot(outputVertex.Normal, lightDirection);
				pixelColor = lightColor * lightIntensity * observedArea * outputVertex.Position.w;
			}
			pixelColor *= m_pDiffuseTexture->Sample(outputVertex.Uv, skipPixel);
			return;

		} else if ( !hasDiffuse && hasNormal )
		{
			pixelColor = { 1,1,1 };
			CalculateNormal(outputVertex, lightDirection, observedArea, pixelColor, lightColor, lightIntensity);
			return;

		} else
		{
			pixelColor = m_pDiffuseTexture->Sample(outputVertex.Uv);
			CalculateNormal(outputVertex, lightDirection, observedArea, pixelColor, lightColor, lightIntensity);
		}
	} else
	{
		float shininess = 25.f;
		Elite::FVector3 reflect = lightDirection - ( 2.f * Elite::Dot(outputVertex.Normal, lightDirection) * outputVertex.Normal );

		float cosA = abs(Elite::Dot(reflect, outputVertex.ViewDirection));


		Elite::RGBColor specularColor = m_pSpecularTexture->Sample(outputVertex.Uv);
		float glossiness = m_pGlossinessTexture->Sample(outputVertex.Uv ).r * shininess;

		float phongSpecularReflection = powf(cosA, glossiness);

		specularColor *= phongSpecularReflection;



		if ( !hasDiffuse && !hasNormal )
		{
			pixelColor = specularColor;
			return;
		} else if ( hasDiffuse && !hasNormal )
		{
			pixelColor = m_pDiffuseTexture->Sample(outputVertex.Uv) + specularColor;
			return;
		} else if ( !hasDiffuse && hasNormal )
		{
			pixelColor += specularColor;
			CalculateNormal(outputVertex, lightDirection, observedArea, pixelColor, lightColor, lightIntensity);
			return;
		} else
		{
			pixelColor = m_pDiffuseTexture->Sample(outputVertex.Uv) + specularColor;
			CalculateNormal(outputVertex, lightDirection, observedArea, pixelColor, lightColor, lightIntensity);
			return;
		}
	}
}

void Mesh::CalculateNormal(const Vertex_Output& outputVertex, Elite::FVector3& lightDirection, float& observedArea, Elite::RGBColor& pixelColor, Elite::RGBColor& lightColor, float lightIntensity)
{
	Elite::FVector3 binormal;
	Elite::FVector3 normal;
	Elite::FVector3 tangent;

	Elite::FMatrix3 tangentSpaceAxis;
	Elite::RGBColor sampleValue;

	normal = Elite::GetNormalized(outputVertex.Normal);
	tangent = Elite::GetNormalized(outputVertex.Tangent);
	binormal = Elite::GetNormalized(Elite::Cross(normal, tangent));

	tangentSpaceAxis = Elite::FMatrix3(tangent, binormal, normal);

	sampleValue = m_pNormalTexture->Sample(outputVertex.Uv);
	normal.x = 2.f * sampleValue.r - 1.f;
	normal.y = 2.f * sampleValue.g - 1.f;
	normal.z = 2.f * sampleValue.b - 1.f;

	normal = tangentSpaceAxis * normal;

	if ( Elite::Dot(-normal, lightDirection) <= 0 )
	{
		observedArea = Elite::Dot(normal, lightDirection);
		pixelColor *= lightColor * lightIntensity * observedArea;
		return;
	}
	pixelColor *= 0;
}


void Mesh::CalculateBoundingBox(const std::vector<uint32_t>& idx, const uint32_t width, const uint32_t height, Elite::IPoint2& boundingBoxMin, Elite::IPoint2& boundingBoxMax)
{
	for ( uint32_t i{}; i < idx.size(); i++ )
	{
		if ( ( m_TransformedVertexBuffer[idx[i]].Position.x >= 0 && m_TransformedVertexBuffer[idx[i]].Position.x <= width ) )
		{
			boundingBoxMin.x = std::min(static_cast<int>( m_TransformedVertexBuffer[idx[i]].Position.x - 1 ), boundingBoxMin.x);
			boundingBoxMax.x = std::max(static_cast<int>( m_TransformedVertexBuffer[idx[i]].Position.x + 1 ), boundingBoxMax.x);
		}

		if ( ( m_TransformedVertexBuffer[idx[i]].Position.y >= 0 && m_TransformedVertexBuffer[idx[i]].Position.y <= height ) )
		{
			boundingBoxMin.y = std::min(static_cast<int>( m_TransformedVertexBuffer[idx[i]].Position.y - 1 ), boundingBoxMin.y);
			boundingBoxMax.y = std::max(static_cast<int>( m_TransformedVertexBuffer[idx[i]].Position.y + 1 ), boundingBoxMax.y);
		}
	}
}

const PrimitiveTopology Mesh::GetTopology() const
{
	return m_Topology;
}

void Mesh::SetPrimitiveTopology(const PrimitiveTopology topology)
{
	m_Topology = topology;
}

const std::vector<Vertex_Input>& Mesh::GetVertexBuffer() const
{
	return m_VertexBuffer;
}

std::vector<Vertex_Output>& Mesh::GetTransformedVertexBuffer()
{
	return m_TransformedVertexBuffer;
}

const std::vector<uint32_t>& Mesh::GetIndexBuffer() const
{
	return m_IndexBuffer;
}

BaseEffect* Mesh::GetEffect() const
{
	return m_pEffect;
}

void Mesh::CycleTriangleStripList()
{
	if ( m_IndexBufferList.size() > 0 && m_IndexBufferStrip.size() > 0 )
	{
		printf("[INFO] PrimitiveTopology Set: %s\n", ( m_Topology == PrimitiveTopology::TriangleList ) ? "TriangleStrip" : "TriangleList");
		m_Topology = ( m_Topology == PrimitiveTopology::TriangleList ) ? PrimitiveTopology::TriangleStrip : PrimitiveTopology::TriangleList;
		m_IndexBuffer = ( m_Topology == PrimitiveTopology::TriangleList ) ? m_IndexBufferList : m_IndexBufferStrip;
		m_IndexBuffer = m_IndexBuffer;
	}
}

void Mesh::SetTexture(ID3D11Device* pDevice, const std::string& filePath, const TextureType type)
{
	bool success = false;

	switch ( type )
	{
		case TextureType::Diffuse:
			if ( m_pDiffuseTexture == nullptr )
			{
				m_pDiffuseTexture = new Texture(pDevice, filePath);
				success = m_pDiffuseTexture->IsSet();
			} else
			{
				success = m_pDiffuseTexture->SetTexture(filePath);
			}
			m_pEffect->SetTexturMap(m_pDiffuseTexture->GetShaderResourceView(), TextureResourceType(type));
			break;
		case TextureType::Normal:
			if ( m_pNormalTexture == nullptr )
			{
				m_pNormalTexture = new Texture(pDevice, filePath);
				success = m_pNormalTexture->IsSet();
			} else
			{
				success = m_pNormalTexture->SetTexture(filePath);
			}
			m_pEffect->SetTexturMap(m_pNormalTexture->GetShaderResourceView(), TextureResourceType(type));
			break;
		case TextureType::Specular:
			if ( m_pSpecularTexture == nullptr )
			{
				m_pSpecularTexture = new Texture(pDevice, filePath);
				success = m_pSpecularTexture->IsSet();
			} else
			{
				success = m_pSpecularTexture->SetTexture(filePath);
			}
			m_pEffect->SetTexturMap(m_pSpecularTexture->GetShaderResourceView(), TextureResourceType(type));
			break;
		case TextureType::Glossiness:
			if ( m_pGlossinessTexture == nullptr )
			{
				m_pGlossinessTexture = new Texture(pDevice, filePath);
				success = m_pGlossinessTexture->IsSet();
			} else
			{
				success = m_pGlossinessTexture->SetTexture(filePath);
			}
			m_pEffect->SetTexturMap(m_pGlossinessTexture->GetShaderResourceView(), TextureResourceType(type));
			break;
		default:
			break;
	}

	printf("[INFO] Texture: %s, Set: %s\n", filePath.c_str(), ( success ) ? "SUCCESS" : "FAILED");
}

void Mesh::ToggleTexture(const TextureType type)
{
	switch ( type )
	{
		case TextureType::Diffuse:
			m_pDiffuseTexture->ToggleIsSet();
			break;
		case TextureType::Normal:
			m_pNormalTexture->ToggleIsSet();
			break;
		case TextureType::Specular:
			m_pSpecularTexture->ToggleIsSet();
			break;
		case TextureType::Glossiness:
			m_pGlossinessTexture->ToggleIsSet();
			break;
		default:
			break;
	}
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
{	
	//Set vertex buffer
	UINT stride = sizeof(Vertex_Input);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Set indx buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set the input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	//Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Render a triangle
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pEffect->GetTechnique()->GetDesc(&techDesc);
	for ( UINT p = 0; p < techDesc.Passes; ++p )
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
	}
}

void Mesh::Update(const float deltaTime)
{
	//m_TransformationMatrix *= Elite::MakeTranslation(-Elite::FVector3(m_Position));
	m_W = Elite::MakeRotation(deltaTime, m_V) * m_W;
	SetWorldMatrix(m_V);

	//Elite::MakeRotation(deltaTime, m_V);
}

void Mesh::ParseFile(const char* filePath, bool flipY)
{
	std::ifstream file;
	std::string line;

	file.open(filePath);

	std::vector<Elite::FPoint3> PosVector;
	std::vector<Elite::FVector3> VNorms;
	std::vector<Elite::FVector2> UvVector;

	if ( file.is_open() )
	{
		while ( !file.eof() )
		{
			std::stringstream lineLable;
			std::getline(file, line);

			lineLable << line;
			std::string lable;
			lineLable >> lable;

			if ( line.size() < 1 || lable == "#" )
			{
				continue;
			}

			if ( lable == "v" )
			{
				Elite::FPoint3 tempP{};
				lineLable >> tempP.x;
				lineLable >> tempP.y;
				lineLable >> tempP.z;

				PosVector.push_back(tempP);
			} else if ( lable == "vn" )
			{
				Elite::FVector3 tempVN{};
				lineLable >> tempVN.x;
				lineLable >> tempVN.y;
				lineLable >> tempVN.z;
				tempVN.y *= ( flipY ) ? -1.f : 1.f;

				VNorms.push_back(tempVN);
			}
			if ( lable == "vt" )
			{
				Elite::FVector2 tempUv{};

				lineLable >> tempUv.x;
				lineLable >> tempUv.y;
				tempUv.y = 1.f - tempUv.y;

				UvVector.push_back(tempUv);
			}

			if ( lable == "f" )
			{
				bool add = true;
				std::string faceLine;

				for ( int i{ 0 }; i < 3; ++i )
				{
					lineLable >> faceLine;

					const std::regex myRegex("([0-9]+)\\/*([0-9]+)\\/*([0-9]+)");
					std::smatch sm;

					regex_match(faceLine, sm, myRegex);

					Vertex_Input tempVertex{ PosVector[stoi(sm[1]) - size_t(1)], Elite::FVector3(), VNorms[stoi(sm[3]) - size_t(1)], Elite::FVector3(), UvVector[stoi(sm[2]) - size_t(1)]};

					add = true;
					int j{};
					for ( const Vertex_Input& v : m_VertexBuffer )
					{
						if ( v.Position == tempVertex.Position && v.Uv == tempVertex.Uv )
						{
							add = false;
							m_IndexBuffer.push_back(j);
							break;
						}
						++j;
					}
					if ( add )
					{
						m_VertexBuffer.push_back(Vertex_Input{ tempVertex.Position, m_Position - tempVertex.Position, tempVertex.Normal, tempVertex.Tangent, tempVertex.Uv });
						m_IndexBuffer.push_back(static_cast<unsigned int>( m_VertexBuffer.size() - 1 ));
					}
				}
			}
		}
		file.close();
	}

	for ( uint32_t i{}; i < m_IndexBuffer.size(); i += 3 )
	{
		uint32_t index0 = m_IndexBuffer[i];
		uint32_t index1 = m_IndexBuffer[size_t(i) + 1];
		uint32_t index2 = m_IndexBuffer[size_t(i) + 2];

		const Elite::FPoint3& p0 = m_VertexBuffer[index0].Position;
		const Elite::FPoint3& p1 = m_VertexBuffer[index1].Position;
		const Elite::FPoint3& p2 = m_VertexBuffer[index2].Position;

		const Elite::FVector2& uv0 = m_VertexBuffer[index0].Uv;
		const Elite::FVector2& uv1 = m_VertexBuffer[index1].Uv;
		const Elite::FVector2& uv2 = m_VertexBuffer[index2].Uv;

		const Elite::FVector3 edge0 = p1 - p0;
		const Elite::FVector3 edge1 = p2 - p0;

		const Elite::FVector2 diffX = Elite::FVector2(uv1.x - uv0.x, uv2.x - uv0.x);
		const Elite::FVector2 diffY = Elite::FVector2(uv1.y - uv0.y, uv2.y - uv0.y);

		float r = 1.f / Elite::Cross(diffX, diffY);

		Elite::FVector3 tangent = ( edge0 * diffY.y - edge1 * diffY.x ) * r;

		m_VertexBuffer[index0].Tangent += tangent;
		m_VertexBuffer[index1].Tangent += tangent;
		m_VertexBuffer[index2].Tangent += tangent;
	}

	for ( Vertex_Input v : m_VertexBuffer )
	{
		v.Tangent = Elite::GetNormalized(Elite::Reject(v.Tangent, v.Normal));
	}
}
