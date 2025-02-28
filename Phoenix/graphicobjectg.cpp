#include "graphicobjectg.hpp"

const std::array<GLfloat, 4> GraphicObjectG::lineColor{0.1f, 0.1f, 0.1f, 1.0f};

GraphicObjectG::GraphicObjectG(_IN_(std::vector<GVERTEX>* vertices), _IN_(GLenum & primitiveType), _IN_(std::vector<GLuint>* indices), Object::OBJECTTYPE type, std::array<GLfloat, 4> parameters):
GraphicObject(vertices, primitiveType, indices),
type(type),
parameters(parameters),
texture(0)
{
}

GraphicObjectG::~GraphicObjectG()
{
}

GLvoid GraphicObjectG::drawObject(_IN_(GLuint & shaderProgram), SceneObject::DRAWMODE mode, GLboolean fixedPipeline)
{
	GLboolean textureOn = GL_FALSE;

	if(!this->bindVao())
	{
		this->bindVbo();
		this->bindEbo();
	}

	if(shaderProgram != 0)
	{
		this->setShaderModelVar(shaderProgram);
	}
	
	//for OpenCSG begin
	if(mode == SceneObject::DRAWMODE::CSG)
	{
		if(fixedPipeline)
		{
			glPushMatrix();
			glm::mat4 model = glm::mat4(1.0f);
			this->transformation.multModelMatrix(model);
			glMultMatrixf(glm::value_ptr(model));
		}
	}
	//for OpenCSG end

	switch(mode)
	{
		case SceneObject::DRAWMODE::WIRE:
			{
				ShaderProgram::setVector<4>(shaderProgram, std::basic_string<GLchar>("color"), &GraphicObjectG::lineColor);

				glLineWidth(1.5f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDrawElements(this->primitiveType, this->indices->size(), GL_UNSIGNED_INT, nullptr);
			}
			break;

		case SceneObject::DRAWMODE::COLOR:
			{
				ShaderProgram::setVector<4>(shaderProgram, std::basic_string<GLchar>("color"), &this->objectColor);

				glPolygonOffset(1.0f, 1.0f);

				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonMode(GL_FRONT, GL_FILL);
				glDrawElements(this->primitiveType, this->indices->size(), GL_UNSIGNED_INT, nullptr);
				glDisable(GL_POLYGON_OFFSET_FILL);

				ShaderProgram::setVector<4>(shaderProgram, std::basic_string<GLchar>("color"), &GraphicObjectG::lineColor);

				glLineWidth(1.5f);
				glPolygonMode(GL_FRONT, GL_LINE);
				glDrawElements(this->primitiveType, this->indices->size(), GL_UNSIGNED_INT, nullptr);
			}
			break;

		case SceneObject::DRAWMODE::CSG:
		case SceneObject::DRAWMODE::TEXTURE:
			{
				if(shaderProgram > 0)
				{
					if(this->texture > 0)
					{
						textureOn = glIsTexture(this->texture);
					}

					if(!textureOn)
					{
						ShaderProgram::setBool(shaderProgram, std::basic_string<GLchar>("texture"), GL_FALSE);
					}
					else
					{
						ShaderProgram::setBool(shaderProgram, std::basic_string<GLchar>("texture"), GL_TRUE);

						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, this->texture);
					}
				}
			} //without break
		case SceneObject::DRAWMODE::LIGHTING:
			{
				if(shaderProgram > 0)
				{
					ShaderProgram::setVector<4>(shaderProgram, std::basic_string<GLchar>("material.ambient"), this->material.getMaterialValues(Material::MATERIALTYPE::AMBIENT));
					ShaderProgram::setVector<4>(shaderProgram, std::basic_string<GLchar>("material.diffuse"), this->material.getMaterialValues(Material::MATERIALTYPE::DIFFUSE));
					ShaderProgram::setVector<4>(shaderProgram, std::basic_string<GLchar>("material.specular"), this->material.getMaterialValues(Material::MATERIALTYPE::SPECULAR));
					ShaderProgram::setVector<4>(shaderProgram, std::basic_string<GLchar>("material.emissive"), this->material.getMaterialValues(Material::MATERIALTYPE::EMISSIVE));
				}
			} //without break
		case SceneObject::DRAWMODE::SELECT:
			{
				glPolygonMode(GL_FRONT, GL_FILL);
				glDrawElements(this->primitiveType, this->indices->size(), GL_UNSIGNED_INT, nullptr);

				if(shaderProgram > 0)
				{
					if(textureOn)
					{
						ShaderProgram::setBool(shaderProgram, std::basic_string<GLchar>("texture"), GL_FALSE);
					}
				}
			}
			break;
	}

	//for OpenCSG begin
	if(mode == SceneObject::DRAWMODE::CSG)
	{
		if(fixedPipeline)
		{
			glPopMatrix();
		}
	}
	//for OpenCSG end

	this->unbind();
}

GraphicObject* GraphicObjectG::clone()
{
	GraphicObjectG* clone = nullptr;

	std::vector<GLuint>* cloneIndices = nullptr;
	std::vector<GVERTEX>* cloneVertices = Object2D::alocHelper(this->vertices->size(), this->indices->size(), &cloneIndices);

	if(cloneVertices != nullptr)
	{
		std::copy(this->vertices->begin(), this->vertices->end(), cloneVertices->begin());
		std::copy(this->indices->begin(), this->indices->end(), cloneIndices->begin());

		clone = new GraphicObjectG(cloneVertices, this->primitiveType, cloneIndices, this->type, this->parameters);

		if(clone != nullptr)
		{
			clone->transformation = this->transformation;
			clone->material = this->material;
			clone->texture = this->texture;
		}
	}

	return clone;
}

GLvoid GraphicObjectG::filePrintf(_INOUT_(std::basic_ostream<TCHAR> & out)) const
{
	FileIO::FormatNumArray4Out<GLfloat> formatedFloat(out);

	FileIO::Export::push(out, _T("type"));
	out << _T("G");
	FileIO::Export::pop(out);

	FileIO::Export::push(out, _T("primitive"));
	out << static_cast<GLubyte>(this->type);
	FileIO::Export::pop(out);

	FileIO::Export::push(out, _T("parameters"));
	std::for_each(this->parameters.begin(), this->parameters.end(), formatedFloat);
	FileIO::Export::pop(out);

	GraphicObject::filePrintf(out);

	FileIO::Export::push(out, _T("material"));
	out << this->material;
	FileIO::Export::pop(out);
}

GLvoid GraphicObjectG::fileScanf(_IN_(rapidxml::xml_node<TCHAR>* parentNode))
{
	GraphicObject::fileScanf(parentNode);

	rapidxml::xml_node<TCHAR>* node = parentNode->first_node(_T("material"));

	if(node != nullptr)
	{
		this->material << node;
	}
}