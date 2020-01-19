#include "GameObject.h"

GameObject::GameObject(string name, Geometry geometry, Material material) : _geometry(geometry), _name(name), _material(material)
{
	_parent = nullptr;
	_diffuse = nullptr;
	_normal = nullptr;
	_displacement = nullptr;
	_vertexShader = nullptr;
	_pixelShader = nullptr;
	_bEnablePixelShader = true;
	_bEnableVertexShader = true;

	_position = XMFLOAT3();
	_rotation = XMFLOAT3();
	_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);	
}

GameObject::~GameObject()
{
}

void GameObject::Update(float t)
{
	// Calculate world matrix
	XMMATRIX scale = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
	XMMATRIX rotation = XMMatrixRotationX(_rotation.x) * XMMatrixRotationY(_rotation.y) * XMMatrixRotationZ(_rotation.z);
	XMMATRIX translation = XMMatrixTranslation(_position.x, _position.y, _position.z);

	XMStoreFloat4x4(&_world, scale * rotation * translation);

	if (_parent != nullptr)
	{
		XMStoreFloat4x4(&_world, this->GetWorldMatrix() * _parent->GetWorldMatrix());
	}
}

void GameObject::Draw(ID3D11DeviceContext * pImmediateContext)
{
	if (HasPixelShader() && _bEnablePixelShader)
	{
		pImmediateContext->PSSetShader(GetPixelShader(), nullptr, 0);
	}
	if (HasVertexShader() && _bEnableVertexShader)
	{
		pImmediateContext->VSSetShader(GetVertexShader(), nullptr, 0);
	}

	if (HasDiffuseTexture())
	{
		pImmediateContext->PSSetShaderResources(0, 1, &_diffuse);
	}

	if (HasNormalTexture())
	{
		pImmediateContext->PSSetShaderResources(1, 1, &_normal); // Normal Map
	}

	if (HasDisplacementTexture())
	{
		pImmediateContext->PSSetShaderResources(2, 1, &_displacement); // Parallax Map
	}

	pImmediateContext->IASetVertexBuffers(0, 1, &_geometry.vertexBuffer, &_geometry.vertexBufferStride, &_geometry.vertexBufferOffset);
	pImmediateContext->IASetIndexBuffer(_geometry.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(_geometry.numberOfIndices, 0, 0);
}

void GameObject::SetDraw(ID3D11DeviceContext * pImmediateContext)
{
	pImmediateContext->IASetVertexBuffers(0, 1, &_geometry.vertexBuffer, &_geometry.vertexBufferStride, &_geometry.vertexBufferOffset);
	pImmediateContext->IASetIndexBuffer(_geometry.indexBuffer, DXGI_FORMAT_R16_UINT, 0);
}
