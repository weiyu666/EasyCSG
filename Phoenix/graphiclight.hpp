#pragma once
//-----
#include "light.hpp"
#include "objectmanager.hpp"
//----
//---
//--
class GraphicLight
{
	public:
		GraphicLight(_IN_(GLushort & lightSource));
		~GraphicLight();

		GLvoid setPosition(_IN_(ARRAY3REF(GLfloat, position)));

		friend std::basic_ostream<TCHAR> & operator<<(_INOUT_(std::basic_ostream<TCHAR> & out), _IN_(GraphicLight & graphicLight));
		GLvoid operator<<(_IN_(rapidxml::xml_node<TCHAR>* parentNode));

		Light light;
		GraphicObjectG* lightCube;
};

class ComparatorGraphicLight
{
	public:
		ComparatorGraphicLight(GLushort lightSource):
		lightSource(lightSource)
		{
		}

		GLboolean operator()(GraphicLight* graphicLight)
		{
			if(graphicLight == nullptr)
			{
				return GL_FALSE;
			}
			else
			{
				return graphicLight->light.getLightSource() == this->lightSource;
			}
		}

	private:
		GLushort lightSource;
};