#include "Renderer.h"
#include <Colider.h>
#include <ObjectSelector.h>

#include <optional>

DebugRenderer::DebugRenderer() : dsv(GameState::engine_path + "shaders/debugVertexShader.glsl", GL_VERTEX_SHADER), dsf(GameState::engine_path + "shaders/debugFragmentShader.glsl", GL_FRAGMENT_SHADER)
{
	dsv.compile();
	dsf.compile();
	dsv.link(dsf);

	vbo.init();
	ebo.init();
	vao.init();

	vao.bind();

	vbo.bind(48 * 6 * sizeof(float), vertices);
	vbo.setVAO(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	vbo.vboEnableVertexAttribArray(0);

	ebo.bind(6 * sizeof(unsigned int), indices);

	setupSceneGrid();
}

void DebugRenderer::setupSceneGrid()
{
    grid_mode = GameState::editor_mode;
    std::cout << "grid setup " << grid_mode << "\n";
    indices_grid.clear();
    vertices_grid.clear();
    vertices_grid.shrink_to_fit();
    indices_grid.shrink_to_fit();

	for (int i = 0; i <= slices; ++i) {
		for (int j = 0; j <= slices; ++j) {
			float x = (float)j;
            float y = 0.0f;
            float z = (float)i;

		    if(grid_mode == 2){	
                x = (float)j;
                y = (float)i;
                z = 0;
            }

            if(grid_mode == 3){

            	//https://devforum.roblox.com/t/how-to-spawn-things-along-the-surface-of-a-sphere/1566555/3
            	//https://stackoverflow.com/questions/9604132/how-to-project-a-point-on-to-a-sphere
            	/*
            	Write the point in a coordinate system centered at the center of the sphere (x0,y0,z0):

				P = (x',y',z') = (x - x0, y - y0, z - z0)

				Compute the length of this vector:

				|P| = sqrt(x'^2 + y'^2 + z'^2)

				Scale the vector so that it has length equal to the radius of the sphere:

				Q = (radius/|P|)*P

				And change back to your original coordinate system to get the projection:

				R = Q + (x0,y0,z0)
            	*/

            	glm::vec3 pos = glm::vec3((x-slices/2) * grid_scale, (y) * grid_scale, (z-slices/2) * grid_scale);

            	//pos.y -= 0.1 * (i > j ? i : j);

                vertices_grid.push_back(pos);
            }

            else if(grid_mode == 2)
                vertices_grid.push_back(glm::vec3((x-slices/2) * grid_scale, (y-slices/2) * grid_scale, (z) * grid_scale));
		}
	}

	for (int i = 0; i < slices; ++i) {
		for (int j = 0; j < slices; ++j) {
			int row1 = j * (static_cast<int>(slices) + 1);
			int row2 = (i + 1) * (static_cast<int>(slices) + 1);

			indices_grid.push_back(glm::vec4(row1 + j, row1 + j + 1, row1 + j + 1, row2 + j + 1));
			indices_grid.push_back(glm::vec4(row2 + j + 1, row2 + j, row2 + j, row1 + j));
		}
	}

	vao_grid.init();
	VBO vbo_grid;
	vbo_grid.init();
	EBO ebo_grid;
	ebo_grid.init();
	
	vao_grid.bind();
	vbo_grid.bind(vertices_grid.size() * sizeof(glm::vec3), &(vertices_grid[0]));
	vbo_grid.setVAO(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	vbo_grid.vboEnableVertexAttribArray(0);
	ebo_grid.bind(indices_grid.size() * sizeof(glm::uvec4), &(indices_grid[0]));

	length = (GLuint)indices_grid.size() * 4;
	glLineWidth(2);
}
void DebugRenderer::renderDebugColider(Window* wind, std::optional<Colider>& collider, std::optional<RigidBody>& body)
{
	if(!collider)
		return;

	glUseProgram(dsv.getProgram());
	auto model = glm::mat4(1.0f);
	model = glm::translate(model, collider->get_transform().getPosition() + glm::vec3{collider->get_size().x / 2, collider->get_size().y / 2, collider->get_size().z / 2} - collider->get_render_shift());
	if(body)
		model *= body->get_quatmat();
	model *= glm::toMat4(collider->get_transform().matrixQuaternion());
	auto vv = collider->get_render_shift();
	model = glm::translate(model, glm::vec3{vv.x/2, vv.y/2, vv.z/2});

	//TODO(darius) its not size, its scale
	model = glm::scale(model, collider->get_size());
	//model = glm::scale(model, glm::vec3{size.x, size.y,size.z});
	//model[3] += glm::vec4{size.x/2 -size.x, size.y/2-size.x,size.z/2-size.x,0};
	dsv.setVec4("objectColor", collider->collider_debug_color);
	dsv.setMat4("model", model);
	vao.bind();
	glDrawArrays(GL_LINE_STRIP, 0, 36);
	glBindVertexArray(0);
}

void DebugRenderer::renderDebugCube(glm::vec3 pos, int r)
{
	glUseProgram(dsv.getProgram());
	auto model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	//TODO(darius) its not size, its scale
	model = glm::scale(model, {r,r,r});
	//model = glm::scale(model, glm::vec3{size.x, size.y,size.z});
	//model[3] += glm::vec4{size.x/2 -size.x, size.y/2-size.x,size.z/2-size.x,0};
	dsv.setVec4("objectColor", {0,1,0,0});
	dsv.setMat4("model", model);
	vao.bind();
	glDrawArrays(GL_LINE_STRIP, 0, 36);
	glBindVertexArray(0); 
}

//TODO(darius) make it instanced. Dont upload each point individually
void DebugRenderer::renderDebugPoint(glm::vec3 a, glm::vec4 color = glm::vec4(0,1,0,0))
{
    if(!debug_render)
        return;
	glUseProgram(dsv.getProgram());
	auto model = glm::mat4(1.0f);
	model = glm::translate(model, a);
	model = glm::scale(model, {0.05,0.05,0.05});
	//TODO(darius) its not size, its scale
	//model = glm::scale(model, {r,r,r});
	//model = glm::scale(model, glm::vec3{size.x, size.y,size.z});
	//model[3] += glm::vec4{size.x/2 -size.x, size.y/2-size.x,size.z/2-size.x,0};
	dsv.setVec4("objectColor", color);
	//dsv.setMat4("model", model);
	flat.DrawRaw(dsv, a, {0.01,0.01,0.01});
	//vao.bind();
	//glDrawArrays(GL_LINE_STRIP, 0, 36);
	//glBindVertexArray(0);
}

void DebugRenderer::renderDebugGrid()
{
    if(GameState::editor_mode != grid_mode)
    {
        setupSceneGrid(); 
    }

	glUseProgram(dsv.getProgram());
	auto model = glm::mat4(1.0f);
	dsv.setVec4("objectColor", {0.4,0.4,0.4,0});
	dsv.setMat4("model", model);
	vao_grid.bind();
	glDrawElements(GL_LINES, static_cast<int>(length), GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}

void DebugRenderer::renderDebugLightSource(std::optional<PointLight>& p)
{
	if(!p)
		return;
	glUseProgram(dsv.getProgram());
	auto model = glm::mat4(1.0f);
	model = glm::translate(model, p->position);
	model = glm::scale(model, glm::vec3{0.2,0.2,0.2});

	dsv.setVec4("objectColor", {p->color.x, p->color.y, p->color.z, 0});
	dsv.setMat4("model", model);
	vao.bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void DebugRenderer::updateCamera(glm::mat4 projection, glm::mat4 view)
{
	glUseProgram(dsv.getProgram());
	dsv.setMat4("projection", projection);
	dsv.setMat4("view", view);
}

void DebugRenderer::renderPoints()
{
	for(auto& i : pointsToRender)
		renderDebugPoint(i.point, i.color);
}

void DebugRenderer::clearPoints()
{
	pointsToRender.clear();
	pointsToRender.shrink_to_fit();
}

Renderer::Renderer(Scene* currScene_in, GameState* instance, Window* wind_in) : currScene(currScene_in), wind(wind_in) {
	//v.compile();
	//sf.compile();
	//sv.link(sf);

	//NOTE(darius) this leak doesnt matter
	shaderLibInstance = new ShaderLibrary();

	//depthv.compile();
	//depthf.compile();
	//depthv.link(depthf);

	//currShaderRoutine = { Shader(sv) };
	FlatMesh flat;
	m.addMesh(flat);
	m.setShader(shaderLibInstance->getDepthShader());
	m.setShaderRoutine(shaderLibInstance->getShaderRoutine());

	/*FlatMesh flat;
	FlatMesh idle;
	FlatMesh runAnimTexture;
	FlatMesh runLeftAnimTexture;
	FlatMesh danceAnimation;
	FlatMesh enemyAnimation;

	flat.setTexture(GameState::engine_path + "textures/hornet", "iddle.png");
	idle.setTexture(GameState::engine_path + "textures/hornet", "iddle.png");
	enemyAnimation.setTexture(GameState::engine_path + "textures", "HollowSpid.png");

	runLeftAnimTexture.setTexture(GameState::engine_path + "textures/hornet", "runLeft.png");
	runAnimTexture.setTexture(GameState::engine_path + "textures/hornet", "runRight.png");
	danceAnimation.setTexture(GameState::engine_path + "textures/hornet", "jump.png");

	auto* obj0 = currScene->AddEmpty(228);
	auto* obj1 = currScene->AddEmpty(229);
	auto* player= currScene->AddEmpty(230);
	auto* obj3 = currScene->AddEmpty(231);
	auto* obj4 = currScene->AddEmpty(232);
	auto* enemy = currScene->AddEmpty(232);

	obj0->hide();
	obj1->hide();
	obj3->hide();
	obj4->hide();

	obj0 ->addModel(std::move(flat), sv, currShaderRoutine);
	obj1->addModel(std::move(runAnimTexture), sv, currShaderRoutine);
	obj3->addModel(std::move(runLeftAnimTexture), sv, currShaderRoutine);
	obj4->addModel(std::move(danceAnimation), sv, currShaderRoutine);
	player->addModel(std::move(idle), sv, currShaderRoutine);
	enemy->addModel(std::move(enemyAnimation), sv, currShaderRoutine);

	auto enemySprite = SpriteAnimation(4, 8, 100);
	enemy->setSpriteAnimation(enemySprite);
	enemy->moveTransform({5,0,0});
	enemy->addCollider();
	enemy->getColider()->set_size({2,2,1});
	enemy->getColider()->set_shift({1,1,0.5});

	//obj->addSpriteAnimation(SpriteAnimation(4,8,500));
	player->addScript(currScene, &routine);
	player->addCollider();
	player->addRigidBody();
	player->getColider()->set_size({1,1,1});
	player->getColider()->set_shift({0.5,0.5,0.5});

	/*for (int i = 0; i < 1; i += 1) {
		auto* op = currScene->createObject("pistol " + std::to_string(i), glm::vec3{ i * 2,i,0 }, glm::vec3{ 1,1,1 }, glm::vec3{ 1,1,3 }, GameState::engine_path + "meshes/pistol/homemade_lasergun_upload.obj",
			sv, currShaderRoutine, currScene, &routine, false, false);
		op->frozeObject();
		op->setMaterial(Material(32));
		//op -> addPointLight(PointLight(glm::vec3{-0.2f, -1.0f, -0.3f}, glm::vec3(1,1,1)));
	}

	//danceAnimation = Animation("../../../meshes/animations/bot/reach.dae", &ourModel);
	*/

	//OpenglWrapper::EnableDepthTest();
	state.EnableDepthTesting();

	framebuffer.AttachMultisampledTexture(wind->getWidth(), wind->getHeight());
	framebuffer.Unbind();

	deferredLightingBuffer.AttachTexture(wind->getWidth(), wind->getHeight(), 3);

	bloomBuffer.AttachTexture(wind->getWidth(), wind->getHeight(), 2);
	//NOTE(darius) make it two buffers, not two color attachments
	pingPongBlurBufferA.AttachTexture(wind->getWidth(), wind->getHeight(), 1);
	pingPongBlurBufferB.AttachTexture(wind->getWidth(), wind->getHeight(), 1);

	bokeBuffer.AttachTexture(wind->getWidth(), wind->getHeight(), 1);
	
	bufferCombination.AttachTexture(wind->getWidth(), wind->getHeight(), 1);

	depthFramebuffer.AttachTexture(wind->getWidth(), wind->getHeight());

	depthTexture.AttachTexture(wind->getWidth(), wind->getHeight());


	//u draw inside of this one
	intermidiateFramebuffer.AttachTexture(wind->getWidth(), wind->getHeight());
	intermidiateFramebuffer.setTaget(GL_DRAW_FRAMEBUFFER);

	OpenglWrapper::UnbindFrameBuffer(GL_FRAMEBUFFER);
}

void Renderer::render(Window* wind)
{
	int display_w, display_h;
	glfwGetFramebufferSize(wind->getWindow(), &display_w, &display_h);
	OpenglWrapper::SetWindow(display_w, display_h);

	shaderLibInstance->checkForShaderReload();

	//depthStage();

	// NOTE(darius) to draw ONLY result of last stage
	// draw it into quad and return from renderer 
	//quad.DrawQuad(depthTexture);
	//return

	//bloomStage();

	EditorIDsStage();
    
	albedoStage();

	//deferredStage();
	//intermidiateFramebuffer.Blit(framebuffer, intermidiateFramebuffer);

	//bokeStage();

	quad.DrawQuad(intermidiateFramebuffer);
}

void Renderer::deferredStage()
{
	shaderLibInstance->stage = ShaderLibrary::STAGE::DEFERRED;

	deferredLightingBuffer.setTaget(GL_FRAMEBUFFER);
	deferredLightingBuffer.Bind();

	OpenglWrapper::ClearScreen(backgroundColor);
	OpenglWrapper::ClearBuffer();
	OpenglWrapper::ClearDepthBuffer();
	OpenglWrapper::EnableDepthTest();

	renderAll(wind);

	deferredLightingBuffer.Unbind();

	intermidiateFramebuffer.Blit(deferredLightingBuffer, intermidiateFramebuffer);
}

void Renderer::EditorIDsStage() 
{
	//OpenglWrapper::BindFrameBuffer(ObjectSelector::pick_fbo);

	shaderLibInstance->stage = ShaderLibrary::STAGE::EDITOR_ID;

	//TODO(darius) move creation of this buffer to Renderer::Renderer()
	FrameBuffer& workerBuff = ObjectSelector::buff;

	workerBuff.setTaget(GL_FRAMEBUFFER);
	workerBuff.Bind();
	

	OpenglWrapper::ClearScreen(backgroundColor);
	OpenglWrapper::ClearBuffer();
	OpenglWrapper::ClearDepthBuffer();
	OpenglWrapper::EnableDepthTest();

	renderScene();

	//TODO(darius) add UI rendering and Guizmos rendering here with id = -1 or something

	workerBuff.Unbind();

	//NOTE(darius) for debug 
	//intermidiateFramebuffer.Blit(workerBuff, intermidiateFramebuffer);
	//quad.DrawQuad(intermidiateFramebuffer);
}

void Renderer::depthStage()
{
	shaderLibInstance->stage = ShaderLibrary::STAGE::DEPTH;
	depthFramebuffer.Bind();

	OpenglWrapper::EnableDepthTest();
	OpenglWrapper::EnableMultisample();
	OpenglWrapper::ClearScreen({ 1,0,0.5 });
	OpenglWrapper::ClearBuffer();


	renderScene();

	depthTexture.setTaget(GL_DRAW_FRAMEBUFFER);
	depthTexture.Bind();
	depthTexture.Blit();
	shaderLibInstance->depthMap = depthTexture.getTexture().get_texture();
}

void Renderer::albedoStage()
{
	shaderLibInstance->stage = ShaderLibrary::STAGE::ALBEDO;
	framebuffer.setTaget(GL_FRAMEBUFFER);
	framebuffer.Bind();

	OpenglWrapper::EnableMultisample();
	OpenglWrapper::ClearScreen(backgroundColor);
	OpenglWrapper::ClearBuffer();
	OpenglWrapper::ClearDepthBuffer();
	OpenglWrapper::EnableDepthTest();

	renderAll(wind);

	intermidiateFramebuffer.Blit(framebuffer, intermidiateFramebuffer);
}

void Renderer::bokeStage()
{
	shaderLibInstance->stage = ShaderLibrary::STAGE::BOKE;
	//NOTE(darius) dont work yet
	float far_plane = 5;
	float close_plane = 0;

	//NOTE(darius) 1) Draw objects that are in focus in separate buffer and then blit on blured?
	//			   2) Blur each objcet using its depth buffer value?

	//bokeBuffer.setTaget(GL_FRAMEBUFFER);
	//bokeBuffer.Bind();

	{
		BinderPointer bptr(pingPongBlurBufferA);
		quad.DrawQuad(framebuffer.getTextureAt(0).get_texture());
	}

	Shader boke = shaderLibInstance->getBokeShader();
	OpenglWrapper::UseProgram(boke.getProgram());

	pingPongBlurBufferA.Bind();

	quad.bindQuadVAO();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(
		GL_TEXTURE_2D, intermidiateFramebuffer.getTextureAt(0).get_texture()
	);

	quad.drawArrays();

	pingPongBlurBufferA.Unbind();

	bokeBuffer.setTaget(GL_FRAMEBUFFER);
	bokeBuffer.Bind();

	OpenglWrapper::ClearDepthBuffer();
	OpenglWrapper::EnableDepthTest();

	//bokeBuffer.Unbind();

	bokeBuffer.Blit(pingPongBlurBufferA, bokeBuffer);

	quad.DrawQuad(bokeBuffer);
}

void Renderer::bloomStage()
{
	shaderLibInstance->stage = ShaderLibrary::STAGE::BLOOM;
	bloomBuffer.Blit(intermidiateFramebuffer, bloomBuffer);
	
	//NOTE(daris) currently works
	//quad.DrawQuad(bloomBuffer.getTextureAt(1).get_texture());
	//return;

	bool horizontal = true, first_iteration = true;
	int amount = 10;

	//NOTE(darius) set HDR main
	{
		BinderPointer bptr(pingPongBlurBufferA);
		quad.DrawQuad(bloomBuffer.getTextureAt(0).get_texture());
	}

	//NOTE(darius) set HDR bright
	{
		BinderPointer bptr(&pingPongBlurBufferB);
		quad.DrawQuad(bloomBuffer.getTextureAt(1).get_texture());
	}

	Shader shaderBlur = shaderLibInstance->getBlurShader();
	OpenglWrapper::UseProgram(shaderBlur.getProgram());

	//TODO(darius) pingpong it here
	pingPongBlurBufferB.Bind();
	for (int i = 0; i < amount; ++i) {
		quad.bindQuadVAO();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(
			GL_TEXTURE_2D, pingPongBlurBufferB.getTextureAt(0).get_texture()
		);

		shaderBlur.setInt("horizontal", i % 2);

		quad.drawArrays();
	}

	pingPongBlurBufferB.Unbind();

	framebuffer.setTaget(GL_FRAMEBUFFER);
	framebuffer.Bind();

	OpenglWrapper::ClearDepthBuffer();
	OpenglWrapper::EnableDepthTest();

	Shader shad = shaderLibInstance->getTextureCombinerShader();
	OpenglWrapper::UseProgram(shad.getProgram());

	quad.bindQuadVAO();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(
		GL_TEXTURE_2D, pingPongBlurBufferA.getTextureAt(0).get_texture()
	);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(
		GL_TEXTURE_2D, pingPongBlurBufferB.getTextureAt(0).get_texture()
	);

	quad.drawArrays();
}

void Renderer::renderDebug(Window* wind) 
{
    if (GameState::cam.cursor_hidden) {
        glm::mat4 projection = GameState::cam.getPerspective(wind->getWidth(), wind->getHeight());
        glm::mat4  view = GameState::cam.getBasicLook();
        dbr.updateCamera(projection, view);
    }

	dbr.renderDebugGrid();

	for (int i = 0; i < currScene->get_objects().size(); ++i) {
		if (currScene->get_objects()[i]) {
			dbr.renderDebugColider(wind, currScene->get_object_at(i)->getColider(),
				currScene->get_object_at(i)->getRigidBody());
			dbr.renderDebugLightSource(currScene->get_objects()[i]->getPointLight());
			currScene->get_objects()[i]->traverseObjects([&dbr = dbr](Object* obj) {
				dbr.renderDebugLightSource(obj->getPointLight());
				});

			//dbr.renderDebugLine(currScene->get_objects()[i]->getTransform().position, {1,1,1});
		}
	}
	//dbr.renderPoints();
}

void Renderer::renderAll(Window* wind)
{
	if (GameState::cam.cursor_hidden) {
		glm::mat4 projection = GameState::cam.getPerspective(wind->getWidth(), wind->getHeight());
		glm::mat4  view = GameState::cam.getBasicLook();
		dbr.updateCamera(projection, view);
	}

	if(dbr.debug_render)
		dbr.renderDebugGrid();

	renderScene();

	if (!dbr.debug_render)
		return;

	for (int i = 0; i < currScene->get_objects().size(); ++i) {
		if (currScene->get_objects()[i]) {
			dbr.renderDebugColider(wind, currScene->get_object_at(i)->getColider(),
				currScene->get_object_at(i)->getRigidBody());
			dbr.renderDebugLightSource(currScene->get_objects()[i]->getPointLight());
			currScene->get_objects()[i]->traverseObjects([&dbr = dbr](Object* obj) {
				dbr.renderDebugLightSource(obj->getPointLight());
				});

			//dbr.renderDebugLine(currScene->get_objects()[i]->getTransform().position, {1,1,1});
		}
	}
	//dbr.renderPoints();
}

void Renderer::renderScene() 
{
	float currentFrame = static_cast<float>(glfwGetTime());
	float deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	//NOTE(darius) 50%+ perfomance boost, but may work weird
	//OpenglWrapper::CullFaces();

	currScene->renderScene();
	Renderer::shaderLibInstance->stage = ShaderLibrary::STAGE::PARTICLES;
	currScene->renderParticles();
	currScene->updateAnimators(deltaTime);//TODO(darius) check if heres bug with time step instead fo time value
	currScene->updateSpriteAnimations(static_cast<float>(glfwGetTime()));
	currScene->updateScene();
}

void Renderer::updateBuffers(Window* wind)
{
    glfwSwapBuffers(wind->getWindow());
	glfwPollEvents();
}

size_t Renderer::getShaderRoutine()
{
    return getShader().getProgram();
}

LightingShaderRoutine& Renderer::getCurrShaderRoutine()
{
    return Renderer::shaderLibInstance->getShaderRoutine();
}

Shader Renderer::getShader()
{
    return Renderer::shaderLibInstance->getCurrShader();
}

Scene* Renderer::getScene()
{
	return currScene;
}

DebugRenderer& Renderer::getDebugRenderer()
{
    return dbr;
}


ShaderLibrary* Renderer::shaderLibInstance = nullptr;

