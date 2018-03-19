
/*
==============================================================================

DockableWindows.cpp
Created: 18 March 2018 10:30pm
Author:  Akira DeMoss

==============================================================================
*/

/**
*  In this module, we create and define components.
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "JDockableWindows.h"
#include "JAdvancedDock.h"
#include <map>



/**
* The ImageView provides the base component where our selected
* image will be displayed. 
*/
class ImageView : public Component
{
public:
	ImageView(const String & componentName)
	{
		Component::setName(componentName);
		setOpaque(true);
		addAndMakeVisible(imagePreview);
	}
	~ImageView()
	{
		jassertfalse;
	}
	void paint(Graphics& g) override
	{
		g.fillAll(Colours::white);
	}

	void resized() override
	{
		imagePreview.setBounds(getLocalBounds());
	}
	ImageComponent imagePreview;
};


/**
*  File FileBrowserView provides the base component where our selected
*  image will be displayed.  Additionally, the FileBrowserView extends
*  the FileBrowserListener, which will update upon a new file being selected.
*  For the purpose of this proof-of-concept demonstration, the FileBrowserListener
*  will update the ImageView when a file type bears an extension recognized by
*  ImageComponent.  In order to enable this functionality, an ImageView instance
*  is passed to FileBrowserView instance in the implementation module (MainComponent.cpp)
*/
class FileBrowserView
	:
	public Component,
	public FileBrowserListener
{
public:
	FileBrowserView(const String & componentName, ImageView & img)
		: image (img), imagesWildcardFilter("*.jpeg;*.jpg;*.png;*.gif", "*", "Image File Filter"),
		thread("Image File Scanner Thread"),
		directoryList (nullptr, thread),
		fileTreeComp(directoryList)
	{
		Component::setName(componentName);
		setOpaque(true);

		directoryList.setDirectory(File::getSpecialLocation(File::userHomeDirectory), true, true);
		thread.startThread(3);
		fileTreeComp.addListener(this);
		fileTreeComp.setColour(TreeView::backgroundColourId, Colours::grey);
		addAndMakeVisible(fileTreeComp);

	}

	~FileBrowserView()
	{
		fileTreeComp.removeListener(this);
		jassertfalse;
	}

	void paint(Graphics & g) override
	{
		g.fillAll(Colours::white);
	}

	void resized() override
	{
		fileTreeComp.setBounds(getLocalBounds());
	}

private:
	WildcardFileFilter imagesWildcardFilter;
	TimeSliceThread thread;
	DirectoryContentsList directoryList;
	FileTreeComponent fileTreeComp;
	ImageView & image;

	void selectionChanged() override
	{
		const File selectedFile(fileTreeComp.getSelectedFile());

		if (selectedFile.existsAsFile())
			image.imagePreview.setImage(ImageCache::getFromFile(selectedFile));
	}

	void fileClicked(const File&, const MouseEvent&) override {}
	void fileDoubleClicked(const File&) override {}
	void browserRootChanged(const File&) override {}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileBrowserView)
};






//==============================================================================
/**
*  Adding This in from the OpenGLAppTutorial so that we can incorporate an additional OpenGLView.
*  To complete this concept, we would use a strategy similar to how we passed am object
*  reference of the ImageView class to the FileBrowser class, so that when the
*  listener detects an appropriate file that is selected, the the OpenGLView will change.
*  However, this is slightly different in the respect that in the Shape struct, we are
*  directly referencing the Resources directory.  We will leave this up to future work while
*  observing the proof of concept with the ImageView example.
*/
class WavefrontObjFile
{
public:
	WavefrontObjFile() {}

	Result load(const String& objFileContent)
	{
		shapes.clear();
		return parseObjFile(StringArray::fromLines(objFileContent));
	}

	Result load(const File& file)
	{
		sourceFile = file;
		return load(file.loadFileAsString());
	}

	//==============================================================================
	typedef juce::uint32 Index;

	struct Vertex { float x, y, z; };
	struct TextureCoord { float x, y; };

	struct Mesh
	{
		Array<Vertex> vertices, normals;
		Array<TextureCoord> textureCoords;
		Array<Index> indices;
	};

	struct Material
	{
		Material() noexcept  : shininess(1.0f), refractiveIndex(0.0f)
		{
			zerostruct(ambient);
			zerostruct(diffuse);
			zerostruct(specular);
			zerostruct(transmittance);
			zerostruct(emission);
		}

		String name;

		Vertex ambient, diffuse, specular, transmittance, emission;
		float shininess, refractiveIndex;

		String ambientTextureName, diffuseTextureName,
			specularTextureName, normalTextureName;

		StringPairArray parameters;
	};

	struct Shape
	{
		String name;
		Mesh mesh;
		Material material;
	};

	OwnedArray<Shape> shapes;

private:
	//==============================================================================
	File sourceFile;

	struct TripleIndex
	{
		TripleIndex() noexcept : vertexIndex(-1), textureIndex(-1), normalIndex(-1) {}

		bool operator< (const TripleIndex& other) const noexcept
		{
			if (this == &other)
				return false;

			if (vertexIndex != other.vertexIndex)
				return vertexIndex < other.vertexIndex;

			if (textureIndex != other.textureIndex)
				return textureIndex < other.textureIndex;

			return normalIndex < other.normalIndex;
		}

		int vertexIndex, textureIndex, normalIndex;
	};

	struct IndexMap
	{
		std::map<TripleIndex, Index> map;

		Index getIndexFor(TripleIndex i, Mesh& newMesh, const Mesh& srcMesh)
		{
			const std::map<TripleIndex, Index>::iterator it(map.find(i));

			if (it != map.end())
				return it->second;

			auto index = (Index)newMesh.vertices.size();

			if (isPositiveAndBelow(i.vertexIndex, srcMesh.vertices.size()))
				newMesh.vertices.add(srcMesh.vertices.getReference(i.vertexIndex));

			if (isPositiveAndBelow(i.normalIndex, srcMesh.normals.size()))
				newMesh.normals.add(srcMesh.normals.getReference(i.normalIndex));

			if (isPositiveAndBelow(i.textureIndex, srcMesh.textureCoords.size()))
				newMesh.textureCoords.add(srcMesh.textureCoords.getReference(i.textureIndex));

			map[i] = index;
			return index;
		}
	};

	static float parseFloat(String::CharPointerType& t)
	{
		t = t.findEndOfWhitespace();
		return (float)CharacterFunctions::readDoubleValue(t);
	}

	static Vertex parseVertex(String::CharPointerType t)
	{
		Vertex v;
		v.x = parseFloat(t);
		v.y = parseFloat(t);
		v.z = parseFloat(t);
		return v;
	}

	static TextureCoord parseTextureCoord(String::CharPointerType t)
	{
		TextureCoord tc;
		tc.x = parseFloat(t);
		tc.y = parseFloat(t);
		return tc;
	}

	static bool matchToken(String::CharPointerType& t, const char* token)
	{
		auto len = (int)strlen(token);

		if (CharacterFunctions::compareUpTo(CharPointer_ASCII(token), t, len) == 0)
		{
			auto end = t + len;

			if (end.isEmpty() || end.isWhitespace())
			{
				t = end.findEndOfWhitespace();
				return true;
			}
		}

		return false;
	}

	struct Face
	{
		Face(String::CharPointerType t)
		{
			while (!t.isEmpty())
				triples.add(parseTriple(t));
		}

		Array<TripleIndex> triples;

		void addIndices(Mesh& newMesh, const Mesh& srcMesh, IndexMap& indexMap)
		{
			TripleIndex i0(triples[0]), i1, i2(triples[1]);

			for (auto i = 2; i < triples.size(); ++i)
			{
				i1 = i2;
				i2 = triples.getReference(i);

				newMesh.indices.add(indexMap.getIndexFor(i0, newMesh, srcMesh));
				newMesh.indices.add(indexMap.getIndexFor(i1, newMesh, srcMesh));
				newMesh.indices.add(indexMap.getIndexFor(i2, newMesh, srcMesh));
			}
		}

		static TripleIndex parseTriple(String::CharPointerType& t)
		{
			TripleIndex i;

			t = t.findEndOfWhitespace();
			i.vertexIndex = t.getIntValue32() - 1;
			t = findEndOfFaceToken(t);

			if (t.isEmpty() || t.getAndAdvance() != '/')
				return i;

			if (*t == '/')
			{
				++t;
			}
			else
			{
				i.textureIndex = t.getIntValue32() - 1;
				t = findEndOfFaceToken(t);

				if (t.isEmpty() || t.getAndAdvance() != '/')
					return i;
			}

			i.normalIndex = t.getIntValue32() - 1;
			t = findEndOfFaceToken(t);
			return i;
		}

		static String::CharPointerType findEndOfFaceToken(String::CharPointerType t) noexcept
		{
			return CharacterFunctions::findEndOfToken(t, CharPointer_ASCII("/ \t"), String().getCharPointer());
		}
	};

	static Shape* parseFaceGroup(const Mesh& srcMesh,
		const Array<Face>& faceGroup,
		const Material& material,
		const String& name)
	{
		if (faceGroup.size() == 0)
			return nullptr;

		std::unique_ptr<Shape> shape(new Shape());
		shape->name = name;
		shape->material = material;

		IndexMap indexMap;

		for (auto& f : faceGroup)
			f.addIndices(shape->mesh, srcMesh, indexMap);

		return shape.release();
	}

	Result parseObjFile(const StringArray& lines)
	{
		Mesh mesh;
		Array<Face> faceGroup;

		Array<Material> knownMaterials;
		Material lastMaterial;
		String lastName;

		for (auto lineNum = 0; lineNum < lines.size(); ++lineNum)
		{
			auto l = lines[lineNum].getCharPointer().findEndOfWhitespace();

			if (matchToken(l, "v")) { mesh.vertices.add(parseVertex(l));            continue; }
			if (matchToken(l, "vn")) { mesh.normals.add(parseVertex(l));             continue; }
			if (matchToken(l, "vt")) { mesh.textureCoords.add(parseTextureCoord(l)); continue; }
			if (matchToken(l, "f")) { faceGroup.add(Face(l));                       continue; }

			if (matchToken(l, "usemtl"))
			{
				auto name = String(l).trim();

				for (auto i = knownMaterials.size(); --i >= 0;)
				{
					if (knownMaterials.getReference(i).name == name)
					{
						lastMaterial = knownMaterials.getReference(i);
						break;
					}
				}

				continue;
			}

			if (matchToken(l, "mtllib"))
			{
				Result r = parseMaterial(knownMaterials, String(l).trim());
				continue;
			}

			if (matchToken(l, "g") || matchToken(l, "o"))
			{
				if (Shape* shape = parseFaceGroup(mesh, faceGroup, lastMaterial, lastName))
					shapes.add(shape);

				faceGroup.clear();
				lastName = StringArray::fromTokens(l, " \t", "")[0];
				continue;
			}
		}

		if (auto* shape = parseFaceGroup(mesh, faceGroup, lastMaterial, lastName))
			shapes.add(shape);

		return Result::ok();
	}

	Result parseMaterial(Array<Material>& materials, const String& filename)
	{
		jassert(sourceFile.exists());
		auto f = sourceFile.getSiblingFile(filename);

		if (!f.exists())
			return Result::fail("Cannot open file: " + filename);

		auto lines = StringArray::fromLines(f.loadFileAsString());

		materials.clear();
		Material material;

		for (auto line : lines)
		{
			auto l = line.getCharPointer().findEndOfWhitespace();

			if (matchToken(l, "newmtl")) { materials.add(material); material.name = String(l).trim(); continue; }

			if (matchToken(l, "Ka")) { material.ambient = parseVertex(l); continue; }
			if (matchToken(l, "Kd")) { material.diffuse = parseVertex(l); continue; }
			if (matchToken(l, "Ks")) { material.specular = parseVertex(l); continue; }
			if (matchToken(l, "Kt")) { material.transmittance = parseVertex(l); continue; }
			if (matchToken(l, "Ke")) { material.emission = parseVertex(l); continue; }
			if (matchToken(l, "Ni")) { material.refractiveIndex = parseFloat(l);  continue; }
			if (matchToken(l, "Ns")) { material.shininess = parseFloat(l);  continue; }

			if (matchToken(l, "map_Ka")) { material.ambientTextureName = String(l).trim(); continue; }
			if (matchToken(l, "map_Kd")) { material.diffuseTextureName = String(l).trim(); continue; }
			if (matchToken(l, "map_Ks")) { material.specularTextureName = String(l).trim(); continue; }
			if (matchToken(l, "map_Ns")) { material.normalTextureName = String(l).trim(); continue; }

			auto tokens = StringArray::fromTokens(l, " \t", "");

			if (tokens.size() >= 2)
				material.parameters.set(tokens[0].trim(), tokens[1].trim());
		}

		materials.add(material);
		return Result::ok();
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavefrontObjFile)
};






//==============================================================================
/**
*  This component lives inside our window, and this is where you should put all
*  your controls and content.
*/
class OpenGLView : public OpenGLAppComponent
{
public:
//==============================================================================
	OpenGLView(const String & componentName)
	{
		Component::setName(componentName);
		//openGLContext.attachTo(*this);
		setSize(800, 600);
	}

	~OpenGLView()
	{
		shutdownOpenGL();
		jassertfalse;
	}

	void initialise() override
	{
		createShaders();
	}

	void shutdown() override
	{
		shader.reset();
		shape.reset();
		attributes.reset();
		uniforms.reset();
	}

	Matrix3D<float> getProjectionMatrix() const
	{
		auto w = 1.0f / (0.5f + 0.1f);
		auto h = w * getLocalBounds().toFloat().getAspectRatio(false);

		return Matrix3D<float>::fromFrustum(-w, w, -h, h, 4.0f, 30.0f);
	}

	Matrix3D<float> getViewMatrix() const
	{
		Matrix3D<float> viewMatrix({ 0.0f, 0.0f, -10.0f });
		Matrix3D<float> rotationMatrix = viewMatrix.rotation({ -0.3f, 5.0f * std::sin(getFrameCounter() * 0.01f), 0.0f });

		return rotationMatrix * viewMatrix;
	}

	void render() override
	{
		jassert(OpenGLHelpers::isContextActive());

		auto desktopScale = (float)openGLContext.getRenderingScale();
		OpenGLHelpers::clear(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glViewport(0, 0, roundToInt(desktopScale * getWidth()), roundToInt(desktopScale * getHeight()));

		shader->use();

		if (uniforms->projectionMatrix.get() != nullptr)
			uniforms->projectionMatrix->setMatrix4(getProjectionMatrix().mat, 1, false);

		if (uniforms->viewMatrix.get() != nullptr)
			uniforms->viewMatrix->setMatrix4(getViewMatrix().mat, 1, false);

		shape->draw(openGLContext, *attributes);

		// Reset the element buffers so child Components draw correctly
		openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
		openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}

	void paint(Graphics& g) override
	{
		// You can add your component specific drawing code here!
		// This will draw over the top of the openGL background.
		g.setColour(getLookAndFeel().findColour(Label::textColourId));
		g.setFont(20);
		g.drawText("OpenGL Example", 25, 20, 300, 30, Justification::left);
		g.drawLine(20, 20, 170, 20);
		g.drawLine(20, 50, 170, 50);
	}

	void resized() override
	{
		// This is called when the OpenGLObj is resized.
		// If you add any child components, this is where you should
		// update their positions.
	}

	void createShaders()
	{
		vertexShader =
			"attribute vec4 position;\n"
			"attribute vec4 sourceColour;\n"
			"attribute vec2 texureCoordIn;\n"
			"\n"
			"uniform mat4 projectionMatrix;\n"
			"uniform mat4 viewMatrix;\n"
			"\n"
			"varying vec4 destinationColour;\n"
			"varying vec2 textureCoordOut;\n"
			"\n"
			"void main()\n"
			"{\n"
			"    destinationColour = sourceColour;\n"
			"    textureCoordOut = texureCoordIn;\n"
			"    gl_Position = projectionMatrix * viewMatrix * position;\n"
			"}\n";

		fragmentShader =
#if JUCE_OPENGL_ES
			"varying lowp vec4 destinationColour;\n"
			"varying lowp vec2 textureCoordOut;\n"
#else
			"varying vec4 destinationColour;\n"
			"varying vec2 textureCoordOut;\n"
#endif
			"\n"
			"void main()\n"
			"{\n"
#if JUCE_OPENGL_ES
			"    lowp vec4 colour = vec4(0.95, 0.57, 0.03, 0.7);\n"
#else
			"    vec4 colour = vec4(0.95, 0.57, 0.03, 0.7);\n"
#endif
			"    gl_FragColor = colour;\n"
			"}\n";

		std::unique_ptr<OpenGLShaderProgram> newShader(new OpenGLShaderProgram(openGLContext));
		String statusText;

		if (newShader->addVertexShader(OpenGLHelpers::translateVertexShaderToV3(vertexShader))
			&& newShader->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(fragmentShader))
			&& newShader->link())
		{
			shape.reset();
			attributes.reset();
			uniforms.reset();

			shader.reset(newShader.release());
			shader->use();

			shape.reset(new Shape(openGLContext));
			attributes.reset(new Attributes(openGLContext, *shader));
			uniforms.reset(new Uniforms(openGLContext, *shader));

			statusText = "GLSL: v" + String(OpenGLShaderProgram::getLanguageVersion(), 2);
		}
		else
		{
			statusText = newShader->getLastError();
		}
	}

private:
	//==============================================================================
	struct Vertex
	{
		float position[3];
		float normal[3];
		float colour[4];
		float texCoord[2];
	};

	//==============================================================================
	// This class just manages the attributes that the shaders use.
	struct Attributes
	{
		Attributes(OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram)
		{
			position.reset(createAttribute(openGLContext, shaderProgram, "position"));
			normal.reset(createAttribute(openGLContext, shaderProgram, "normal"));
			sourceColour.reset(createAttribute(openGLContext, shaderProgram, "sourceColour"));
			texureCoordIn.reset(createAttribute(openGLContext, shaderProgram, "texureCoordIn"));
		}

		void enable(OpenGLContext& openGLContext)
		{
			if (position.get() != nullptr)
			{
				openGLContext.extensions.glVertexAttribPointer(position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
				openGLContext.extensions.glEnableVertexAttribArray(position->attributeID);
			}

			if (normal.get() != nullptr)
			{
				openGLContext.extensions.glVertexAttribPointer(normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 3));
				openGLContext.extensions.glEnableVertexAttribArray(normal->attributeID);
			}

			if (sourceColour.get() != nullptr)
			{
				openGLContext.extensions.glVertexAttribPointer(sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 6));
				openGLContext.extensions.glEnableVertexAttribArray(sourceColour->attributeID);
			}

			if (texureCoordIn.get() != nullptr)
			{
				openGLContext.extensions.glVertexAttribPointer(texureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 10));
				openGLContext.extensions.glEnableVertexAttribArray(texureCoordIn->attributeID);
			}
		}

		void disable(OpenGLContext& openGLContext)
		{
			if (position.get() != nullptr)       openGLContext.extensions.glDisableVertexAttribArray(position->attributeID);
			if (normal.get() != nullptr)         openGLContext.extensions.glDisableVertexAttribArray(normal->attributeID);
			if (sourceColour.get() != nullptr)   openGLContext.extensions.glDisableVertexAttribArray(sourceColour->attributeID);
			if (texureCoordIn.get() != nullptr)  openGLContext.extensions.glDisableVertexAttribArray(texureCoordIn->attributeID);
		}

		std::unique_ptr<OpenGLShaderProgram::Attribute> position, normal, sourceColour, texureCoordIn;

	private:
		static OpenGLShaderProgram::Attribute* createAttribute(OpenGLContext& openGLContext,
			OpenGLShaderProgram& shader,
			const char* attributeName)
		{
			if (openGLContext.extensions.glGetAttribLocation(shader.getProgramID(), attributeName) < 0)
				return nullptr;

			return new OpenGLShaderProgram::Attribute(shader, attributeName);
		}
	};

	//==============================================================================
	// This class just manages the uniform values that the demo shaders use.
	struct Uniforms
	{
		Uniforms(OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram)
		{
			projectionMatrix.reset(createUniform(openGLContext, shaderProgram, "projectionMatrix"));
			viewMatrix.reset(createUniform(openGLContext, shaderProgram, "viewMatrix"));
		}

		std::unique_ptr<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix;

	private:
		static OpenGLShaderProgram::Uniform* createUniform(OpenGLContext& openGLContext,
			OpenGLShaderProgram& shaderProgram,
			const char* uniformName)
		{
			if (openGLContext.extensions.glGetUniformLocation(shaderProgram.getProgramID(), uniformName) < 0)
				return nullptr;

			return new OpenGLShaderProgram::Uniform(shaderProgram, uniformName);
		}
	};

	//==============================================================================
	/** This loads a 3D model from an OBJ file and converts it into some vertex buffers
	that we can draw.
	*/
	struct Shape
	{
		Shape(OpenGLContext& openGLContext)
		{
			auto dir = File::getCurrentWorkingDirectory();

			int numTries = 0;

			while (!dir.getChildFile("Resources").exists() && numTries++ < 15)
				dir = dir.getParentDirectory();

			if (shapeFile.load(dir.getChildFile("Resources").getChildFile("humanoid_quad.obj")).wasOk())
				for (auto* shape : shapeFile.shapes)
					vertexBuffers.add(new VertexBuffer(openGLContext, *shape));
		}

		void draw(OpenGLContext& openGLContext, Attributes& glAttributes)
		{
			for (auto* vertexBuffer : vertexBuffers)
			{
				vertexBuffer->bind();

				glAttributes.enable(openGLContext);
				glDrawElements(GL_TRIANGLES, vertexBuffer->numIndices, GL_UNSIGNED_INT, 0);
				glAttributes.disable(openGLContext);
			}
		}

	private:
		struct VertexBuffer
		{
			VertexBuffer(OpenGLContext& context, WavefrontObjFile::Shape& aShape) : openGLContext(context)
			{
				numIndices = aShape.mesh.indices.size();

				openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
				openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

				Array<Vertex> vertices;
				createVertexListFromMesh(aShape.mesh, vertices, Colours::green);

				openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER,
					static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof(Vertex)),
					vertices.getRawDataPointer(), GL_STATIC_DRAW);

				openGLContext.extensions.glGenBuffers(1, &indexBuffer);
				openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
				openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) * sizeof(juce::uint32)),
					aShape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
			}

			~VertexBuffer()
			{
				openGLContext.extensions.glDeleteBuffers(1, &vertexBuffer);
				openGLContext.extensions.glDeleteBuffers(1, &indexBuffer);
			}

			void bind()
			{
				openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
				openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
			}

			GLuint vertexBuffer, indexBuffer;
			int numIndices;
			OpenGLContext& openGLContext;

			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VertexBuffer)
		};

		WavefrontObjFile shapeFile;
		OwnedArray<VertexBuffer> vertexBuffers;

		static void createVertexListFromMesh(const WavefrontObjFile::Mesh& mesh, Array<Vertex>& list, Colour colour)
		{
			auto scale = 0.2f;
			WavefrontObjFile::TextureCoord defaultTexCoord{ 0.5f, 0.5f };
			WavefrontObjFile::Vertex defaultNormal{ 0.5f, 0.5f, 0.5f };

			for (auto i = 0; i < mesh.vertices.size(); ++i)
			{
				const auto& v = mesh.vertices.getReference(i);
				const auto& n = i < mesh.normals.size() ? mesh.normals.getReference(i) : defaultNormal;
				const auto& tc = i < mesh.textureCoords.size() ? mesh.textureCoords.getReference(i) : defaultTexCoord;

				list.add({ { scale * v.x, scale * v.y, scale * v.z, },
					{ scale * n.x, scale * n.y, scale * n.z, },
					{ colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
					{ tc.x, tc.y } });
			}
		}
	};

	const char* vertexShader;
	const char* fragmentShader;

	std::unique_ptr<OpenGLShaderProgram> shader;
	std::unique_ptr<Shape> shape;
	std::unique_ptr<Attributes> attributes;
	std::unique_ptr<Uniforms> uniforms;

	String newVertexShader, newFragmentShader;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGLView)
};













class MainContentComponent : public Component
{
public:
	//==============================================================================
	MainContentComponent();
	~MainContentComponent();

	void paint(Graphics&) override;
	void resized() override;

private:
	DockableWindowManager dockManager;
	WindowDockVertical dock { dockManager };
	TabDock tabDock { dockManager };
	JAdvancedDock advancedDock{ dockManager };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED