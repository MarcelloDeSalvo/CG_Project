// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"
#include <unordered_map>
#define W_WIDTH 1700
#define W_HEIGHT 1200


const std::string MODEL_PATH = "models/museo_prof_remake.obj";
const std::string TEXTURE_PATH = "textures/reamakeLayout.png";

// Mountains
const std::string MODEL_MOUNTAIN = "models/3dmountains.obj";
const std::string TEXTURE_MOUNTAIN = "textures/MyGrid.png";

// Statue
const std::string MODEL_STATUE = "models/Venus.obj";
const std::string TEXTURE_STATUE = "textures/marble.png";


const std::string CARD_MODEL_PATH = "models/card.obj";

const std::vector<std::string> CARD_TEXTURE_PATH = {
	"textures/Desc/Guernica.png",
	"textures/Desc/Cezanne.png",
	"textures/Desc/VanGoghSelf.png",
	"textures/Desc/QuartoStato.png",
	"textures/Desc/Seurat.png",
	"textures/Desc/Colazione.png",
	"textures/Desc/Colazione.png"
};

// The uniform buffer object used in this example
struct GlobalUniformBufferLight {
	alignas(16) glm::vec3 DIR_light_direction;
	alignas(16) glm::vec3 DIR_light_color;

	alignas(16) glm::vec3 POINT_light_pos;
	alignas(16) glm::vec3 POINT_light_direction;
	alignas(16) glm::vec3 POINT_light_color;
	alignas(16) glm::vec4 POINT_coneInOutDecayExp;
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct UniformBufferObjectCard {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(4) int textureID;
};

// MAIN ! 
class MyProject : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	std::unordered_map<int, int> pixel_map;

	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSL1;
	DescriptorSetLayout DSLCard;
	DescriptorSetLayout DSLGlobal;
	DescriptorSet DSGlobal;

	// Pipelines [Shader couples]
	Pipeline P1;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model M1;
	Texture T1;
	DescriptorSet DS1;

	// Mountain
	Model mountainModel;
	Texture mountainTexture;
	DescriptorSet mountainDS;

	// Statue
	Model statueModel;
	Texture statueTexture;
	DescriptorSet statueDS;

	// Card pipeline
	Pipeline PC;

	Model MC;
	Texture TC[TEXTURE_ARRAY_SIZE];
	Texture CardSampler;
	DescriptorSet DSC;
	int pix = 0, textId = 0;
	
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = W_WIDTH;
		windowHeight = W_HEIGHT;
		windowTitle = "My Project";
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
		
		// Descriptor pool sizes ---------------------------------------------------------------------IMPORTANTE CAMBIARE QUI
		uniformBlocksInPool = 10;
		texturesInPool = 20;
		setsInPool = 10;
	}
	
	// Here you load and setup all your Vulkan objects
	void localInit() {
		loadPixelMap();


		// Descriptor Layouts [what will be passed to the shaders]
		//GLOBAL
		DSLGlobal.init(this, {
			// this array contains the binding:
			// first  element : the binding number
			// second element : the time of element (buffer or texture)
			// third  element : the pipeline stage where it will be used
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, 1}
			});

		DSGlobal.init(this, &DSLGlobal, {
				{0, UNIFORM, sizeof(GlobalUniformBufferLight), nullptr}
			});


		DSL1.init(this, {
			// this array contains the binding:
			// first  element : the binding number
			// second element : the time of element (buffer or texture)
			// third  element : the pipeline stage where it will be used
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1}
			});



		DSLCard.init(this, {
			// this array contains the binding:
			// first  element : the binding number
			// second element : the time of element (buffer or texture)
			// third  element : the pipeline stage where it will be used
			// fourth  element : #descriptorCount
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1},
			{1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, TEXTURE_ARRAY_SIZE},
			{2, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1}
			});



		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", { &DSLGlobal, &DSL1 });
		// Models, textures and Descriptors (values assigned to the uniforms)
		M1.init(this, MODEL_PATH);
		T1.init(this, TEXTURE_PATH);
		DS1.init(this, &DSL1, {
			// the second parameter, is a pointer to the Uniform Set Layout of this set
			// the last parameter is an array, with one element per binding of the set.
			// first  elmenet : the binding number
			// second element : UNIFORM or TEXTURE (an enum) depending on the type
			// third  element : only for UNIFORMs, the size of the corresponding C++ object
			// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T1}
			});

		// Mountain
		mountainModel.init(this, MODEL_MOUNTAIN);
		mountainTexture.init(this, TEXTURE_MOUNTAIN);
		mountainDS.init(this, &DSL1, {
				{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
				{1, TEXTURE, 0, &mountainTexture}
			});

		// Statue
		statueModel.init(this, MODEL_STATUE);
		statueTexture.init(this, TEXTURE_STATUE);
		statueDS.init(this, &DSL1, {
				{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
				{1, TEXTURE, 0, &statueTexture}
			});


		// CARD PIPELINE
		PC.init(this, "shaders/CardVert.spv", "shaders/CardFrag.spv", { &DSLCard });
		//PT.init(this, "shaders/vert.spv", "shaders/frag.spv", { &DSL1 }, textVertexDescriptor);
		MC.init(this, CARD_MODEL_PATH);
		for (int i= 0; i < TEXTURE_ARRAY_SIZE; i++) {
			TC[i].init(this, CARD_TEXTURE_PATH[i]);
		}
		CardSampler.init(this, TEXTURE_PATH); //We'll only need the sampler of this Texture struct
		DSC.init(this, &DSLCard, {
			// the second parameter, is a pointer to the Uniform Set Layout of this set
			// the last parameter is an array, with one element per binding of the set.
			// first  elmenet : the binding number
			// second element : UNIFORM or TEXTURE (an enum) depending on the type
			// third  element : only for UNIFORMs, the size of the corresponding C++ object
			// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
						{0, UNIFORM, sizeof(UniformBufferObjectCard), nullptr},
						{1, TEXTURE_ARRAY, 0, TC},
						{2, SAMPLER, 0, &CardSampler}
			});

		//MAP
		loadMap();

		
	}

	void loadPixelMap() {
		pixel_map[21] = 0;
		pixel_map[42] = 1;
		pixel_map[63] = 2;
		pixel_map[84] = 3;
		pixel_map[105] = 4;
		pixel_map[126] = 5;
		pixel_map[147] = 6;
		pixel_map[168] = 7;
		pixel_map[189] = 8;
		pixel_map[210] = 9;
		pixel_map[231] = 10;

	}

	// Here you destroy all the objects you created!		
	void localCleanup() {

		//Global
		DSLGlobal.cleanup();
		DSGlobal.cleanup();

		// Mountain
		mountainDS.cleanup();
		mountainTexture.cleanup();
		mountainModel.cleanup();

		// Statue
		statueDS.cleanup();
		statueTexture.cleanup();
		statueModel.cleanup();

		//MAIN
		DS1.cleanup();
		T1.cleanup();
		M1.cleanup();
		P1.cleanup();

		
		//TEXT
		PC.cleanup();
		for (int i = 0; i < TEXTURE_ARRAY_SIZE; i++) {
			TC[i].cleanup();
		}

		MC.cleanup();
		DSC.cleanup();
		CardSampler.cleanup();
		DSL1.cleanup();
		DSLCard.cleanup();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
	
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.graphicsPipeline);
				
		VkBuffer vertexBuffers[] = {M1.vertexBuffer};
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		// property .indexBuffer of models, contains the VkBuffer handle to its index buffer
		vkCmdBindIndexBuffer(commandBuffer, M1.indexBuffer, 0,
								VK_INDEX_TYPE_UINT32);

		// property .pipelineLayout of a pipeline contains its layout.
		// property .descriptorSets of a descriptor set contains its elements.
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 0, 1, &DSGlobal.descriptorSets[currentImage],
			0, nullptr);

		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &DS1.descriptorSets[currentImage],
			0, nullptr);
						
		// property .indices.size() of models, contains the number of triangles * 3 of the mesh.
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);

		// Mountain
		VkBuffer vertexBuffersM[] = { mountainModel.vertexBuffer };
		VkDeviceSize offsetsM[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersM, offsetsM);
		vkCmdBindIndexBuffer(commandBuffer, mountainModel.indexBuffer, 0,
			VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 0, 1, &DSGlobal.descriptorSets[currentImage],

			0, nullptr);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &mountainDS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(mountainModel.indices.size()), 1, 0, 0, 0);

		// Statue
		VkBuffer vertexBuffersS[] = { statueModel.vertexBuffer };
		VkDeviceSize offsetsS[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersS, offsetsS);
		vkCmdBindIndexBuffer(commandBuffer, statueModel.indexBuffer, 0,
			VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 0, 1, &DSGlobal.descriptorSets[currentImage],
			0, nullptr);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &statueDS.descriptorSets[currentImage],
			0, nullptr);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(statueModel.indices.size()), 1, 0, 0, 0);


		//CARD UI PIPELINE BINDING-----------------------------------------------------------------------------------------------------------------
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PC.graphicsPipeline);

		VkBuffer TvertexBuffers[] = { MC.vertexBuffer };
		VkDeviceSize Toffsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, TvertexBuffers, Toffsets);
		vkCmdBindIndexBuffer(commandBuffer, MC.indexBuffer, 0,
			VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			PC.pipelineLayout, 0, 1, &DSC.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MC.indices.size()), 1, 0, 0, 0);
		

	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {

		//TIME
		static auto startTime = std::chrono::high_resolution_clock::now();
		static float lastTime = 0.0f;

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
			(currentTime - startTime).count();
		float deltaT = time - lastTime;
		lastTime = time;

		static float modTime = 0;
		float mod = 10;
		modTime = modTime + 0.01f * deltaT;
		if (modTime >= mod) {
			modTime = 0;
		}
		float ang = abs(sin(modTime / mod * 360.0f));

		std::cout << ang << std::endl;


		//LIGHTS GUBO
		GlobalUniformBufferLight gubo{};
		gubo.DIR_light_direction = glm::vec3(-0.4830f, 0.8365f, -0.2588f);
		gubo.DIR_light_color = glm::vec3(0.7f, 0.1f, 0.1f);

		gubo.POINT_light_color = glm::vec4(1.0f);
		gubo.POINT_light_pos = glm::vec4(3.0f,1.0f,-0.2f,0.0f);
		gubo.POINT_light_direction = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
		gubo.POINT_coneInOutDecayExp = glm::vec4(0.9f, 0.92f, 2.0f, 15.0f);


		//PLAYER MOVEMENT VARIABLES
		const float ROT_SPEED = glm::radians(60.0f);
		const float MOVE_SPEED = 1.0f;
		const float MOUSE_RES = 500.0f;

		static double old_xpos = 0, old_ypos = 0;
		double xpos, ypos;
		// Map
		glm::vec3 oldCamPos = CamPos;
		float aspect_ratio = swapChainExtent.width / (float)swapChainExtent.height;

		//CURSOR POSITION
		glfwGetCursorPos(window, &xpos, &ypos);
		double m_dx = xpos - old_xpos;
		double m_dy = ypos - old_ypos;
		old_xpos = xpos; old_ypos = ypos;

		//UBO
		UniformBufferObjectCard ubo_UI{};
		ubo_UI.model = glm::translate(glm::mat4(1), glm::vec3(200, 1, 1));
		ubo_UI.proj = glm::ortho(-2.0f, 2.0f, -2.0f / aspect_ratio, 2.0f / aspect_ratio, -0.1f, 12.0f);
		ubo_UI.view = glm::mat4(1.0f);
		ubo_UI.textureID = textId;


		UniformBufferObject ubo{};

		//CURSOR CAMERA MOVEMENT
		glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			CamAng.y += m_dx * ROT_SPEED / MOUSE_RES;	//PITCH
			CamAng.x += m_dy * ROT_SPEED / MOUSE_RES;	//YAW
		}


		//KEY PRESS MOVEMENT
		static float debounce = time;

		static bool xray = false;

		if (glfwGetKey(window, GLFW_KEY_X)) {
			if (time - debounce > 0.33) {
				xray = !xray;
				debounce = time;
				
			}
		}

		
		if (glfwGetKey(window, GLFW_KEY_SPACE) && pix!=255 && pix!=0) {
			ubo_UI.model = glm::mat4(1);
			if(pixel_map[pix]<CARD_TEXTURE_PATH.size())
				textId = pixel_map[pix];
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT)) {
			CamAng.y += deltaT * ROT_SPEED;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
			CamAng.y -= deltaT * ROT_SPEED;
		}
		if (glfwGetKey(window, GLFW_KEY_UP)) {
			CamAng.x += deltaT * ROT_SPEED;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN)) {
			CamAng.x -= deltaT * ROT_SPEED;
		}
		/*
		* 		if (glfwGetKey(window, GLFW_KEY_Q)) {
			CamAng.z -= deltaT * ROT_SPEED;
		}
		if (glfwGetKey(window, GLFW_KEY_E)) {
			CamAng.z += deltaT * ROT_SPEED;
		}
		*/


		glm::mat3 CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.y, glm::vec3(0.0f, 1.0f, 0.0f))) *
						   glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.x, glm::vec3(1.0f, 0.0f, 0.0f))) *
						   glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.z, glm::vec3(0.0f, 0.0f, 1.0f)));


		if (glfwGetKey(window, GLFW_KEY_A)) {
			CamPos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)) * deltaT;
		}
		if (glfwGetKey(window, GLFW_KEY_D)) {
			CamPos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)) * deltaT;
		}
		if (glfwGetKey(window, GLFW_KEY_S)) {
			CamPos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1)) * deltaT;
		}
		if (glfwGetKey(window, GLFW_KEY_W)) {
			CamPos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1)) * deltaT;
		}
		if (glfwGetKey(window, GLFW_KEY_F)) {
			CamPos -= MOVE_SPEED * glm::vec3(0, 1, 0) * deltaT;
		}
		if (glfwGetKey(window, GLFW_KEY_R)) {
			CamPos += MOVE_SPEED * glm::vec3(0, 1, 0) * deltaT;
		}

		if (!canStep(CamPos.x, CamPos.z)) {
			CamPos = oldCamPos;
		}

		/*if (CamPos.x < -7.5 || CamPos.x > 1.5) {
			CamPos = oldCamPos;
		}

		if (CamPos.z < -1.5 || CamPos.z > 3.5) {
			CamPos = oldCamPos;
		}*/

		if (CamPos.x != oldCamPos.x || CamPos.z != oldCamPos.z) {
			//std::cout << CamPos.x << ' ' << CamPos.y << ' ' << CamPos.z << std::endl;
		}
		

		//WORLD MATRIX 
		ubo.model = glm::mat4(1.0f);
		

		//CAMERA VIEW MATRIX
		glm::mat4 CamMat = glm::translate(glm::transpose(glm::mat4(CamDir)), -CamPos);
		ubo.view = CamMat;

		//CAMERA PROJECTION MATRIX 
		glm::mat4 out = glm::perspective(glm::radians(90.0f), aspect_ratio, 0.1f, 100.0f);
		out[1][1] *= -1;
		ubo.proj = out;

		//WVP MATRIX  = Project * View * World		Calculated directly inside shader.vert for the position

		// Mountain
		UniformBufferObject mountainUbo;
		mountainUbo.model = glm::mat4(1.0f);
		mountainUbo.view = CamMat;
		mountainUbo.proj = out;

		// Statue
		UniformBufferObject statueUbo;
		statueUbo.model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, ang, 0.0f));
		statueUbo.view = CamMat;
		statueUbo.proj = out;


		// Here is where you actually update your uniforms
		void* data;

		//MAIN (Museum and paintings)
		vkMapMemory(device, DS1.uniformBuffersMemory[0][currentImage], 0,
			sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS1.uniformBuffersMemory[0][currentImage]);


		// Mountain
		vkMapMemory(device, mountainDS.uniformBuffersMemory[0][currentImage], 0,
			sizeof(mountainUbo), 0, &data);
		memcpy(data, &mountainUbo, sizeof(mountainUbo));
		vkUnmapMemory(device, mountainDS.uniformBuffersMemory[0][currentImage]);


		// Statue
		vkMapMemory(device, statueDS.uniformBuffersMemory[0][currentImage], 0,
			sizeof(statueUbo), 0, &data);
		memcpy(data, &statueUbo, sizeof(statueUbo));
		vkUnmapMemory(device, statueDS.uniformBuffersMemory[0][currentImage]);

		//CARD
		vkMapMemory(device, DSC.uniformBuffersMemory[0][currentImage], 0,
			sizeof(ubo_UI), 0, &data);
		memcpy(data, &ubo_UI, sizeof(ubo_UI));
		vkUnmapMemory(device, DSC.uniformBuffersMemory[0][currentImage]);

		//GLOBAL
		vkMapMemory(device, DSGlobal.uniformBuffersMemory[0][currentImage], 0,
			sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DSGlobal.uniformBuffersMemory[0][currentImage]);
	}


	// Map
	stbi_uc* stationMap;
	int stationMapWidth, stationMapHeight;
	const float checkRadius = 0.1;
	const int checkSteps = 12;
	glm::vec2 oldMapPos = glm::vec2(0, 0);
	

	void loadMap() {
		stationMap = stbi_load("textures/museumMapNoOff.png",
			&stationMapWidth, &stationMapHeight,
			NULL, 1);
		if (!stationMap) {
			std::cout << "textures/museumMap.png" << "\n";
			throw std::runtime_error("failed to load map image!");
		}
		std::cout << "Station map -> size: " << stationMapWidth
			<< "x" << stationMapHeight << "\n";
	}

	bool canStepPoint(float x, float y) {
		int pixX = stationMapWidth - round(fmax(0.0f, fmin(stationMapWidth - 1, (-x/9.0f) * stationMapWidth)));
		int pixY = round(fmax(0.0f, fmin(stationMapHeight - 1, (y/5.0f) * stationMapHeight)));

		pix = (int)stationMap[stationMapWidth * pixY + pixX];

		if ((int)oldMapPos.x != pixX || (int)oldMapPos.y != pixY) {
			//std::cout << pixX << " " << pixY << " " << x << " " << y << " \t P = " << pix << "\n";
			if (pix == 0) {
				std::cout << "HIT!" << std::endl;
			}
		}

		oldMapPos.x = pixX;
		oldMapPos.y = pixY;
		
		return pix != 0;
	}
	
	bool canStep(float x, float y) {
		for (int i = 0; i < checkSteps; i++) {
			if (!canStepPoint(x + cos(6.2832 * i / (float)checkSteps) * checkRadius,
				y + sin(6.2832 * i / (float)checkSteps) * checkRadius)) {
				return false;
			}
		}
		return true;
	}
};


// This is the main: probably you do not need to touch this!
int main() {
    MyProject app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}