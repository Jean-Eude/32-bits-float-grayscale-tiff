#include <EngineManager.hpp>

EngineManager::EngineManager() : m_mode(0), gameLoop(120.0f, 0.008f), runTimer() {
    // Mode Raw par défaut
}

void EngineManager::OnReadConfigFile(const char* cfg) {
    readConfigFile(cfg);
    
    for (const auto& a : m_cVars) {
        //std::cout << a.first << " : ";

        std::visit([&](const auto& arg) {
            //std::cout << arg;

            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>) {
                if (a.first == "engine_mode") {
                    this->m_mode = static_cast<unsigned int>(arg);
                }

                if (a.first == "window_width") {
                    this->m_width = arg;
                }

                if (a.first == "window_height") {
                    this->m_height = arg;
                }
                
            } else if constexpr (std::is_same_v<T, std::string>) {
                if (a.first == "window_title") {
                    this->m_title = arg.c_str();
                }

            }
        }, a.second);

        //std::cout << std::endl;
    }
}

// Core
void EngineManager::OnEngineInit() {
    // Lecture du fichier de configuration
    OnReadConfigFile("config.flx");

    // Initialisation de la fenêtre
    if(this->m_mode == 0) {
        OnWindowInit();
    } else if(this->m_mode == 1) {
        OnWindowInitOpenGL();
    }

    // Juste avant la loop
    runTimer.Start();
    this->m_running = true;
}

void EngineManager::OnEngineUpdate() {
    gameLoop.BeginFrame();

    PollEvents();

    while (gameLoop.ShouldRunFixedUpdate()) {
        FixedUpdate(gameLoop.GetFixedTimeStep());
        gameLoop.ConsumeFixedUpdate();
    }

    Update(gameLoop.GetDeltaTime());
    Render(gameLoop.GetAlpha());

    gameLoop.EndFrame();

    Debug();
}

void EngineManager::OnEngineDestroy() {
    // Arrêt du timer
    runTimer.Stop();

    // Destruction de la fenêtre
    OnWindowDestroy();
}


// Update
void EngineManager::PollEvents() {
    OnWindowHandleEvents();
}

void EngineManager::Debug() {

}

void EngineManager::Update(float dt) {
    //std::cout << "[Update] dt = " << dt << "\n";
    if(this->m_mode == 0) {
        OnWindowUpdateBackground(255, 0, 0);
    } else if(this->m_mode == 1) {
        OnWindowOpenGLUpdateBackground(0.5, 0.5, 0.5);
    }
}

void EngineManager::FixedUpdate(float dt) {
    //std::cout << "[FixedUpdate] dt = " << dt << "\n";
}

void EngineManager::Render(float alpha) {
    //std::cout << "[Render] alpha = " << alpha << "\n";
    if(this->m_mode == 0) {
        OnWindowRender();
    } else if(this->m_mode == 1) {
        OnWindowOpenGLRender();
    }
}

/*
if(this->m_mode == 0) {
    OnWindowRenderBackground(255, 0, 0);

    // -------- OPS -------- //
    
    

    // --------------------- //

    OnWindowRender();
} else if(this->m_mode == 1) {
    OnWindowOpenGLRenderBackground(0.5, 0.5, 0.5);

    // -------- OPS -------- //

    // Update

    timer.Update();
    while(timer.getAcc() >= timer.getMSPerUpdate()) {
        float deltaTime = static_cast<float>(timer.getDeltaTime());
        std::cout << deltaTime << std::endl;

        timer.UpdateDeltaTime();
    }


    // Render

    // --------------------- //

    OnWindowOpenGLRender();
    
}*/