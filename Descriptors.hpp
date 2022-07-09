
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct VertexDescriptor {

	std::vector<float> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	

	bool hasPos;
	bool hasNormal;
	bool hasTexCoord;
	bool hasColor;
	bool hasTangent;

	int deltaPos;
	int deltaNormal;
	int deltaTexCoord;
	int deltaColor;
	int deltaTangent;

	int locPos;
	int locNormal;
	int locTexCoord;
	int locColor;
	int locTangent;

	int size;
	int loc;

	VertexDescriptor(bool hPos, bool hNormal, bool hTexCoord, bool hColor, bool hTangent) {
		size = 0;
		loc = 0;

		hasPos = hPos;
		hasNormal = hNormal;
		hasTexCoord = hTexCoord;
		hasColor = hColor;
		hasTangent = hTangent;

		if (hasPos) { deltaPos = size; size += 3; locPos = loc; loc++; }
		else { deltaPos = -1; locPos = -1; }
		if (hasNormal) { deltaNormal = size; size += 3; locNormal = loc; loc++; }
		else { deltaNormal = -1; locNormal = -1; }
		if (hasTexCoord) { deltaTexCoord = size; size += 2; locTexCoord = loc; loc++; }
		else { deltaTexCoord = -1; locTexCoord = -1; }
		if (hasColor) { deltaColor = size; size += 4; locColor = loc; loc++; }
		else { deltaColor = -1; locColor = -1; }
		if (hasTangent) { deltaTangent = size; size += 4; locTangent = loc; loc++; }
		else { deltaTangent = -1; locTangent = -1; }
	}


	VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = size * sizeof(float);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription>
		getAttributeDescriptions(int binding = 0) {
		std::vector<VkVertexInputAttributeDescription>
			attributeDescriptions{};
		attributeDescriptions.resize(loc);
		if (hasPos) {
			attributeDescriptions[locPos].binding = binding;
			attributeDescriptions[locPos].location = locPos;
			attributeDescriptions[locPos].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[locPos].offset = deltaPos * sizeof(float);
		}

		if (hasNormal) {
			attributeDescriptions[locNormal].binding = binding;
			attributeDescriptions[locNormal].location = locNormal;
			attributeDescriptions[locNormal].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[locNormal].offset = deltaNormal * sizeof(float);
		}

		if (hasTexCoord) {
			attributeDescriptions[locTexCoord].binding = binding;
			attributeDescriptions[locTexCoord].location = locTexCoord;
			attributeDescriptions[locTexCoord].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[locTexCoord].offset = deltaTexCoord * sizeof(float);
		}

		if (hasColor) {
			attributeDescriptions[locColor].binding = binding;
			attributeDescriptions[locColor].location = locColor;
			attributeDescriptions[locColor].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[locColor].offset = deltaColor * sizeof(float);
		}

		if (hasTangent) {
			attributeDescriptions[locTangent].binding = binding;
			attributeDescriptions[locTangent].location = locTangent;
			attributeDescriptions[locTangent].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[locTangent].offset = deltaTangent * sizeof(float);
		}

		return attributeDescriptions;
	}

	glm::vec3 getPos(float* data, int i) {
		if (hasPos) {
			return glm::vec3(data[i * size + deltaPos], data[i * size + deltaPos + 1], data[i * size + deltaPos + 2]);
		}
		else {
			return glm::vec3(0.0f);
			std::cerr << "Vertex has no position \n";
		}
	}

	void setPos(float* data, int i, glm::vec3 pos) {
		if (hasPos) {
			data[i * size + deltaPos] = pos.x;
			data[i * size + deltaPos + 1] = pos.y;
			data[i * size + deltaPos + 2] = pos.z;
		}
		else {
			std::cerr << "Vertex has no position \n";
		}
	}

	glm::vec3 getNormal(float* data, int i) {
		if (hasPos) {
			return glm::vec3(data[i * size + deltaNormal], data[i * size + deltaNormal + 1], data[i * size + deltaNormal + 2]);
		}
		else {
			return glm::vec3(0.0f);
			std::cerr << "Vertex has no normal \n";
		}
	}

	void setNormal(float* data, int i, glm::vec3 norm) {
		if (hasNormal) {
			data[i * size + deltaNormal] = norm.x;
			data[i * size + deltaNormal + 1] = norm.y;
			data[i * size + deltaNormal + 2] = norm.z;
		}
		else {
			std::cerr << "Vertex has no normal \n";
		}
	}

	glm::vec2 getTexCoord(float* data, int i) {
		if (hasPos) {
			return glm::vec2(data[i * size + deltaTexCoord], data[i * size + deltaTexCoord + 1]);
		}
		else {
			return glm::vec2(0.0f);
			std::cerr << "Vertex has no UV \n";
		}
	}

	void setTexCoord(float* data, int i, glm::vec3 uv) {
		if (hasNormal) {
			data[i * size + deltaTexCoord] = uv.x;
			data[i * size + deltaTexCoord + 1] = uv.y;
		}
		else {
			std::cerr << "Vertex has no UV \n";
		}
	}
};

