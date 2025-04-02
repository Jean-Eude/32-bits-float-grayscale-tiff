#pragma once


#include <Filesystem.hpp>
#include <Parser.hpp>
#include <Window.hpp>
#include <Timer.hpp>
#include <IntervalTimer.hpp>
#include <FramerateLimiter.hpp>
#include <Gameloop.hpp>

class EngineManager : public Parser, public Window {
    public:
        EngineManager();
        ~EngineManager() = default;

        void OnReadConfigFile(const char* cfg);

        // Core
        void OnEngineInit();
        void OnEngineUpdate();
        void OnEngineDestroy();

    private:
        unsigned int m_mode;

        GameLoop gameLoop;
        Timer runTimer;

        // Update
        void Update(float dt);
        void FixedUpdate(float dt);
        void Render(float alpha);
        void PollEvents();
        void Debug();
};