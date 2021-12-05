#include "Demo.h"



Demo::Demo() {

}


Demo::~Demo() {
}



void Demo::Init() {
	BuildShaders();
	BuildDepthMap();
	BuildTexturedCube();
	BuildTexturedPlane();

}

void Demo::DeInit() {
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &planeEBO);
	glDeleteBuffers(1, &depthMapFBO);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Demo::ProcessInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void Demo::Update(double deltaTime) {
}

void Demo::Render() {
	//Shadow mapping
	// atau shadowing projection adalah proses di mana bayangan ditambahkan ke grafik komputer 3D.

	// prosedur
	// Generate a depth map
	// Rendering shadows
			
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	
	// Step 1 Render depth of scene to texture
	// ----------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	UseShader(this->depthmapShader);
	glUniformMatrix4fv(glGetUniformLocation(this->depthmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glViewport(0, 0, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawTexturedCube(this->depthmapShader);
	DrawTexturedPlane(this->depthmapShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	// Step 2 Render scene normally using generated depth map
	// ------------------------------------------------------
	// prosedur render shadow
	// cek setiap fragment apakah terdapat shadow atau tidak
	// apabila terdapat shadow, maka lakukan multiply(diffuse + specular) by 0.
	glViewport(0, 0, this->screenWidth, this->screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Pass perspective projection matrix
	UseShader(this->shadowmapShader);
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	// LookAt camera (position, target/direction, up)
	glm::vec3 cameraPos = glm::vec3(0, 1, 2); //0, 1, 2
	glm::vec3 cameraFront = glm::vec3(0, 0, 0);
	glm::mat4 view = glm::lookAt(cameraPos, cameraFront, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	
	// Setting Light Attributes
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "lightPos"), -2.0f, 4.0f, -1.0f);

	// Configure Shaders
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "shadowMap"), 1);

	// Render floor
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedPlane(this->shadowmapShader);
	
	// Render cube
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedCube(this->shadowmapShader);

	glDisable(GL_DEPTH_TEST);
}



void Demo::BuildTexturedCube()
{
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &cube_texture);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("crate.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);


	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		//=============== Section Alas meja ===============
		// front
		-0.5, 0.3, 0.5, 0, 0, 0.0f,  0.0f,  1.0f,
		0.5, 0.3, 0.5, 1, 0, 0.0f,  0.0f,  1.0f,
		0.5,  0.5, 0.5, 1, 1, 0.0f,  0.0f,  1.0f,
		-0.5,  0.5, 0.5, 0, 1, 0.0f,  0.0f,  1.0f,

		// right
		0.5,  0.5,  0.5, 0, 0, 1.0f,  0.0f,  0.0f,
		0.5,  0.5, -0.5, 1, 0, 1.0f,  0.0f,  0.0f,
		0.5, 0.3, -0.5, 1, 1, 1.0f,  0.0f,  0.0f,
		0.5, 0.3,  0.5, 0, 1, 1.0f,  0.0f,  0.0f,

		// back
		-0.5, 0.3, -0.5, 0, 0,  0.0f,  0.0f,  -1.0f,
		0.5,  0.3, -0.5, 1, 0,  0.0f,  0.0f,  -1.0f,
		0.5,   0.5, -0.5, 1, 1,  0.0f,  0.0f,  -1.0f,
		-0.5,  0.5, -0.5, 0, 1,  0.0f,  0.0f,  -1.0f,

		// left
		-0.5, 0.3, -0.5, 0, 0, -1.0f,  0.0f,  0.0f,
		-0.5, 0.3,  0.5, 1, 0, -1.0f,  0.0f,  0.0f,
		-0.5,  0.5,  0.5, 1, 1, -1.0f,  0.0f,  0.0f,
		-0.5,  0.5, -0.5, 0, 1, -1.0f,  0.0f,  0.0f,

		// upper
		0.5, 0.5,  0.5, 0, 0,  0.0f,  1.0f,  0.0f,
		-0.5, 0.5,  0.5, 1, 0,  0.0f,  1.0f,  0.0f,
		-0.5, 0.5, -0.5, 1, 1,  0.0f,  1.0f,  0.0f,
		0.5, 0.5, -0.5, 0, 1,  0.0f,  1.0f,  0.0f,

		// bottom
		-0.5, 0.3, -0.5, 0, 0, 0.0f,  -1.0f,  0.0f,
		0.5, 0.3, -0.5, 1, 0, 0.0f,  -1.0f,  0.0f,
		0.5, 0.3,  0.5, 1, 1, 0.0f,  -1.0f,  0.0f,
		-0.5, 0.3,  0.5, 0, 1, 0.0f,  -1.0f,  0.0f,

		// =============== Section Kaki kiri depan ===============
		// front
		-0.5, -0.5, 0.5, 0, 0,  0.0f,  0.0f,  1.0f,
		-0.4, -0.5, 0.5, 1, 0,  0.0f,  0.0f,  1.0f,
		-0.4,  0.4, 0.5, 1, 1,  0.0f,  0.0f,  1.0f,
		-0.5,  0.4, 0.5, 0, 1,  0.0f,  0.0f,  1.0f,

		// right
		-0.4,  0.4,  0.5, 0, 0, 1.0f,  0.0f,  0.0f,
		-0.4,  0.4, 0.4, 1, 0, 1.0f,  0.0f,  0.0f,
		-0.4, -0.5, 0.4, 1, 1, 1.0f,  0.0f,  0.0f,
		-0.4, -0.5,  0.5, 0, 1, 1.0f,  0.0f,  0.0f,

		// back
		-0.5, -0.5, 0.4, 0, 0,  0.0f,  0.0f,  -1.0f,
		-0.4,  -0.5, 0.4, 1, 0,  0.0f,  0.0f,  -1.0f,
		-0.4,   0.4, 0.4, 1, 1,  0.0f,  0.0f,  -1.0f,
		-0.5,  0.4, 0.4, 0, 1,  0.0f,  0.0f,  -1.0f,

		// left
		-0.5, -0.5, 0.4, 0, 0, -1.0f,  0.0f,  0.0f,
		-0.5, -0.5,  0.5, 1, 0, -1.0f,  0.0f,  0.0f,
		-0.5,  0.4,  0.5, 1, 1, -1.0f,  0.0f,  0.0f,
		-0.5,  0.4, 0.4, 0, 1, -1.0f,  0.0f,  0.0f,

		// upper
		-0.4, 0.4,  0.5, 0, 0,  0.0f,  1.0f,  0.0f,
		-0.5, 0.4,  0.5, 1, 0,  0.0f,  1.0f,  0.0f,
		-0.5, 0.4, 0.4, 1, 1,  0.0f,  1.0f,  0.0f,
		-0.4, 0.4, 0.4, 0, 1,  0.0f,  1.0f,  0.0f,

		// bottom
		-0.5, -0.5, 0.4, 0, 0, 0.0f,  -1.0f,  0.0f,
		-0.4, -0.5, 0.4, 1, 0, 0.0f,  -1.0f,  0.0f,
		-0.4, -0.5,  0.5, 1, 1, 0.0f,  -1.0f,  0.0f,
		-0.5, -0.5,  0.5, 0, 1, 0.0f,  -1.0f,  0.0f,

		// =============== Section kanan depan =============== 
		// front
		0.4, -0.5, 0.5, 0, 0,  0.0f,  0.0f,  1.0f,
		0.5, -0.5, 0.5, 1, 0,  0.0f,  0.0f,  1.0f,
		0.5,  0.4, 0.5, 1, 1,  0.0f,  0.0f,  1.0f,
		0.4,  0.4, 0.5, 0, 1,  0.0f,  0.0f,  1.0f,

		// right
		0.5,  0.4,  0.5, 0, 0, 1.0f,  0.0f,  0.0f,
		0.5,  0.4, 0.4, 1, 0, 1.0f,  0.0f,  0.0f,
		0.5, -0.5, 0.4, 1, 1, 1.0f,  0.0f,  0.0f,
		0.5, -0.5,  0.5, 0, 1, 1.0f,  0.0f,  0.0f,

		// back
		0.4, -0.5, 0.4, 0, 0,  0.0f,  0.0f,  -1.0f,
		0.5,  -0.5, 0.4, 1, 0,  0.0f,  0.0f,  -1.0f,
		0.5,   0.4, 0.4, 1, 1,  0.0f,  0.0f,  -1.0f,
		0.4,  0.4, 0.4, 0, 1,  0.0f,  0.0f,  -1.0f,

		// left
		0.4, -0.5, 0.4, 0, 0, -1.0f,  0.0f,  0.0f,
		0.4, -0.5,  0.5, 1, 0, -1.0f,  0.0f,  0.0f,
		0.4,  0.4,  0.5, 1, 1, -1.0f,  0.0f,  0.0f,
		0.4,  0.4, 0.4, 0, 1, -1.0f,  0.0f,  0.0f,

		// upper
		0.5, 0.4,  0.5, 0, 0, 0.0f, 1.0f, 0.0f,
		0.4, 0.4,  0.5, 1, 0, 0.0f, 1.0f, 0.0f,
		0.5, 0.4, 0.4, 1, 1, 0.0f, 1.0f, 0.0f,
		0.5, 0.4, 0.4, 0, 1, 0.0f, 1.0f, 0.0f,

		// bottom
		0.4, -0.5, 0.4, 0, 0, 0.0f, -1.0f, 0.0f,
		0.5, -0.5, 0.4, 1, 0, 0.0f, -1.0f, 0.0f,
		0.5, -0.5,  0.5, 1, 1, 0.0f, -1.0f, 0.0f,
		0.4, -0.5,  0.5, 0, 1, 0.0f, -1.0f, 0.0f,

		// =============== Section kanan belakang ===============
		// front
		0.4, -0.5, -0.4, 0, 0, 0.0f, 0.0f, 1.0f,
		0.5, -0.5, -0.4, 1, 0, 0.0f, 0.0f, 1.0f,
		0.5, 0.4, -0.4, 1, 1, 0.0f, 0.0f, 1.0f,
		0.4, 0.4, -0.4, 0, 1, 0.0f, 0.0f, 1.0f,

		// right
		0.5, 0.4, -0.4, 0, 0, 1.0f, 0.0f, 0.0f,
		0.5, 0.4, -0.5, 1, 0, 1.0f, 0.0f, 0.0f,
		0.5, -0.5, -0.5, 1, 1, 1.0f, 0.0f, 0.0f,
		0.5, -0.5, -0.4, 0, 1, 1.0f, 0.0f, 0.0f,

		// back
		0.4, -0.5, -0.5, 0, 0, 0.0f, 0.0f, -1.0f,
		0.5, -0.5, -0.5, 1, 0, 0.0f, 0.0f, -1.0f,
		0.5, 0.4, -0.5, 1, 1, 0.0f, 0.0f, -1.0f,
		0.4, 0.4, -0.5, 0, 1, 0.0f, 0.0f, -1.0f,

		// left
		0.4, -0.5, -0.5, 0, 0, -1.0f, 0.0f, 0.0f,
		0.4, -0.5, -0.4, 1, 0, -1.0f, 0.0f, 0.0f,
		0.4, 0.4, -0.4, 1, 1, -1.0f, 0.0f, 0.0f,
		0.4, 0.4, -0.5, 0, 1, -1.0f, 0.0f, 0.0f,

		// upper
		0.5, 0.4, -0.4, 0, 0, 0.0f, 1.0f, 0.0f,
		0.4, 0.4, -0.4, 1, 0, 0.0f, 1.0f, 0.0f,
		0.5, 0.4, -0.5, 1, 1, 0.0f, 1.0f, 0.0f,
		0.5, 0.4, -0.5, 0, 1, 0.0f, 1.0f, 0.0f,

		// bottom
		0.4, -0.5, -0.5, 0, 0, 0.0f, -1.0f, 0.0f,
		0.5, -0.5, -0.5, 1, 0, 0.0f, -1.0f, 0.0f,
		0.5, -0.5, -0.4, 1, 1, 0.0f, -1.0f, 0.0f,
		0.4, -0.5, -0.4, 0, 1, 0.0f, -1.0f, 0.0f,

		// =============== Section Kaki kiri belakang ===============
		// front
		-0.5, -0.5, -0.4, 0, 0, 0.0f, 0.0f, 1.0f,
		-0.4, -0.5, -0.4, 1, 0, 0.0f, 0.0f, 1.0f,
		-0.4, 0.4, -0.4, 1, 1, 0.0f, 0.0f, 1.0f,
		-0.5, 0.4, -0.4, 0, 1, 0.0f, 0.0f, 1.0f,

		// right
		-0.4, 0.4, -0.4, 0, 0, 1.0f, 0.0f, 0.0f,
		-0.4, 0.4, -0.5, 1, 0, 1.0f, 0.0f, 0.0f,
		-0.4, -0.5, -0.5, 1, 1, 1.0f, 0.0f, 0.0f,
		-0.4, -0.5, -0.4, 0, 1, 1.0f, 0.0f, 0.0f,

		// back
		-0.5, -0.5, -0.5, 0, 0, 0.0f, 0.0f, -1.0f,
		-0.4, -0.5, -0.5, 1, 0, 0.0f, 0.0f, -1.0f,
		-0.4, 0.4, -0.5, 1, 1, 0.0f, 0.0f, -1.0f,
		-0.5, 0.4, -0.5, 0, 1, 0.0f, 0.0f, -1.0f,

		// left
		-0.5, -0.5, -0.5, 0, 0, -1.0f, 0.0f, 0.0f,
		-0.5, -0.5, -0.4, 1, 0, -1.0f, 0.0f, 0.0f,
		-0.5, 0.4, -0.4, 1, 1, -1.0f, 0.0f, 0.0f,
		-0.5, 0.4, -0.5, 0, 1, -1.0f, 0.0f, 0.0f,

		// upper
		-0.4, 0.4, -0.4, 0, 0, 0.0f, 1.0f, 0.0f,
		-0.5, 0.4, -0.4, 1, 0, 0.0f, 1.0f, 0.0f,
		-0.5, 0.4, -0.5, 1, 1, 0.0f, 1.0f, 0.0f,
		-0.4, 0.4, -0.5, 0, 1, 0.0f, 1.0f, 0.0f,

		// bottom
		-0.5, -0.5, -0.5, 0, 0, 0.0f, -1.0f, 0.0f,
		-0.4, -0.5, -0.5, 1, 0, 0.0f, -1.0f, 0.0f,
		-0.4, -0.5, -0.4, 1, 1, 0.0f, -1.0f, 0.0f,
		-0.5, -0.5, -0.4, 0, 1, 0.0f, -1.0f, 0.0f,

		//============= section sekat depan ===============
		// front
		-0.4, 0.1, 0.5, 0, 0, 0.0f, 0.0f, 1.0f,
		0.4, 0.1, 0.5, 1, 0, 0.0f, 0.0f, 1.0f,
		0.4, 0.2, 0.5, 1, 1, 0.0f, 0.0f, 1.0f,
		-0.4, 0.2, 0.5, 0, 1, 0.0f, 0.0f, 1.0f,

		// right
		0.4, 0.2, 0.5, 0, 0, 1.0f, 0.0f, 0.0f,
		0.4, 0.2, 0.4, 1, 0, 1.0f, 0.0f, 0.0f,
		0.4, 0.1, 0.4, 1, 1, 1.0f, 0.0f, 0.0f,
		0.4, 0.1, 0.5, 0, 1, 1.0f, 0.0f, 0.0f,

		// back
		-0.4, 0.1, 0.4, 0, 0, 0.0f, 0.0f, -1.0f,
		0.4, 0.1, 0.4, 1, 0, 0.0f, 0.0f, -1.0f,
		0.4, 0.2, 0.4, 1, 1, 0.0f, 0.0f, -1.0f,
		-0.4, 0.2, 0.4, 0, 1, 0.0f, 0.0f, -1.0f,

		// left
		-0.4, 0.1, 0.4, 0, 0, -1.0f, 0.0f, 0.0f,
		-0.4, 0.1, 0.5, 1, 0, -1.0f, 0.0f, 0.0f,
		-0.4, 0.2, 0.5, 1, 1, -1.0f, 0.0f, 0.0f,
		-0.4, 0.2, 0.4, 0, 1, -1.0f, 0.0f, 0.0f,

		// upper
		0.4, 0.2, 0.5, 0, 0, 0.0f, 1.0f, 0.0f,
		-0.4, 0.2, 0.5, 1, 0, 0.0f, 1.0f, 0.0f,
		-0.4, 0.2, 0.4, 1, 1, 0.0f, 1.0f, 0.0f,
		0.4, 0.2, 0.4, 0, 1, 0.0f, 1.0f, 0.0f,

		// bottom
		-0.4, 0.1, 0.4, 0, 0, 0.0f, -1.0f, 0.0f,
		0.4, 0.1, 0.4, 1, 0, 0.0f, -1.0f, 0.0f,
		0.4, 0.1, 0.5, 1, 1, 0.0f, -1.0f, 0.0f,
		-0.4, 0.1, 0.5, 0, 1, 0.0f, -1.0f, 0.0f,


		//============= section sekat belakang ===============
		// front
		-0.4, 0.1, -0.5, 0, 0, 0.0f, 0.0f, 1.0f,
		0.4, 0.1, -0.5, 1, 0, 0.0f, 0.0f, 1.0f,
		0.4, 0.2, -0.5, 1, 1, 0.0f, 0.0f, 1.0f,
		-0.4, 0.2, -0.5, 0, 1, 0.0f, 0.0f, 1.0f,

		// right
		0.4, 0.2, -0.5, 0, 0, 1.0f, 0.0f, 0.0f,
		0.4, 0.2, -0.4, 1, 0, 1.0f, 0.0f, 0.0f,
		0.4, 0.1, -0.4, 1, 1, 1.0f, 0.0f, 0.0f,
		0.4, 0.1, -0.5, 0, 1, 1.0f, 0.0f, 0.0f,

		// back
		-0.4, 0.1, -0.4, 0, 0, 0.0f, 0.0f, -1.0f,
		0.4, 0.1, -0.4, 1, 0, 0.0f, 0.0f, -1.0f,
		0.4, 0.2, -0.4, 1, 1, 0.0f, 0.0f, -1.0f,
		-0.4, 0.2, -0.4, 0, 1, 0.0f, 0.0f, -1.0f,

		// left
		-0.4, 0.1, -0.4, 0, 0, -1.0f, 0.0f, 0.0f,
		-0.4, 0.1, -0.5, 1, 0, -1.0f, 0.0f, 0.0f,
		-0.4, 0.2, -0.5, 1, 1, -1.0f, 0.0f, 0.0f,
		-0.4, 0.2, -0.4, 0, 1, -1.0f, 0.0f, 0.0f,

		// upper
		0.4, 0.2, -0.5, 0, 0, 0.0f, 1.0f, 0.0f,
		-0.4, 0.2, -0.5, 1, 0, 0.0f, 1.0f, 0.0f,
		-0.4, 0.2, -0.4, 1, 1, 0.0f, 1.0f, 0.0f,
		0.4, 0.2, -0.4, 0, 1, 0.0f, 1.0f, 0.0f,

		// bottom
		-0.4, 0.1, -0.4, 0, 0, 0.0f, -1.0f, 0.0f,
		0.4, 0.1, -0.4, 1, 0, 0.0f, -1.0f, 0.0f,
		0.4, 0.1, -0.5, 1, 1, 0.0f, -1.0f, 0.0f,
		-0.4, 0.1, -0.5, 0, 1, 0.0f, -1.0f, 0.0f,

		//============= section sekat kiri ===============
		// front
		-0.5, 0.1, 0.4, 0, 0, 0.0f, 0.0f, 1.0f,
		-0.4, 0.1, 0.4, 1, 0, 0.0f, 0.0f, 1.0f,
		-0.4, 0.2, 0.4, 1, 1, 0.0f, 0.0f, 1.0f,
		-0.5, 0.2, 0.4, 0, 1, 0.0f, 0.0f, 1.0f,

		// right
		-0.4, 0.2, 0.4, 0, 0, 1.0f, 0.0f, 0.0f,
		-0.4, 0.2, -0.4, 1, 0, 1.0f, 0.0f, 0.0f,
		-0.4, 0.1, -0.4, 1, 1, 1.0f, 0.0f, 0.0f,
		-0.4, 0.1, 0.4, 0, 1, 1.0f, 0.0f, 0.0f,

		// back
		-0.5, 0.1, -0.4, 0, 0, 0.0f, 0.0f, -1.0f,
		-0.4, 0.1, -0.4, 1, 0, 0.0f, 0.0f, -1.0f,
		-0.4, 0.2, -0.4, 1, 1, 0.0f, 0.0f, -1.0f,
		-0.5, 0.2, -0.4, 0, 1, 0.0f, 0.0f, -1.0f,

		// left
		-0.5, 0.1, -0.4, 0, 0, -1.0f, 0.0f, 0.0f,
		-0.5, 0.1, 0.4, 1, 0, -1.0f, 0.0f, 0.0f,
		-0.5, 0.2, 0.4, 1, 1, -1.0f, 0.0f, 0.0f,
		-0.5, 0.2, -0.4, 0, 1, -1.0f, 0.0f, 0.0f,

		// upper
		-0.4, 0.2, 0.4, 0, 0, 0.0f, 1.0f, 0.0f,
		-0.5, 0.2, 0.4, 1, 0, 0.0f, 1.0f, 0.0f,
		-0.5, 0.2, -0.4, 1, 1, 0.0f, 1.0f, 0.0f,
		-0.4, 0.2, -0.4, 0, 1, 0.0f, 1.0f, 0.0f,

		// bottom
		-0.5, 0.1, -0.4, 0, 0, 0.0f, -1.0f, 0.0f,
		-0.4, 0.1, -0.4, 1, 0, 0.0f, -1.0f, 0.0f,
		-0.4, 0.1, 0.4, 1, 1, 0.0f, -1.0f, 0.0f,
		-0.5, 0.1, 0.4, 0, 1, 0.0f, -1.0f, 0.0f,

		//============= section sekat kanan ===============
		// front
		0.5, 0.1, 0.4, 0, 0, 0.0f, 0.0f, 1.0f,
		0.4, 0.1, 0.4, 1, 0, 0.0f, 0.0f, 1.0f,
		0.4, 0.2, 0.4, 1, 1, 0.0f, 0.0f, 1.0f,
		0.5, 0.2, 0.4, 0, 1, 0.0f, 0.0f, 1.0f,

		// right
		0.4, 0.2, 0.4, 0, 0, 1.0f, 0.0f, 0.0f,
		0.4, 0.2, -0.4, 1, 0, 1.0f, 0.0f, 0.0f,
		0.4, 0.1, -0.4, 1, 1, 1.0f, 0.0f, 0.0f,
		0.4, 0.1, 0.4, 0, 1, 1.0f, 0.0f, 0.0f,

		// back
		0.5, 0.1, -0.4, 0, 0, 0.0f, 0.0f, -1.0f,
		0.4, 0.1, -0.4, 1, 0, 0.0f, 0.0f, -1.0f,
		0.4, 0.2, -0.4, 1, 1, 0.0f, 0.0f, -1.0f,
		0.5, 0.2, -0.4, 0, 1, 0.0f, 0.0f, -1.0f,

		// left
		0.5, 0.1, -0.4, 0, 0, -1.0f, 0.0f, 0.0f,
		0.5, 0.1, 0.4, 1, 0, -1.0f, 0.0f, 0.0f,
		0.5, 0.2, 0.4, 1, 1, -1.0f, 0.0f, 0.0f,
		0.5, 0.2, -0.4, 0, 1, -1.0f, 0.0f, 0.0f,

		// upper
		0.4, 0.2, 0.4, 0, 0, 0.0f, 1.0f, 0.0f,
		0.5, 0.2, 0.4, 1, 0, 0.0f, 1.0f, 0.0f,
		0.5, 0.2, -0.4, 1, 1, 0.0f, 1.0f, 0.0f,
		0.4, 0.2, -0.4, 0, 1, 0.0f, 1.0f, 0.0f,

		// bottom
		0.5, 0.1, -0.4, 0, 0, 0.0f, -1.0f, 0.0f,
		0.4, 0.1, -0.4, 1, 0, 0.0f, -1.0f, 0.0f,
		0.4, 0.1, 0.4, 1, 1, 0.0f, -1.0f, 0.0f,
		0.5, 0.1, 0.4, 0, 1, 0.0f, -1.0f, 0.0f,
	};

	unsigned int indices[] = {
		//Alas Meja
		0,  1,  2,  0,  2,  3,   // front
		4,  5,  6,  4,  6,  7,   // right
		8,  9,  10, 8,  10, 11,  // back
		12, 14, 13, 12, 15, 14,  // left
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22,   // bottom

		//Kaki kiri depan
		24, 25, 26, 24, 26, 27,
		28, 29, 30, 28, 30, 31,
		32, 33, 34, 32, 34, 35,
		36, 38, 37, 36, 39, 38,
		40, 42, 41, 40, 43, 42,
		44, 46, 45, 44, 47, 46,

		//Kaki kanan depan
		48, 49, 50, 48, 50, 51,
		52, 53, 54, 52, 54, 55,
		56, 57, 58, 56, 58, 59,
		60, 62, 61, 60, 63, 62,
		64, 66, 65, 64, 67, 66,
		68, 70, 69, 68, 71, 70,

		//Kaki kanan belakang
		72, 73, 74, 72, 74, 75,
		76, 77, 78, 76, 78, 79,
		80, 81, 82, 80, 82, 83,
		84, 86, 85, 84, 87, 86,
		88, 90, 89, 88, 91, 90,
		92, 94, 93, 92, 95, 94,

		//Kaki kiri belakang
		96, 97, 98, 96, 98, 99,
		100, 101, 102, 100, 102,
		103, 104, 105, 106, 104,
		106, 107, 108, 110, 109,
		108, 111, 110, 112, 114,
		113, 112, 115, 114, 116,
		118, 117, 116, 119, 118,

		//sekat depan
		120, 121, 122, 120, 122, 123,
		124, 125, 126, 124, 126, 127,
		128, 129, 130, 128, 130, 131,
		132, 134, 133, 132, 135, 134,
		136, 138, 137, 136, 139, 138,
		140, 142, 141, 140, 143, 142,

		//sekat belakang
		144, 145, 146, 144, 146, 147,
		148, 149, 150, 148, 150, 151,
		152, 153, 154, 152, 154, 155,
		156, 158, 157, 156, 159, 158,
		160, 162, 161, 160, 163, 162,
		164, 166, 165, 164, 167, 166,

		//sekat kiri
		168, 169, 170, 168, 170, 171,
		172, 173, 174, 172, 174, 175,
		176, 177, 178, 176, 178, 179,
		180, 182, 181, 180, 183, 182,
		184, 186, 185, 184, 187, 186,
		188, 190, 189, 188, 191, 190,

		//sekat kanan
		192, 193, 194, 192, 194, 195,
		196, 197, 198, 196, 198, 199,
		200, 201, 202, 200, 202, 203,
		204, 206, 205, 204, 207, 206,
		208, 210, 209, 208, 211, 210,
		212, 214, 213, 212, 215, 214,
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Demo::BuildTexturedPlane()
{
	// Load and create a texture 
	glGenTextures(1, &plane_texture);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("wood.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	// Build geometry
	GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		-25.0f,	-0.5f, -25.0f,  0,  0, 0.0f,  1.0f,  0.0f,
		25.0f,	-0.5f, -25.0f, 25,  0, 0.0f,  1.0f,  0.0f,
		25.0f,	-0.5f,  25.0f, 25, 25, 0.0f,  1.0f,  0.0f,
		-25.0f,	-0.5f,  25.0f,  0, 25, 0.0f,  1.0f,  0.0f,
	};

	GLuint indices[] = { 0,  2,  1,  0,  3,  2 };

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glGenBuffers(1, &planeEBO);

	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO
}

void Demo::DrawTexturedCube(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 0.5f, 0));

	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 1000, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedPlane(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(planeVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glm::mat4 model;
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildDepthMap() {
	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->SHADOW_WIDTH, this->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Demo::BuildShaders()
{
	// build and compile our shader program
	// ------------------------------------
	shadowmapShader = BuildShader("shadowMapping.vert", "shadowMapping.frag", nullptr);
	depthmapShader = BuildShader("depthMap.vert", "depthMap.frag", nullptr);
}


int main(int argc, char** argv) {
	RenderEngine &app = Demo();
	app.Start("Shadow Mapping Demo", 800, 600, false, false);
}