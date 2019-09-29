/**
* @file Geometry.h
*/
#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED
#include <GL/glew.h>

/// 2D ベクトル型
struct Vector2
{
	float x, y;
};

/// 3D ベクトル型
struct Vector3
{
	float x, y, z;
};

/// RGB カラー型
struct Color
{
	float r, g, b, a;
};

/// 頂点データ型
struct Vertex
{
	Vector3 position; ///< 座標
	Color color; ///< 色
	Vector2 texCoord; ///< テクスチャ座標
	Vector3 normal; ///< 法線
};

/**
* ポリゴン表示単位
*/
struct Mesh
{
	GLenum mode;		///<プリミティブの種類
	GLsizei count;		///<描画するインデックス数
	const GLvoid* indices;	///<描画開始インデックスのバイトオフセット
	GLint baseVertex;	///<インデックス０番とみなされる頂点配置内の位置
};


#endif // GEOMETRY_H_INCLUDED
