// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"
#include <unordered_map>
#include "SDL2SoundEffects.h"
#include "SDL2Music.h"
#define W_WIDTH 1700
#define W_HEIGHT 1200

// Museum
const std::string MODEL_PATH = "models/museo_prof_remake.obj";
const std::string TEXTURE_PATH = "textures/reamakeLayout.png";

// Mountains
const std::string MODEL_MOUNTAIN = "models/3dmountains.obj";
const std::string TEXTURE_MOUNTAIN = "textures/MyGrid.png";

// Card
const std::string CARD_MODEL_PATH = "models/card.obj";

// Card Texture Descriptions
const std::vector<std::string> CARD_TEXTURE_PATH = {
	"textures/Desc/Venere.png",
	"textures/Desc/Guernica.png",
	"textures/Desc/Cezanne.png",
	"textures/Desc/VanGoghSelf.png",
	"textures/Desc/QuartoStato.png",
	"textures/Desc/Seurat.png",
	"textures/Desc/Colazione.png",
	"textures/Desc/Montmartre.png",
	"textures/Desc/Munch.png",
	"textures/Desc/Notte.png",
	"textures/Desc/Danza.png",
	"textures/Desc/Monet.png"
};

// Global Uniform used for lights informations
struct GlobalUniformBufferLight {
	alignas(16) glm::vec3 DIR_light_direction;
	alignas(16) glm::vec3 DIR_light_color;

	alignas(16) glm::vec3 SPOT_light_pos;
	alignas(16) glm::vec3 SPOT_light_direction;
	alignas(16) glm::vec3 SPOT_light_color;
	alignas(16) glm::vec4 SPOT_coneInOutDecayExp;

	alignas(16) glm::vec3 AMB_light_color_up;
	alignas(16) glm::vec3 AMB_light_color_down;
};

// Global Uniform for all objects
struct GlobalUniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

// Model Uniform Buffer
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
};


// UBO for Card 
struct UniformBufferObjectCard {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(4) int textureID;
};

// UBO for Skybox
struct UniformBufferObjectSkybox {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

// Skybox
struct SkyBoxModel {
	const char* ObjFile;
	const char* TextureFile[6];
};


const SkyBoxModel SkyBoxToLoad = { "models/SkyBoxCube.obj", 
	{"textures/sky/right.png", "textures/sky/left.png", "textures/sky/top.png", 
	"textures/sky/bottom.png", "textures/sky/front.png", "textures/sky/back.png"} };


// Statue 
struct Statue {
	Model SModel;
	Texture STexture;
	DescriptorSet DSS;
	UniformBufferObject uboStatue;
};


// Vector of all statues
std::vector<Statue> statues;

// Statue model and texture paths
struct Statue_info {
	const std::string model_p;
	const std::string text_p;
};

const std::vector<Statue_info> STATUES_INFO = {
	{ "models/Venus.obj", "textures/marble_4.jpg" },
	{ "models/heliosbust.obj","textures/helios.png"},
	{ "models/stand.obj","textures/marble_4.jpg"},
	{ "models/flamingo.obj","textures/marble_4.jpg"}
};

// MAIN ! 
class MyProject : public BaseProject {
	protected:

	// HASHMAP (Maps the painting with a grey scale value as the ID)
	std::unordered_map<int, int> pixel_map;

	// CAMERA (Initial pos and angle)
	float characterHeight = 0.8f;
	glm::vec3 CamAng = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);	//YAW, PITCH, ROLL
	glm::vec3 CamPos = glm::vec3(4.0f, characterHeight, 2.0f);

	// Animations and audio
	float step = 0.1f;
	float differentialSign = 0.0f;
	int soundEffectIndex = 0;
	SDL2SoundEffects se;
	SDL2Music sm;
	bool playPausePressed = false;
	bool firstPlay = true;
	bool drawCardPressed = false;

	// Pixel map value and current text id (used by Card U.I)
	int pix = 0, textId = 0;

	// Descriptor Set Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSLGlobal;			//DSL for global lights 
	DescriptorSetLayout DSLGlobalModels;	//DSL for global models
	DescriptorSetLayout DSLObjModels;		//DSL for single models

	DescriptorSetLayout DSLCard;			
	DescriptorSetLayout skyBoxDSL;

	//Descriptor sets
	DescriptorSet DSGlobalModels;
	DescriptorSet DSGlobal;

	DescriptorSet DS1;	
	DescriptorSet mountainDS;
	DescriptorSet DSC;

	// Pipelines
	Pipeline P1; // Pipeline for Museum and Mountains
	Pipeline PMarble; //Marble for statues
	Pipeline PC; //Pipeline for card U.I.

	//Custom pipeline for skybox
	Pipeline skyBoxPipeline;
	std::vector<VkBuffer> SkyBoxUniformBuffers;
	std::vector<VkDeviceMemory> SkyBoxUniformBuffersMemory;
	std::vector<VkDescriptorSet> SkyBoxDescriptorSets;	// to access uniforms in the pipeline

	//Models and textures
	Model M1; // Museum
	Texture T1;

	Model mountainModel; // Mountain
	Texture mountainTexture;

	Model skyBox;	// Skybox 
	Texture skyBoxTexture;

	Model MC;	//Card 
	Texture TC[TEXTURE_ARRAY_SIZE]; // Texture Array for all descriptions
	Texture CardSampler;
	
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = W_WIDTH;
		windowHeight = W_HEIGHT;
		windowTitle = "La fabbrica del Vaporwave";
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 10; // Museum + Mountain + 4*Statue + Skybox + Card + Global OBJ  + Global lights
		texturesInPool = 24; // Museum + Mountain + 4*Statue + 6*Skybox + 12*Card
		setsInPool = 10; // Museum + Mountain + 4*Statue + Skybox + Card + Global OBJ  + Global lights
	}
	
	// Here you load and setup all your Vulkan objects
	void localInit() {

		// Maps the painting with a grey scale value as the ID
		loadPixelMap();

		// INIT Descriptor Layouts [what will be passed to the shaders]
		loadDescriptorLayouts();

		// INIT Models and textures
		loadModels();

		// INIT Descriptor Sets
		loadDescriptorSets();

		// INIT Pipelines
		loadPipelines();

		//MAP
		loadMap();

		//Load audio
		loadAudio();
	}

	void loadPixelMap() {
		pixel_map[13] = 0; // Statue
		pixel_map[21] = 1; // Guernica
		pixel_map[42] = 2; // etc..
		pixel_map[63] = 3;
		pixel_map[84] = 4;
		pixel_map[105] = 5;
		pixel_map[126] = 6;
		pixel_map[147] = 7;
		pixel_map[168] = 8;
		pixel_map[189] = 9;
		pixel_map[210] = 10;
		pixel_map[231] = 11;
	}

	void loadDescriptorLayouts() {
		DSLGlobal.init(this, {
			// this array contains the binding:
			// first  element : the binding number
			// second element : the time of element (buffer or texture)
			// third  element : the pipeline stage where it will be used
			// fourth  element : #descriptorCount
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, 1}
			});

		DSLGlobalModels.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, 1},
			});

		DSLObjModels.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1}
			});

		DSLCard.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1},
			{1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, TEXTURE_ARRAY_SIZE},
			{2, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1}
			});

		skyBoxDSL.initSkybox(this);
	}

	void loadDescriptorSets() {
		DSGlobal.init(this, &DSLGlobal, {
			// the second parameter, is a pointer to the Uniform Set Layout of this set
			// the last parameter is an array, with one element per binding of the set.
			// first  elmenet : the binding number
			// second element : UNIFORM or TEXTURE (an enum) depending on the type
			// third  element : only for UNIFORMs, the size of the corresponding C++ object
			// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
				{0, UNIFORM, sizeof(GlobalUniformBufferLight), nullptr}
			});

		DSGlobalModels.init(this, &DSLGlobalModels, {
				{0, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});

		DS1.init(this, &DSLObjModels, {
				{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
				{1, TEXTURE, 0, &T1}
			});

		mountainDS.init(this, &DSLObjModels, {
				{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
				{1, TEXTURE, 0, &mountainTexture}
			});

		DSC.init(this, &DSLCard, {
				{0, UNIFORM, sizeof(UniformBufferObjectCard), nullptr},
				{1, TEXTURE_ARRAY, 0, TC},
				{2, SAMPLER, 0, &CardSampler}
			});
	}
	
	// Pipelines [Shader couples]
	// The last array, is a vector of pointer to the layouts of the sets that will
	// be used in this pipeline. The first element will be set 0, and so on..
	void loadPipelines() {
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", { &DSLGlobal, &DSLGlobalModels, &DSLObjModels });
		PMarble.init(this, "shaders/MarbleVert.spv", "shaders/MarbleFrag.spv", { &DSLGlobal, &DSLGlobalModels, &DSLObjModels });
		PC.init(this, "shaders/CardVert.spv", "shaders/CardFrag.spv", { &DSLCard });
		skyBoxPipeline.init(this, "shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv", { &skyBoxDSL });
	}

	// Models, textures and Descriptors (values assigned to the uniforms)
	void loadModels() {
		
		M1.init(this, MODEL_PATH);
		T1.init(this, TEXTURE_PATH);

		// Mountain
		mountainModel.init(this, MODEL_MOUNTAIN);
		mountainTexture.init(this, TEXTURE_MOUNTAIN);

		// Card
		MC.init(this, CARD_MODEL_PATH);
		for (int i = 0; i < TEXTURE_ARRAY_SIZE; i++) {
			TC[i].init(this, CARD_TEXTURE_PATH[i]);
		}
		CardSampler.init(this, TEXTURE_PATH); //We'll only need the sampler of this Texture struct

		// Statues
		for (Statue_info i : STATUES_INFO)
		{
			Statue s;
			s.SModel.init(this, i.model_p);
			s.STexture.init(this, i.text_p);
			s.DSS.init(this, &DSLObjModels, {
				{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
				{1, TEXTURE, 0, &s.STexture}
				});
			statues.push_back(s);
		}

		// Skybox
		loadSkyBox();
	}

	void loadAudio() {
		se.addSoundEffect("./audio/footstep.wav");
		se.addSoundEffect("./audio/footstep2.wav");
		se.addSoundEffect("./audio/paper.wav");

		sm.addMusicTrack("./audio/Resonance.wav");
		sm.playMusicTrack(0);
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {

		//Global
		DSLGlobal.cleanup();
		DSLGlobalModels.cleanup();
		DSLObjModels.cleanup();
		DSLCard.cleanup();
		skyBoxDSL.cleanup();

		//Global Descriptor sets
		DSGlobal.cleanup();
		DSGlobalModels.cleanup();

		//Pipelines
		P1.cleanup();
		PC.cleanup();
		PMarble.cleanup();
		skyBoxPipeline.cleanup();
		
		//Skybox
		skyBox.cleanup();
		skyBoxTexture.cleanup();
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			vkDestroyBuffer(device, SkyBoxUniformBuffers[i], nullptr);
			vkFreeMemory(device, SkyBoxUniformBuffersMemory[i], nullptr);
		}

		// Mountain
		mountainDS.cleanup();
		mountainTexture.cleanup();
		mountainModel.cleanup();

		// Museum
		DS1.cleanup();
		T1.cleanup();
		M1.cleanup();

		// STATUES
		for each (Statue s in statues)
		{
			s.SModel.cleanup();
			s.STexture.cleanup();
			s.DSS.cleanup();
		}
		
		// Card		
		DSC.cleanup();
		MC.cleanup();
		for (int i = 0; i < TEXTURE_ARRAY_SIZE; i++) {
			TC[i].cleanup();
		}
		CardSampler.cleanup();

	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
	
	//PIPELINE MUSEUM and MOUNTAINS
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.graphicsPipeline);
				
	   //MUSEUM
		VkBuffer vertexBuffers[] = {M1.vertexBuffer}; // property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
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
		vkCmdBindDescriptorSets(commandBuffer,	// Global Descriptor Set Models binding
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &DSGlobalModels.descriptorSets[currentImage],
			0, nullptr);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 2, 1, &DS1.descriptorSets[currentImage],
			0, nullptr);
						
		// property .indices.size() of models, contains the number of triangles * 3 of the mesh.
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);

	   // MOUNTAIN
		VkBuffer vertexBuffersM[] = { mountainModel.vertexBuffer };
		VkDeviceSize offsetsM[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersM, offsetsM);
		vkCmdBindIndexBuffer(commandBuffer, mountainModel.indexBuffer, 0,
			VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 0, 1, &DSGlobal.descriptorSets[currentImage],
			0, nullptr);

		// Global Descriptor Set Models binding
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &DSGlobalModels.descriptorSets[currentImage],
			0, nullptr);

		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 2, 1, &mountainDS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(mountainModel.indices.size()), 1, 0, 0, 0);

	// PIPELINE MARBLE (Statues)
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PMarble.graphicsPipeline);
		
		for each (Statue s in statues)
		{
			VkBuffer vertexBuffersS[] = { s.SModel.vertexBuffer };
			VkDeviceSize offsetsS[] = { 0 };

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersS, offsetsS);
			vkCmdBindIndexBuffer(commandBuffer, s.SModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			// Global Descriptors Set binding
			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PMarble.pipelineLayout, 0, 1, &DSGlobal.descriptorSets[currentImage],
				0, nullptr);
			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PMarble.pipelineLayout, 1, 1, &DSGlobalModels.descriptorSets[currentImage],
				0, nullptr);
			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				PMarble.pipelineLayout, 2, 1, &s.DSS.descriptorSets[currentImage],
				0, nullptr);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(s.SModel.indices.size()), 1, 0, 0, 0);
		}


	//PIPELINE CARD UI  
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
		
	//PIPELINE SKYBOX
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			skyBoxPipeline.graphicsPipeline);
		VkBuffer vertexBuffersSk[] = { skyBox.vertexBuffer};
		VkDeviceSize offsetsSk[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSk, offsetsSk);
		vkCmdBindIndexBuffer(commandBuffer, skyBox.indexBuffer, 0,
			VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			skyBoxPipeline.pipelineLayout, 0, 1,
			&SkyBoxDescriptorSets[currentImage],
			0, nullptr);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(skyBox.indices.size()), 1, 0, 0, 0);
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

		static float modTime = -1;

		//ANIMATIONS 
		float animationCap = 1;
		modTime = modTime + step * deltaT; // modTime goes from -1 to 1 with speed given by step * deltaT
		if (modTime >= animationCap || modTime <= -animationCap) {
			step *= -1;
		}

		//LIGHTS GUBO
		GlobalUniformBufferLight gubo{};
		gubo.DIR_light_direction = glm::vec3(0.6830f, 0.7365f, 0.2588f);
		gubo.DIR_light_color = glm::vec3(0.96f, 0.76f, 0.86f);

		gubo.SPOT_light_color = glm::vec3(0.09f, 0.24f, 0.71f);
		gubo.SPOT_light_pos = glm::vec4(2.0f, 1.0f, 1.5f, 0.0f);
		gubo.SPOT_light_direction = glm::vec3(glm::radians(30.0f), glm::radians(90.0f), glm::radians(0.0f));
		gubo.SPOT_coneInOutDecayExp = glm::vec4(0.92f, 0.94f, 2.0f, 0.0f);

		gubo.AMB_light_color_up = glm::vec3(0.725f, 0.403f, 1.0f); 
		gubo.AMB_light_color_down = glm::vec3(0.003f, 0.803f, 0.996f);

		//PLAYER MOVEMENT VARIABLES
		const float ROT_SPEED = glm::radians(60.0f);
		float MOVE_SPEED = 1.0f;
		const float MOUSE_RES = 500.0f;

		static double old_xpos = 0, old_ypos = 0;
		double xpos, ypos;
		bool isMoving = false;

		glm::vec3 oldCamPos = CamPos;

		//ASPECT RATIO
		float aspect_ratio = swapChainExtent.width / (float)swapChainExtent.height;

		//STATIC OBJECTS 
			// MUSEUM MODEL MATRIX 
		UniformBufferObject ubo_museum{};
		ubo_museum.model = glm::mat4(1.0f);

			// MOUNTAINS
		UniformBufferObject mountainUbo{};
		mountainUbo.model = glm::mat4(1.0f);

		//UI
			//UBO UI CARD
		UniformBufferObjectCard ubo_UI{};
		ubo_UI.model = glm::translate(glm::mat4(1), glm::vec3(200, 1, 1));
		ubo_UI.proj = glm::ortho(-2.0f, 2.0f, -2.0f / aspect_ratio, 2.0f / aspect_ratio, -0.1f, 12.0f);
		ubo_UI.view = glm::mat4(1.0f);
		ubo_UI.textureID = textId;


		//CURSOR POSITION
		glfwGetCursorPos(window, &xpos, &ypos);
		double m_dx = xpos - old_xpos;
		double m_dy = ypos - old_ypos;
		old_xpos = xpos; old_ypos = ypos;


		//CURSOR CAMERA MOVEMENT
		glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			CamAng.y += m_dx * ROT_SPEED / MOUSE_RES;	//PITCH
			CamAng.x += m_dy * ROT_SPEED / MOUSE_RES;	//YAW
		}


		//KEY PRESS MOVEMENT
		static float debounce = time;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
			MOVE_SPEED = 2.5f;
		}

		bool drawCardCurrentyPressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
		int oldTextId = textId;
		if (glfwGetKey(window, GLFW_KEY_SPACE) && pix!=255 && pix!=0) {
			if (pix != 253) { // Hotfix for dispaly card outside museum bug
				ubo_UI.model = glm::mat4(1);

				if (pixel_map[pix] < CARD_TEXTURE_PATH.size()) {
					textId = pixel_map[pix];
				}

				if (oldTextId != textId || (!drawCardPressed && drawCardCurrentyPressed))
					se.playSoundEffect(2);
			}
		}
		drawCardPressed = drawCardCurrentyPressed;


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

		//LOOK IN DIR MAT
		glm::mat3 CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.y, glm::vec3(0.0f, 1.0f, 0.0f))) *
						   glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.x, glm::vec3(1.0f, 0.0f, 0.0f))) *
						   glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.z, glm::vec3(0.0f, 0.0f, 1.0f)));


		if (glfwGetKey(window, GLFW_KEY_A)) {
			CamPos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)) * deltaT;
			isMoving = true;
		}
		if (glfwGetKey(window, GLFW_KEY_D)) {
			CamPos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)) * deltaT;
			isMoving = true;
		}
		if (glfwGetKey(window, GLFW_KEY_S)) {
			CamPos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1)) * deltaT;
			isMoving = true;
		}
		if (glfwGetKey(window, GLFW_KEY_W)) {
			CamPos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y, 
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1)) * deltaT;
			isMoving = true;
		}
		if (glfwGetKey(window, GLFW_KEY_F)) {
			CamPos -= MOVE_SPEED * glm::vec3(0, 1, 0) * deltaT;
		}
		if (glfwGetKey(window, GLFW_KEY_R)) {
			CamPos += MOVE_SPEED * glm::vec3(0, 1, 0) * deltaT;
		}

		// Play/pause music
		bool playPauseCurrentyPressed = (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS);
		if (!playPausePressed && playPauseCurrentyPressed) { 
			if (!firstPlay) {
				sm.playMusicTrack(0);
				firstPlay = true;
			} else 
				sm.Play_Pause();
		}
		playPausePressed = playPauseCurrentyPressed;

		if (!canStep(CamPos.x, CamPos.z)) {
			CamPos = oldCamPos;
		}


		//WALK ANIMATION
		float oldCameraHeitgh = CamPos.y;
		float oldDifferentialSign = differentialSign;
		
		float walk_speed = 40.0f;
		float walk = 0.05f * abs(cos(MOVE_SPEED * walk_speed * modTime)) * isMoving;
		CamPos.y = characterHeight + walk;
		
		if (isMoving) {
			differentialSign = (CamPos.y - oldCameraHeitgh) >= 0 ? 1.0f : -1.0f;

			if (oldDifferentialSign != differentialSign && CamPos.y <= characterHeight + 0.025f) {
				se.playSoundEffect(soundEffectIndex);
				soundEffectIndex += 1;
				if (soundEffectIndex == 2)
					soundEffectIndex = 0;
			}
			
		}

		//CAMERA VIEW MATRIX
		GlobalUniformBufferObject guboObj{};
		glm::mat4 CamMat = glm::translate(glm::transpose(glm::mat4(CamDir)), -CamPos);
		guboObj.view = CamMat;

		//CAMERA PROJECTION MATRIX 
		glm::mat4 out = glm::perspective(glm::radians(90.0f), aspect_ratio, 0.1f, 100.0f);
		out[1][1] *= -1;
		guboObj.proj = out;
		
		// SKYBOX
		UniformBufferObjectSkybox uboSky{};
		uboSky.mMat = glm::mat4(1.0f);
		uboSky.nMat = glm::mat4(1.0f);
		uboSky.mvpMat = out * glm::mat4(glm::mat3(CamMat)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.15f, 0.0f));

		// Statue
		if (statues.size() == 4) {
			// Venus
			statues[0].uboStatue.model = glm::translate(glm::mat4(1.0f), glm::vec3(-7.0f, 0.1f, 4.15f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(0.6f));

			// Helios
			statues[1].uboStatue.model = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, 0.5f + 1.1f * sin(2.5 * modTime), -20.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(80.0f), glm::vec3(1, 0, 0)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));

			//TV stand
			statues[2].uboStatue.model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(150.0f), glm::vec3(0, 1, 0)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));

			// Flamingo
			statues[3].uboStatue.model =
				glm::rotate(glm::mat4(1.0f), glm::radians(10.0f), glm::vec3(0, 0, 1)) *
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.5f + 0.5f * sin(3 * modTime + 1), 5.5f)) *
				glm::rotate(glm::mat4(1.0f), glm::radians(360.0f * modTime), glm::vec3(0, 1, 0));
		}


		//MAPPING - Here is where you actually update your uniforms
		void* data;

		//Museum and paintings
		vkMapMemory(device, DS1.uniformBuffersMemory[0][currentImage], 0,
			sizeof(ubo_museum), 0, &data);
		memcpy(data, &ubo_museum, sizeof(ubo_museum));
		vkUnmapMemory(device, DS1.uniformBuffersMemory[0][currentImage]);


		// Mountain
		vkMapMemory(device, mountainDS.uniformBuffersMemory[0][currentImage], 0,
			sizeof(mountainUbo), 0, &data);
		memcpy(data, &mountainUbo, sizeof(mountainUbo));
		vkUnmapMemory(device, mountainDS.uniformBuffersMemory[0][currentImage]);

		//STATUES
		for each(Statue s in statues)
		{
			vkMapMemory(device, s.DSS.uniformBuffersMemory[0][currentImage], 0,
				sizeof(s.uboStatue.model), 0, &data);
			memcpy(data, &s.uboStatue.model, sizeof(s.uboStatue.model));
			vkUnmapMemory(device, s.DSS.uniformBuffersMemory[0][currentImage]);
		}

		//CARD
		vkMapMemory(device, DSC.uniformBuffersMemory[0][currentImage], 0,
			sizeof(ubo_UI), 0, &data);
		memcpy(data, &ubo_UI, sizeof(ubo_UI));
		vkUnmapMemory(device, DSC.uniformBuffersMemory[0][currentImage]);


		// SkyBox uniforms
		vkMapMemory(device, SkyBoxUniformBuffersMemory[currentImage], 0,
			sizeof(uboSky), 0, &data);
		memcpy(data, &uboSky, sizeof(uboSky));
		vkUnmapMemory(device, SkyBoxUniformBuffersMemory[currentImage]);

		//GLOBAL
		vkMapMemory(device, DSGlobal.uniformBuffersMemory[0][currentImage], 0,
			sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DSGlobal.uniformBuffersMemory[0][currentImage]);

		vkMapMemory(device, DSGlobalModels.uniformBuffersMemory[0][currentImage], 0,
			sizeof(guboObj), 0, &data);
		memcpy(data, &guboObj, sizeof(GlobalUniformBufferObject));
		vkUnmapMemory(device, DSGlobalModels.uniformBuffersMemory[0][currentImage]);

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


	// Skybox aux functions
	void createCubicTextureImage(const char* const FName[6], Texture& TD) {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels[6];

		for (int i = 0; i < 6; i++) {
			pixels[i] = stbi_load(FName[i], &texWidth, &texHeight,
				&texChannels, STBI_rgb_alpha);
			if (!pixels[i]) {
				std::cout << FName[i]<< "\n";
				throw std::runtime_error("failed to load texture image!");
			}
			std::cout << FName[i] << " -> size: " << texWidth
				<< "x" << texHeight << ", ch: " << texChannels << "\n";
		}

		VkDeviceSize imageSize = texWidth * texHeight * 4;
		VkDeviceSize totalImageSize = texWidth * texHeight * 4 * 6;
		TD.mipLevels = static_cast<uint32_t>(std::floor(
			std::log2(std::max(texWidth, texHeight)))) + 1;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		createBuffer(totalImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, totalImageSize, 0, &data);
		for (int i = 0; i < 6; i++) {
			memcpy(static_cast<char*>(data) + imageSize * i, pixels[i], static_cast<size_t>(imageSize));
		}
		vkUnmapMemory(device, stagingBufferMemory);

		for (int i = 0; i < 6; i++) {
			stbi_image_free(pixels[i]);
		}
		createSkyBoxImage(texWidth, texHeight, TD.mipLevels, TD.textureImage,
			TD.textureImageMemory);

		transitionImageLayout(TD.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, TD.mipLevels, 6);
		copyBufferToImage(stagingBuffer, TD.textureImage,
			static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 6);

		generateMipmaps(TD.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
			texWidth, texHeight, TD.mipLevels, 6);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void createSkyBoxImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkImage& image,
		VkDeviceMemory& imageMemory) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 6;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VkResult result = vkCreateImage(device, &imageInfo, nullptr, &image);
		if (result != VK_SUCCESS) {
			PrintVkError(result);
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	void createSkyBoxImageView(Texture& TD) {
		TD.textureImageView = createImageView(TD.textureImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT,
			TD.mipLevels,
			VK_IMAGE_VIEW_TYPE_CUBE, 6);
	}

	void createSkyBoxDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(),
			skyBoxDSL.descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
		allocInfo.pSetLayouts = layouts.data();

		SkyBoxDescriptorSets.resize(swapChainImages.size());

		VkResult result = vkAllocateDescriptorSets(device, &allocInfo,
			SkyBoxDescriptorSets.data());
		if (result != VK_SUCCESS) {
			PrintVkError(result);
			throw std::runtime_error("failed to allocate Skybox descriptor sets!");
		}

		for (size_t k = 0; k < swapChainImages.size(); k++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = SkyBoxUniformBuffers[k];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = skyBoxTexture.textureImageView;
			imageInfo.sampler = skyBoxTexture.textureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = SkyBoxDescriptorSets[k];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = SkyBoxDescriptorSets[k];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType =
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device,
				static_cast<uint32_t>(descriptorWrites.size()),
				descriptorWrites.data(), 0, nullptr);
		}
	}

	void loadSkyBox() {
		skyBox.init(this, SkyBoxToLoad.ObjFile);

		createCubicTextureImage(SkyBoxToLoad.TextureFile, skyBoxTexture);
		createSkyBoxImageView(skyBoxTexture);
		skyBoxTexture.BP = this;
		skyBoxTexture.createTextureSampler();

		// Skybox createUniformBuffers 
		VkDeviceSize bufferSize = sizeof(UniformBufferObjectSkybox);
		SkyBoxUniformBuffers.resize(swapChainImages.size());
		SkyBoxUniformBuffersMemory.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				SkyBoxUniformBuffers[i], SkyBoxUniformBuffersMemory[i]);
		}

		// Skybox descriptor sets
		createSkyBoxDescriptorSets();
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