#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <string>

using namespace DirectX;
using namespace std;

struct Geometry
{
	ID3D11Buffer * vertexBuffer;
	ID3D11Buffer * indexBuffer;
	int numberOfIndices;

	UINT vertexBufferStride;
	UINT vertexBufferOffset;
};

struct Material
{
	XMFLOAT4 diffuse;
	XMFLOAT4 ambient;
	XMFLOAT4 specular;
	float specularPower;
};


class GameObject
{
public:
	GameObject(string type, Geometry geometry, Material material);
	~GameObject();

	// Setters and Getters for position/rotation/scale
	void SetPosition(XMFLOAT3 position) { _position = position; }
	void SetPosition(float x, float y, float z) { _position.x = x; _position.y = y; _position.z = z; }

	XMFLOAT3 GetPosition() const { return _position; }

	void SetScale(XMFLOAT3 scale) { _scale = scale; }
	void SetScale(float x, float y, float z) { _scale.x = x; _scale.y = y; _scale.z = z; }

	XMFLOAT3 GetScale() const { return _scale; }

	void SetRotation(XMFLOAT3 rotation) { _rotation = rotation; }
	void SetRotation(float x, float y, float z) { _rotation.x = x; _rotation.y = y; _rotation.z = z; }

	XMFLOAT3 GetRotation() const { return _rotation; }

	string GetName() const { return _name; }

	Geometry GetGeometryData() const { return _geometry; }
	int GetNumberOfIndices() const { return _geometry.numberOfIndices; }

	Material GetMaterial() const { return _material; }

	XMMATRIX GetWorldMatrix() const { return XMLoadFloat4x4(&_world); }

	void SetDiffuseTextureRV(ID3D11ShaderResourceView * diffuseTextureRV) { _diffuse = diffuseTextureRV; }
	ID3D11ShaderResourceView * GetDiffuseTextureRV() const { return _diffuse; }
	bool HasDiffuseTexture() const { return _diffuse ? true : false; }

	void SetNormalTextureRV(ID3D11ShaderResourceView * normalTextureRV) { _normal = normalTextureRV; }
	ID3D11ShaderResourceView * GetNormalTextureRV() const { return _normal; }
	bool HasNormalTexture() const { return _normal ? true : false; }

	void SetDisplacementTextureRV(ID3D11ShaderResourceView * displacementTextureRV) { _displacement = displacementTextureRV; }
	ID3D11ShaderResourceView * GetDisplacementTextureRV() const { return _displacement; }
	bool HasDisplacementTexture() const { return _displacement ? true : false; }

	void SetVertexShader(ID3D11VertexShader * vertexshader) { _vertexShader = vertexshader; }
	ID3D11VertexShader * GetVertexShader() const { return _vertexShader; }
	bool HasVertexShader() const { return _vertexShader ? true : false; }

	void SetPixelShader(ID3D11PixelShader * pixelshader) { _pixelShader = pixelshader; }
	ID3D11PixelShader * GetPixelShader() const { return _pixelShader; }
	bool HasPixelShader() const { return _pixelShader ? true : false; }

	void SetEnablePixelShader(bool isenabled) { _bEnablePixelShader = isenabled;  }
	void SetEnableVertexShader(bool isenabled) { _bEnableVertexShader = isenabled; }

	void SetParent(GameObject * parent) { _parent = parent; }

	void Update(float t);
	void Draw(ID3D11DeviceContext * pImmediateContext);
	void SetDraw(ID3D11DeviceContext * pImmediateContext);

private:
	XMFLOAT3 _position;
	XMFLOAT3 _rotation;
	XMFLOAT3 _scale;

	bool _bEnablePixelShader;
	bool _bEnableVertexShader;

	string _name;

	XMFLOAT4X4 _world;

	Geometry _geometry;
	Material _material;

	ID3D11ShaderResourceView* _diffuse;
	ID3D11ShaderResourceView* _normal;
	ID3D11ShaderResourceView* _displacement;

	ID3D11VertexShader*     _vertexShader;
	ID3D11PixelShader*      _pixelShader;


	GameObject * _parent;
};

