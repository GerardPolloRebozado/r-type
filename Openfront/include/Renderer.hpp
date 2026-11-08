#ifndef OPENFRONT_RENDERER_HPP_
#define OPENFRONT_RENDERER_HPP_

#include "Common.hpp"
#include "NetworkClient.hpp"
#include "Simulation.hpp"
#include <future>
#include <raylib.h>
#include <string>

namespace Engine {
class AssetManager;
class IController;
class Registry;
} // namespace Engine

namespace Openfront {

class NetworkManager;

enum class UIState { StartMenu, HostLobby, JoinLobby, HowToPlay, InGame, InGamePause };

class Renderer {
  public:
    Renderer(int screenWidth, int screenHeight);
    ~Renderer();

    void Shutdown();

    void Init(Simulation& sim, PlayerId localPlayerId = 1);
    void Update(Simulation& sim, NetworkManager& net, float dt);
    void Render(Simulation& sim, const NetworkManager& net);

    // Coordinate conversions
    [[nodiscard]] TileCoord ScreenToTile(Vector2 screenPos) const;
    [[nodiscard]] Vector2 TileToScreen(int x, int y) const;

    // Viewport control
    void CenterCameraOn(int tileX, int tileY);
    void CenterOnPlayer(const Simulation& sim, PlayerId pid);

    void SetUIState(UIState state) {
        if (state == uiState_) {
            return;
        }
        bool wasInGame = (uiState_ == UIState::InGame || uiState_ == UIState::InGamePause);
        bool inGame = (state == UIState::InGame || state == UIState::InGamePause);
        uiState_ = state;
        if (inGame && !wasInGame) {
            SetWindowState(FLAG_FULLSCREEN_MODE);
        } else if (!inGame && wasInGame) {
            ClearWindowState(FLAG_FULLSCREEN_MODE);
        }
    }
    [[nodiscard]] UIState GetUIState() const {
        return uiState_;
    }

    void SetAssetManager(Engine::AssetManager* assetManager) {
        assetManager_ = assetManager;
    }
    void SetSelectedMapType(Openfront::MapType type) {
        selectedMapType_ = type;
    }
    void SetController(Engine::IController* controller) {
        controller_ = controller;
    }
    void SetCameraEntity(std::size_t entity) {
        cameraEntity_ = entity;
    }
    void SetRegistry(Engine::Registry* registry) {
        registry_ = registry;
    }
    void PlaySoundEffect(const std::string& name);
    void DrawTextCustom(const char* text, int x, int y, int size, Color color);
    void DrawTitleCustom(const char* text, int x, int y, int size, Color color);
    void DrawCustomCursor();

  private:
    int screenWidth_{1600};
    int screenHeight_{900};
    UIState uiState_{UIState::StartMenu};

    Engine::AssetManager* assetManager_{nullptr};
    Engine::IController* controller_{nullptr};
    std::size_t cameraEntity_{0};
    Engine::Registry* registry_{nullptr};
    PlayerId localPlayerId_{1};
    Openfront::MapType selectedMapType_{Openfront::MapType::Procedural};
    bool randomSpawn_{true};
    size_t prevExplosionsCount_{0};
    size_t prevMissilesCount_{0};
    bool gameOverSoundPlayed_{false};
    float nukeShakeTimer_{0.0f};

    Camera2D camera_{};
    Texture2D mapTexture_{};
    Image mapImage_{};
    Font gameFont_{};  // LiberationSans-Bold (body text, standard metrics)
    Font titleFont_{}; // Press Start 2P (titles/headers, retro)
    Texture2D buildingIcons_[6]{};
    Texture2D terrainTextures_[4]{};
    Image terrainImages_[4]{};
    Color* pixelBuffer_{nullptr};
    std::size_t pixelBufferSize_{0};
    Color* blockBuffer_{nullptr};
    std::size_t blockBufferSize_{0};
    Texture2D tradeTrain_{};

    // User Interaction State
    TileCoord hoveredTile_{-1, -1};
    TileCoord selectedTile_{-1, -1};
    bool isDraggingAttack_{false};
    TileCoord attackTargetTile_{-1, -1};
    BuildingType activeBuildMode_{BuildingType::None};
    bool isTargetingNuke_{false};
    BombType nukeBombType_{BombType::AtomBomb};
    TileCoord nukeSiloTile_{-1, -1};

    // Menu / Text Inputs
    int activeInputBox_{0}; // 0 = none, 1 = name, 2 = host port, 3 = join IP, 4 = join port
    char playerNameBuffer_[32]{"Commander"};
    char hostPortBuffer_[16]{"7777"};
    char joinIpBuffer_[64]{"127.0.0.1"};
    char joinPortBuffer_[16]{"7777"};
    std::string connectionStatusText_{"Ready to connect"};

    std::future<std::vector<NetworkClient::ScannedRoom>> roomScanFuture_;
    std::vector<NetworkClient::ScannedRoom> joinRooms_;
    bool scanningRooms_{false};
    void PollRoomScan();
    void BeginRoomScan(NetworkManager& net);
    void JoinRoom(Simulation& sim, NetworkManager& net, uint16_t port);

    // Render subroutines
    void UpdateMapTexture(const Simulation& sim);
    void TileColor(const Simulation& sim, const WorldMap& map, int x, int y, const Tile& t,
                   Color& out);
    void DrawWorld(Simulation& sim);
    void DrawBuildings(const Simulation& sim);
    void DrawTradeRoutes(const Simulation& sim);
    void DrawFrontLines(const Simulation& sim);
    void DrawMissiles(const Simulation& sim);
    void DrawExplosions(const Simulation& sim);
    void DrawParticlesAndFloatingText(const Simulation& sim);
    void DrawSelectionHighlights(const Simulation& sim);

    // UI Screens
    void DrawHUD(Simulation& sim, const NetworkManager& net);
    void DrawTopBar(const Simulation& sim, NetworkManager& net);
    void DrawBottomDock(Simulation& sim, NetworkManager& net);
    void DrawLeaderboard(const Simulation& sim, PlayerId localId);
    void DrawGameOver(Simulation& sim);

    void DrawStartMenu(Simulation& sim, NetworkManager& net);
    void DrawHostLobby(Simulation& sim, NetworkManager& net);
    void DrawJoinLobby(Simulation& sim, NetworkManager& net);
    void DrawHowToPlay();
    void DrawPauseMenu(Simulation& sim, NetworkManager& net);

    void HandleInput(Simulation& sim, NetworkManager& net);
    void HandleTextInput(char* buffer, size_t maxLen);
};

} // namespace Openfront

#endif // OPENFRONT_RENDERER_HPP_
