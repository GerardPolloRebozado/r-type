#include "Renderer.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/Position.hpp"
#include "Core/QuitRequest.hpp"
#include "ECS/Registry.hpp"
#include "Inputs/Controller.hpp"
#include "NetworkManager.hpp"
#include "Systems/CameraSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <raymath.h>

namespace Openfront {

namespace {
constexpr int kTileBlocks = WorldMap::kDirtyBlock;
}

Renderer::Renderer(int screenWidth, int screenHeight)
    : screenWidth_(screenWidth), screenHeight_(screenHeight) {
    camera_.offset = {screenWidth_ * 0.5f, screenHeight_ * 0.5f};
    camera_.target = {kMapWidth * kTileSize * 0.5f, kMapHeight * kTileSize * 0.5f};
    camera_.rotation = 0.0f;
    camera_.zoom = 1.0f;
}

Renderer::~Renderer() {
    Shutdown();
}

void Renderer::Shutdown() {
    if (mapTexture_.id != 0) {
        UnloadTexture(mapTexture_);
        mapTexture_ = {};
    }
    if (mapImage_.data != nullptr) {
        UnloadImage(mapImage_);
        mapImage_ = {};
    }
    if (pixelBuffer_ != nullptr) {
        delete[] pixelBuffer_;
        pixelBuffer_ = nullptr;
    }
    if (blockBuffer_ != nullptr) {
        delete[] blockBuffer_;
        blockBuffer_ = nullptr;
    }
}

namespace {
std::string FormatBigCount(double v) {
    char buf[32];
    if (v < 1000.0) {
        std::snprintf(buf, sizeof(buf), "%d", static_cast<int>(v));
    } else if (v < 1.0e6) {
        double k = v / 1000.0;
        if (k < 100.0) {
            std::snprintf(buf, sizeof(buf), "%.1fk", k);
        } else {
            std::snprintf(buf, sizeof(buf), "%dk", static_cast<int>(k));
        }
    } else {
        double m = v / 1.0e6;
        if (m < 100.0) {
            std::snprintf(buf, sizeof(buf), "%.1fM", m);
        } else {
            std::snprintf(buf, sizeof(buf), "%dM", static_cast<int>(m));
        }
    }
    return buf;
}
} // namespace

void Renderer::PlaySoundEffect(const std::string& name) {
    if (!assetManager_) {
        return;
    }
    try {
        auto snd = assetManager_->GetSound(name);
        if (snd) {
            snd->Play();
        }
    } catch (...) {
    }
}

void Renderer::DrawTextCustom(const char* text, int x, int y, int size, Color color) {
    if (gameFont_.texture.id != 0) {
        // Shadow for readability
        DrawTextEx(gameFont_, text, {static_cast<float>(x + 1), static_cast<float>(y + 1)},
                   static_cast<float>(size), 1.0f, Color{0, 0, 0, 120});
        DrawTextEx(gameFont_, text, {static_cast<float>(x), static_cast<float>(y)},
                   static_cast<float>(size), 1.0f, color);
    } else {
        DrawText(text, x + 1, y + 1, size, Color{0, 0, 0, 120});
        DrawText(text, x, y, size, color);
    }
}

void Renderer::DrawTitleCustom(const char* text, int x, int y, int size, Color color) {
    if (titleFont_.texture.id != 0) {
        DrawTextEx(titleFont_, text, {static_cast<float>(x), static_cast<float>(y)},
                   static_cast<float>(size) * 2.0f, 1.0f, color);
    } else {
        DrawText(text, x, y, size, color);
    }
}

void Renderer::Init(Simulation& sim, PlayerId localPlayerId) {
    localPlayerId_ = localPlayerId;
    gameOverSoundPlayed_ = false;

    // Load terrain textures
    if (terrainTextures_[0].id == 0) {
        terrainTextures_[0] = LoadTexture("Openfront/assets/terrain_plains.png");
        terrainTextures_[1] = LoadTexture("Openfront/assets/terrain_highlands.png");
        terrainTextures_[2] = LoadTexture("Openfront/assets/terrain_mountains.png");
        terrainTextures_[3] = LoadTexture("Openfront/assets/terrain_water.png");
    }

    // Load body font (Liberation Sans Bold - clean, professional, good metrics)
    if (gameFont_.texture.id == 0) {
        gameFont_ = LoadFontEx("Openfront/assets/fonts/LiberationSans-Bold.ttf", 16, nullptr, 0);
        SetTextureFilter(gameFont_.texture, TEXTURE_FILTER_BILINEAR);
    }
    // Load title font (Press Start 2P - retro pixel font for headers)
    if (titleFont_.texture.id == 0) {
        titleFont_ = LoadFontEx("Openfront/assets/fonts/pressstart2p.ttf", 8, nullptr, 0);
        SetTextureFilter(titleFont_.texture, TEXTURE_FILTER_POINT);
    }

    // Load building icons
    const char* iconPaths[] = {
        "Openfront/assets/building_city.png",    "Openfront/assets/building_port.png",
        "Openfront/assets/building_factory.png", "Openfront/assets/building_defense.png",
        "Openfront/assets/building_sam.png",     "Openfront/assets/building_silo.png",
    };
    for (int i = 0; i < 6; ++i) {
        if (buildingIcons_[i].id == 0) {
            buildingIcons_[i] = LoadTexture(iconPaths[i]);
        }
    }

    if (tradeTrain_.id == 0) {
        tradeTrain_ = LoadTexture("Openfront/assets/trade_train.png");
    }

    // Load terrain images for texture sampling
    const char* terrainPaths[] = {
        "Openfront/assets/terrain_plains.png",
        "Openfront/assets/terrain_highlands.png",
        "Openfront/assets/terrain_mountains.png",
        "Openfront/assets/terrain_water.png",
    };
    for (int i = 0; i < 4; ++i) {
        if (terrainImages_[i].data == nullptr) {
            terrainImages_[i] = LoadImage(terrainPaths[i]);
        }
        if (terrainTextures_[i].id == 0) {
            terrainTextures_[i] = LoadTexture(terrainPaths[i]);
        }
    }

    UpdateMapTexture(sim);
    CenterOnPlayer(sim, localPlayerId);
}

void Renderer::CenterOnPlayer(const Simulation& sim, PlayerId pid) {
    const WorldMap& map = sim.GetMap();
    float mapW = map.GetWidth() * kTileSize;
    float mapH = map.GetHeight() * kTileSize;
    camera_.target = {mapW * 0.5f, mapH * 0.5f};
    float fitW = static_cast<float>(screenWidth_);
    float fitH = static_cast<float>(screenHeight_) - 130.0f;
    camera_.zoom = std::min(fitW / mapW, fitH / mapH);
    camera_.zoom = std::clamp(camera_.zoom, 0.02f, 2.0f);
}

void Renderer::CenterCameraOn(int tileX, int tileY) {
    camera_.target = {(tileX + 0.5f) * kTileSize, (tileY + 0.5f) * kTileSize};
}

TileCoord Renderer::ScreenToTile(Vector2 screenPos) const {
    Vector2 worldPos = GetScreenToWorld2D(screenPos, camera_);
    int tx = static_cast<int>(std::floor(worldPos.x / kTileSize));
    int ty = static_cast<int>(std::floor(worldPos.y / kTileSize));
    return {tx, ty};
}

Vector2 Renderer::TileToScreen(int x, int y) const {
    Vector2 worldPos = {(x + 0.5f) * kTileSize, (y + 0.5f) * kTileSize};
    return GetWorldToScreen2D(worldPos, camera_);
}

void Renderer::Update(Simulation& sim, NetworkManager& net, float dt) {
    screenWidth_ = GetScreenWidth();
    screenHeight_ = GetScreenHeight();

    if (sim.GetExplosions().size() > prevExplosionsCount_) {
        PlaySoundEffect("explosion");
        // Trigger screen shake for nuke explosions
        nukeShakeTimer_ = 1.0f;
    }
    prevExplosionsCount_ = sim.GetExplosions().size();

    // Update nuke shake
    if (nukeShakeTimer_ > 0.0f) {
        nukeShakeTimer_ -= dt;
        float intensity = nukeShakeTimer_ * 8.0f;
        camera_.target.x += (rand() % 100 - 50) * 0.01f * intensity;
        camera_.target.y += (rand() % 100 - 50) * 0.01f * intensity;
    }

    if (sim.GetMissiles().size() > prevMissilesCount_) {
        PlaySoundEffect("nuke_launch");
    }
    prevMissilesCount_ = sim.GetMissiles().size();

    HandleInput(sim, net);
    // Sync custom camera_ to engine CameraComponent (demonstrates engine camera usage)
    if (cameraEntity_ != 0 && registry_ != nullptr) {
        if (registry_->HasComponent<Engine::Position>(cameraEntity_)) {
            auto& pos = registry_->RequireComponent<Engine::Position>(cameraEntity_);
            pos.x = camera_.target.x;
            pos.y = camera_.target.y;
        }
        if (registry_->HasComponent<Engine::CameraComponent>(cameraEntity_)) {
            auto& cam = registry_->RequireComponent<Engine::CameraComponent>(cameraEntity_);
            cam.zoom = camera_.zoom;
        }
    }
    UpdateMapTexture(sim);
}

void Renderer::HandleTextInput(char* buffer, size_t maxLen) {
    int key = GetCharPressed();
    while (key > 0) {
        size_t len = std::strlen(buffer);
        if (len < maxLen && (key >= 32 && key <= 126)) {
            buffer[len] = static_cast<char>(key);
            buffer[len + 1] = '\0';
        }
        key = GetCharPressed();
    }
    if (controller_ && controller_->isActionPressed("BACKSPACE")) {
        size_t len = std::strlen(buffer);
        if (len > 0) {
            buffer[len - 1] = '\0';
        }
    }
}

void Renderer::HandleInput(Simulation& sim, NetworkManager& net) {
    float dt = GetFrameTime();
    Vector2 mousePos = GetMousePosition();
    const WorldMap& map = sim.GetMap();
    PlayerId localId = net.GetLocalPlayerId();

    // Menu States text typing
    if (uiState_ == UIState::HostLobby) {
        if (activeInputBox_ == 1) {
            HandleTextInput(playerNameBuffer_, sizeof(playerNameBuffer_) - 1);
        }
        if (activeInputBox_ == 2) {
            HandleTextInput(hostPortBuffer_, sizeof(hostPortBuffer_) - 1);
        }
        if (controller_ && controller_->isActionPressed("CANCEL")) {
            net.Stop();
            uiState_ = UIState::StartMenu;
        }
        if ((controller_ && controller_->isActionPressed("CONFIRM")) && activeInputBox_ == 0) {
            if (net.GetMode() == GameMode::Host) {
                net.StartHostMatch(sim);
                Init(sim, 1);
                uiState_ = UIState::InGame;
            }
        }
        return;
    }
    if (uiState_ == UIState::JoinLobby) {
        if (activeInputBox_ == 1) {
            HandleTextInput(playerNameBuffer_, sizeof(playerNameBuffer_) - 1);
        }
        if (activeInputBox_ == 3) {
            HandleTextInput(joinIpBuffer_, sizeof(joinIpBuffer_) - 1);
        }
        if (activeInputBox_ == 4) {
            HandleTextInput(joinPortBuffer_, sizeof(joinPortBuffer_) - 1);
        }
        if (controller_ && controller_->isActionPressed("CANCEL")) {
            net.Stop();
            uiState_ = UIState::StartMenu;
        }
        return;
    }
    if (uiState_ == UIState::HowToPlay) {
        if ((controller_ && controller_->isActionPressed("CANCEL")) ||
            (controller_ && controller_->isActionPressed("CONFIRM"))) {
            uiState_ = UIState::StartMenu;
        }
        return;
    }
    if (uiState_ == UIState::StartMenu) {
        float mapWorldW = sim.GetMap().GetWidth() * kTileSize;
        float mapWorldH = sim.GetMap().GetHeight() * kTileSize;
        camera_.zoom = std::min(static_cast<float>(screenWidth_) / mapWorldW,
                                static_cast<float>(screenHeight_) / mapWorldH);
        camera_.target.x += 45.0f * dt * camera_.zoom * 8.0f;
        camera_.target.y += 25.0f * dt * camera_.zoom * 8.0f;
        if (camera_.target.x > mapWorldW) {
            camera_.target.x = 0;
        }
        if (camera_.target.y > mapWorldH) {
            camera_.target.y = 0;
        }
        return;
    }

    if (controller_ && controller_->isActionPressed("CANCEL")) {
        if (activeBuildMode_ != BuildingType::None || isTargetingNuke_ || selectedTile_.x >= 0) {
            activeBuildMode_ = BuildingType::None;
            isTargetingNuke_ = false;
            selectedTile_ = {-1, -1};
            isDraggingAttack_ = false;
        } else {
            uiState_ = (uiState_ == UIState::InGame) ? UIState::InGamePause : UIState::InGame;
        }
    }

    if (uiState_ == UIState::InGamePause) {
        return;
    }

    float panSpeed = 650.0f / camera_.zoom * dt;
    bool panUp = false, panDown = false, panLeft = false, panRight = false;
    if (controller_) {
        panUp = controller_->isActionActive("PAN_UP");
        panDown = controller_->isActionActive("PAN_DOWN");
        panLeft = controller_->isActionActive("PAN_LEFT");
        panRight = controller_->isActionActive("PAN_RIGHT");
    } else {
        panUp = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
        panDown = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
        panLeft = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
        panRight = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
    }
    if (panUp) {
        camera_.target.y -= panSpeed;
    }
    if (panDown) {
        camera_.target.y += panSpeed;
    }
    if (panLeft) {
        camera_.target.x -= panSpeed;
    }
    if (panRight) {
        camera_.target.x += panSpeed;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        camera_.target.x -= delta.x / camera_.zoom;
        camera_.target.y -= delta.y / camera_.zoom;
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        Vector2 mouseWorldBefore = GetScreenToWorld2D(mousePos, camera_);
        camera_.zoom = std::clamp(camera_.zoom + wheel * 0.15f * camera_.zoom, 0.02f, 6.0f);
        Vector2 mouseWorldAfter = GetScreenToWorld2D(mousePos, camera_);
        camera_.target.x += (mouseWorldBefore.x - mouseWorldAfter.x);
        camera_.target.y += (mouseWorldBefore.y - mouseWorldAfter.y);
    }

    if (mousePos.y < 50.0f || mousePos.y > screenHeight_ - 80.0f) {
        return;
    }

    TileCoord tile = ScreenToTile(mousePos);
    if (map.InBounds(tile.x, tile.y)) {
        hoveredTile_ = tile;
    } else {
        hoveredTile_ = {-1, -1};
    }

    if (controller_ && controller_->isActionPressed("ATTACK_10")) {
        net.QueueSetAttackRatio(sim, localId, 0.10f);
    }
    if (controller_ && controller_->isActionPressed("ATTACK_20")) {
        net.QueueSetAttackRatio(sim, localId, 0.20f);
    }
    if (controller_ && controller_->isActionPressed("ATTACK_50")) {
        net.QueueSetAttackRatio(sim, localId, 0.50f);
    }
    if (controller_ && controller_->isActionPressed("ATTACK_100")) {
        net.QueueSetAttackRatio(sim, localId, 1.00f);
    }

    if (controller_ && controller_->isActionPressed("BUILD_CITY")) {
        activeBuildMode_ = BuildingType::City;
    }
    if (controller_ && controller_->isActionPressed("BUILD_PORT")) {
        activeBuildMode_ = BuildingType::Port;
    }
    if (controller_ && controller_->isActionPressed("BUILD_FACTORY")) {
        activeBuildMode_ = BuildingType::Factory;
    }
    if (controller_ && controller_->isActionPressed("BUILD_DEFENSE")) {
        activeBuildMode_ = BuildingType::DefensePost;
    }
    if (controller_ && controller_->isActionPressed("BUILD_SAM")) {
        activeBuildMode_ = BuildingType::SamLauncher;
    }
    if (controller_ && controller_->isActionPressed("BUILD_SILO")) {
        activeBuildMode_ = BuildingType::MissileSilo;
    }

    if (controller_ && controller_->isActionPressed("LAUNCH_ATOM")) {
        for (const auto& b : sim.GetBuildings()) {
            if (b.ownerId == localId && b.type == BuildingType::MissileSilo && b.active) {
                isTargetingNuke_ = true;
                nukeBombType_ = BombType::AtomBomb;
                nukeSiloTile_ = {b.x, b.y};
                break;
            }
        }
    }
    if (controller_ && controller_->isActionPressed("LAUNCH_H")) {
        for (const auto& b : sim.GetBuildings()) {
            if (b.ownerId == localId && b.type == BuildingType::MissileSilo && b.active) {
                isTargetingNuke_ = true;
                nukeBombType_ = BombType::HydrogenBomb;
                nukeSiloTile_ = {b.x, b.y};
                break;
            }
        }
    }

    // Accept pending alliance proposals (Enter key when proposals exist)

    // Reinforce front on click
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 worldMouse = GetScreenToWorld2D(mousePos, camera_);
        for (const auto& front : sim.GetAttackFronts()) {
            if (front.attackerId == localId && !front.finished) {
                if (Vector2Distance(worldMouse, front.frontCenter) < kTileSize * 2.0f) {
                    net.QueueReinforce(sim, localId, front.id);
                    return;
                }
            }
        }
    }

    // Right-click cancels front or selection
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        Vector2 worldMouse = GetScreenToWorld2D(mousePos, camera_);
        for (const auto& front : sim.GetAttackFronts()) {
            if (front.attackerId == localId && !front.finished) {
                if (Vector2Distance(worldMouse, front.frontCenter) < kTileSize * 2.0f) {
                    sim.CancelFront(localId, front.id);
                    return;
                }
            }
        }
        selectedTile_ = {-1, -1};
        activeBuildMode_ = BuildingType::None;
        isTargetingNuke_ = false;
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && map.InBounds(tile.x, tile.y)) {
        // Build mode
        if (activeBuildMode_ != BuildingType::None) {
            net.QueueBuild(sim, localId, tile.x, tile.y, activeBuildMode_);
            PlaySoundEffect("build");
            activeBuildMode_ = BuildingType::None;
            return;
        }

        // Nuclear targeting
        if (isTargetingNuke_) {
            net.QueueLaunchNuke(sim, localId, nukeSiloTile_.x, nukeSiloTile_.y, tile.x, tile.y,
                                nukeBombType_);
            PlaySoundEffect("nuke_launch");
            isTargetingNuke_ = false;
            return;
        }

        const Tile& clicked = map.GetTile(tile.x, tile.y);

        if (clicked.ownerId != localId) {
            Player* p = sim.GetPlayer(localId);
            float ratio = p ? p->attackRatio : 0.20f;
            net.QueueAttack(sim, localId, tile.x, tile.y, tile.x, tile.y, ratio);
        } else if (clicked.ownerId == localId) {
            selectedTile_ = tile;
            isDraggingAttack_ = true;
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && isDraggingAttack_) {
        if (map.InBounds(tile.x, tile.y) && selectedTile_.x >= 0 &&
            (tile.x != selectedTile_.x || tile.y != selectedTile_.y)) {
            const Tile& target = map.GetTile(tile.x, tile.y);
            if (target.ownerId != localId) {
                Player* p = sim.GetPlayer(localId);
                float ratio = p ? p->attackRatio : 0.20f;
                net.QueueAttack(sim, localId, selectedTile_.x, selectedTile_.y, tile.x, tile.y,
                                ratio);
            }
        }
        isDraggingAttack_ = false;
    }
}

void Renderer::TileColor(const Simulation& sim, const WorldMap& map, int x, int y, const Tile& t,
                         Color& out) {
    if (t.terrain == TerrainType::Water) {
        out = Color{30, 84, 152, 255};
        return;
    }

    if (t.ownerId != kNeutralPlayerId) {
        const Player* p = sim.GetPlayer(t.ownerId);
        Color base = p ? p->color : GRAY;

        if (t.terrain == TerrainType::Highlands) {
            base.r = static_cast<unsigned char>(std::min(255, base.r + 25));
            base.g = static_cast<unsigned char>(std::min(255, base.g + 25));
            base.b = static_cast<unsigned char>(std::min(255, base.b + 25));
        } else if (t.terrain == TerrainType::Mountains) {
            base.r = static_cast<unsigned char>((base.r + 140) / 2);
            base.g = static_cast<unsigned char>((base.g + 140) / 2);
            base.b = static_cast<unsigned char>((base.b + 140) / 2);
        }

        if (t.falloutTimer > 0.0f) {
            base = Color{75, 45, 80, 255};
        }

        if (t.isBorder) {
            base.r = static_cast<unsigned char>((base.r * 7) / 10);
            base.g = static_cast<unsigned char>((base.g * 7) / 10);
            base.b = static_cast<unsigned char>((base.b * 7) / 10);
        }

        if (t.captureFlashTimer > 0.0f) {
            float flash = t.captureFlashTimer / 0.5f;
            float ripple = std::sin(flash * 3.14159f * 2.0f) * 0.3f + 0.7f;
            base.r = static_cast<unsigned char>(
                std::min(255, base.r + (int)((255 - base.r) * flash * ripple)));
            base.g = static_cast<unsigned char>(
                std::min(255, base.g + (int)((255 - base.g) * flash * ripple)));
            base.b = static_cast<unsigned char>(
                std::min(255, base.b + (int)((230 - base.b) * flash * ripple)));
        }

        out = base;
        return;
    }

    if (t.falloutTimer > 0.0f) {
        out = Color{60, 50, 65, 255};
    } else if (t.terrain == TerrainType::Plains) {
        if (terrainImages_[0].data != nullptr) {
            int tx = (x * 7) % terrainImages_[0].width;
            int ty = (y * 13) % terrainImages_[0].height;
            Color tc = GetImageColor(terrainImages_[0], tx, ty);
            out = Color{tc.r, tc.g, tc.b, 255};
        } else {
            int var = (x * 7 + y * 13) % 15 - 7;
            out =
                Color{static_cast<unsigned char>(135 + var), static_cast<unsigned char>(148 + var),
                      static_cast<unsigned char>(122 + var), 255};
        }
    } else if (t.terrain == TerrainType::Highlands) {
        if (terrainImages_[1].data != nullptr) {
            int tx = (x * 11) % terrainImages_[1].width;
            int ty = (y * 7) % terrainImages_[1].height;
            Color tc = GetImageColor(terrainImages_[1], tx, ty);
            out = Color{tc.r, tc.g, tc.b, 255};
        } else {
            int var = (x * 11 + y * 7) % 20 - 10;
            out =
                Color{static_cast<unsigned char>(160 + var), static_cast<unsigned char>(142 + var),
                      static_cast<unsigned char>(108 + var), 255};
        }
    } else {
        if (terrainImages_[2].data != nullptr) {
            int tx = (x * 13) % terrainImages_[2].width;
            int ty = (y * 11) % terrainImages_[2].height;
            Color tc = GetImageColor(terrainImages_[2], tx, ty);
            out = Color{tc.r, tc.g, tc.b, 255};
        } else {
            int var = (x * 13 + y * 11) % 25 - 12;
            out =
                Color{static_cast<unsigned char>(120 + var), static_cast<unsigned char>(122 + var),
                      static_cast<unsigned char>(128 + var), 255};
        }
    }

    if (t.captureFlashTimer > 0.0f) {
        out = Color{255, 255, 240, 255};
    }
}

void Renderer::UpdateMapTexture(const Simulation& sim) {
    const WorldMap& map = sim.GetMap();
    int width = map.GetWidth();
    int height = map.GetHeight();

    // Recreate texture if map size changed
    if (mapTexture_.width != width || mapTexture_.height != height) {
        if (mapTexture_.id != 0) {
            UnloadTexture(mapTexture_);
        }
        if (mapImage_.data != nullptr) {
            UnloadImage(mapImage_);
        }
        mapImage_ = GenImageColor(width, height, BLACK);
        mapTexture_ = LoadTextureFromImage(mapImage_);
        SetTextureFilter(mapTexture_, TEXTURE_FILTER_POINT);
        const_cast<WorldMap&>(map).MarkAllDirty();
    }

    size_t tileCount = static_cast<size_t>(width) * static_cast<size_t>(height);
    if (pixelBuffer_ == nullptr || pixelBufferSize_ < tileCount) {
        delete[] pixelBuffer_;
        pixelBuffer_ = new Color[tileCount];
        pixelBufferSize_ = tileCount;
    }
    if (blockBuffer_ == nullptr || blockBufferSize_ < kTileBlocks * kTileBlocks) {
        delete[] blockBuffer_;
        blockBuffer_ = new Color[kTileBlocks * kTileBlocks];
        blockBufferSize_ = kTileBlocks * kTileBlocks;
    }

    if (map.IsAllDirty()) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                TileColor(sim, map, x, y, map.GetTile(x, y), pixelBuffer_[y * width + x]);
            }
        }
        UpdateTexture(mapTexture_, pixelBuffer_);
        const_cast<WorldMap&>(map).ClearDirtyBlocks();
        return;
    }

    if (map.GetDirtyBlocks().empty()) {
        return;
    }

    int blockCols = map.GetBlockCols();
    for (uint32_t id : map.GetDirtyBlocks()) {
        int bx = static_cast<int>(id % blockCols) * kTileBlocks;
        int by = static_cast<int>(id / blockCols) * kTileBlocks;
        int bw = std::min(kTileBlocks, width - bx);
        int bh = std::min(kTileBlocks, height - by);
        for (int yy = 0; yy < bh; ++yy) {
            int mapY = by + yy;
            for (int xx = 0; xx < bw; ++xx) {
                int mapX = bx + xx;
                Color c;
                TileColor(sim, map, mapX, mapY, map.GetTile(mapX, mapY), c);
                pixelBuffer_[mapY * width + mapX] = c;
                blockBuffer_[yy * kTileBlocks + xx] = c;
            }
        }
        Rectangle rec = {static_cast<float>(bx), static_cast<float>(by), static_cast<float>(bw),
                         static_cast<float>(bh)};
        UpdateTextureRec(mapTexture_, rec, blockBuffer_);
    }
    const_cast<WorldMap&>(map).ClearDirtyBlocks();
}
void Renderer::Render(Simulation& sim, const NetworkManager& net) {
    BeginDrawing();
    ClearBackground(Color{10, 15, 25, 255});

    BeginMode2D(camera_);
    DrawWorld(sim);
    EndMode2D();

    switch (uiState_) {
        case UIState::StartMenu:
            DrawStartMenu(sim, const_cast<NetworkManager&>(net));
            break;
        case UIState::HostLobby:
            DrawHostLobby(sim, const_cast<NetworkManager&>(net));
            break;
        case UIState::JoinLobby:
            DrawJoinLobby(sim, const_cast<NetworkManager&>(net));
            break;
        case UIState::HowToPlay:
            DrawHowToPlay();
            break;
        case UIState::InGame:
            DrawHUD(sim, net);
            break;
        case UIState::InGamePause:
            DrawHUD(sim, net);
            DrawPauseMenu(sim, const_cast<NetworkManager&>(net));
            break;
    }

    DrawCustomCursor();
    EndDrawing();
}

void Renderer::DrawCustomCursor() {
    Vector2 mousePos = GetMousePosition();
    if (isTargetingNuke_) {
        // Crosshair for nuke targeting
        DrawLineEx({mousePos.x - 15, mousePos.y}, {mousePos.x - 5, mousePos.y}, 2.0f, RED);
        DrawLineEx({mousePos.x + 5, mousePos.y}, {mousePos.x + 15, mousePos.y}, 2.0f, RED);
        DrawLineEx({mousePos.x, mousePos.y - 15}, {mousePos.x, mousePos.y - 5}, 2.0f, RED);
        DrawLineEx({mousePos.x, mousePos.y + 5}, {mousePos.x, mousePos.y + 15}, 2.0f, RED);
        DrawCircleLines(mousePos.x, mousePos.y, 8.0f, RED);
    } else if (activeBuildMode_ != BuildingType::None) {
        // Building placement cursor
        DrawCircleLines(mousePos.x, mousePos.y, 6.0f, Color{100, 200, 100, 200});
        DrawLineEx({mousePos.x - 8, mousePos.y}, {mousePos.x + 8, mousePos.y}, 1.0f,
                   Color{100, 200, 100, 200});
        DrawLineEx({mousePos.x, mousePos.y - 8}, {mousePos.x, mousePos.y + 8}, 1.0f,
                   Color{100, 200, 100, 200});
    }
}

void Renderer::DrawWorld(Simulation& sim) {
    const WorldMap& map = sim.GetMap();
    Rectangle src = {0.0f, 0.0f, static_cast<float>(mapTexture_.width),
                     static_cast<float>(mapTexture_.height)};
    Rectangle dst = {0.0f, 0.0f, map.GetWidth() * kTileSize, map.GetHeight() * kTileSize};
    DrawTexturePro(mapTexture_, src, dst, {0.0f, 0.0f}, 0.0f, WHITE);

    DrawBuildings(sim);
    DrawSelectionHighlights(sim);
    DrawTradeRoutes(sim);
    DrawFrontLines(sim);
    DrawMissiles(sim);
    DrawExplosions(sim);
    DrawParticlesAndFloatingText(sim);
}

void Renderer::DrawBuildings(const Simulation& sim) {
    // Draw defense post ranges first (behind buildings)
    for (const auto& b : sim.GetBuildings()) {
        if (!b.active || b.type != BuildingType::DefensePost) {
            continue;
        }
        float cx = (b.x + 0.5f) * kTileSize;
        float cy = (b.y + 0.5f) * kTileSize;
        float radius = b.range * kTileSize;
        // Semi-transparent red circle
        DrawCircle(cx, cy, radius, Color{220, 60, 60, 30});
        DrawCircleLines(cx, cy, radius, Color{220, 60, 60, 80});
    }

    for (const auto& b : sim.GetBuildings()) {
        if (!b.active) {
            continue;
        }

        float bx = (b.x + 0.5f) * kTileSize;
        float by = (b.y + 0.5f) * kTileSize;

        // Draw building icon if available
        int iconIdx = -1;
        switch (b.type) {
            case BuildingType::City:
                iconIdx = 0;
                break;
            case BuildingType::Port:
                iconIdx = 1;
                break;
            case BuildingType::Factory:
                iconIdx = 2;
                break;
            case BuildingType::DefensePost:
                iconIdx = 3;
                break;
            case BuildingType::SamLauncher:
                iconIdx = 4;
                break;
            case BuildingType::MissileSilo:
                iconIdx = 5;
                break;
            default:
                break;
        }

        if (iconIdx >= 0 && buildingIcons_[iconIdx].id != 0) {
            float iconSize = 28.0f;
            DrawTexturePro(
                buildingIcons_[iconIdx],
                {0, 0, (float)buildingIcons_[iconIdx].width, (float)buildingIcons_[iconIdx].height},
                {bx - iconSize / 2, by - iconSize / 2, iconSize, iconSize}, {0, 0}, 0.0f,
                b.active ? WHITE : Color{100, 100, 100, 150});
        } else {
            // Fallback to shapes
            switch (b.type) {
                case BuildingType::City: {
                    DrawRectangle(bx - 5, by - 6, 4, 10, WHITE);
                    DrawRectangle(bx, by - 8, 5, 12, Color{240, 240, 240, 255});
                    DrawRectangle(bx - 3, by - 2, 2, 2, YELLOW);
                    DrawRectangle(bx + 2, by - 4, 2, 2, YELLOW);
                    break;
                }
                case BuildingType::Port: {
                    DrawCircle(bx, by, 5.0f, Color{40, 160, 220, 255});
                    DrawRectangle(bx - 2, by - 1, 4, 6, WHITE);
                    DrawRectangle(bx - 4, by + 3, 8, 2, WHITE);
                    break;
                }
                case BuildingType::Factory: {
                    DrawRectangle(bx - 6, by - 3, 12, 7, Color{80, 80, 90, 255});
                    DrawRectangle(bx - 4, by - 7, 3, 5, Color{60, 60, 70, 255});
                    DrawRectangle(bx + 1, by - 7, 3, 5, Color{60, 60, 70, 255});
                    DrawCircle(bx - 2.5f, by - 8.0f, 2.0f, Color{200, 200, 200, 160});
                    break;
                }
                case BuildingType::DefensePost: {
                    DrawCircle(bx, by, 5.5f, Color{220, 60, 60, 255});
                    DrawCircleLines(bx, by, 6.5f, WHITE);
                    DrawRectangle(bx - 2, by - 2, 4, 4, WHITE);
                    break;
                }
                case BuildingType::SamLauncher: {
                    DrawCircle(bx, by, 4.5f, Color{50, 180, 190, 255});
                    DrawLine(bx, by, bx + 4, by - 5, WHITE);
                    DrawLine(bx, by, bx - 4, by - 5, WHITE);
                    break;
                }
                case BuildingType::MissileSilo: {
                    DrawCircle(bx, by, 5.5f, Color{40, 40, 40, 255});
                    DrawCircleLines(bx, by, 6.0f, YELLOW);
                    DrawCircle(bx, by, 2.5f, RED);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void Renderer::DrawTradeRoutes(const Simulation& sim) {
    const auto& buildings = sim.GetBuildings();
    for (const auto& b : buildings) {
        if (!b.active) {
            continue;
        }
        if (b.type != BuildingType::Port && b.type != BuildingType::Factory) {
            continue;
        }

        const Player* p = sim.GetPlayer(b.ownerId);
        if (!p) {
            continue;
        }

        float bx = (b.x + 0.5f) * kTileSize;
        float by = (b.y + 0.5f) * kTileSize;

        // Find nearest city of same owner
        float bestDist = 1e9f;
        Vector2 target = {bx, by};
        for (const auto& c : buildings) {
            if (!c.active || c.ownerId != b.ownerId || c.type != BuildingType::City) {
                continue;
            }
            float dx = (c.x + 0.5f) * kTileSize - bx;
            float dy = (c.y + 0.5f) * kTileSize - by;
            float d = std::sqrt(dx * dx + dy * dy);
            if (d < bestDist) {
                bestDist = d;
                target = {(c.x + 0.5f) * kTileSize, (c.y + 0.5f) * kTileSize};
            }
        }

        if (bestDist < 1e8f) {
            // Draw animated trade route
            float time = static_cast<float>(GetTime());
            float pulse = 0.5f + 0.5f * std::sin(time * 2.0f);
            Color routeColor = (b.type == BuildingType::Port) ? Color{100, 200, 200, 120}
                                                              : Color{200, 180, 100, 120};
            DrawLineEx({bx, by}, target, 1.5f, routeColor);

            if (tradeTrain_.id != 0) {
                float t = std::fmod(time * 0.35f, 1.0f);
                Vector2 pos = {bx + (target.x - bx) * t, by + (target.y - by) * t};
                Vector2 dir = {target.x - bx, target.y - by};
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                float angle = len > 0.5f ? atan2f(dir.y, dir.x) : 0.0f;
                float size = 7.0f;
                bool flip = dir.x > 0.15f;
                Rectangle src = flip ? Rectangle{0, 0, -static_cast<float>(tradeTrain_.width),
                                                 static_cast<float>(tradeTrain_.height)}
                                     : Rectangle{0, 0, static_cast<float>(tradeTrain_.width),
                                                 static_cast<float>(tradeTrain_.height)};
                DrawTexturePro(tradeTrain_, src, {pos.x - size / 2, pos.y - size / 2, size, size},
                               {size / 2, size / 2}, angle * RAD2DEG, WHITE);
            } else {
                float t = std::fmod(time * 0.5f, 1.0f);
                Vector2 dot = {bx + (target.x - bx) * t, by + (target.y - by) * t};
                DrawCircle(dot.x, dot.y, 2.0f, routeColor);
            }
        }
    }
}

void Renderer::DrawSelectionHighlights(const Simulation& sim) {
    if (hoveredTile_.x >= 0) {
        Rectangle r = {hoveredTile_.x * kTileSize, hoveredTile_.y * kTileSize, kTileSize,
                       kTileSize};
        DrawRectangleLinesEx(r, 1.5f, Color{255, 255, 255, 180});
    }

    if (selectedTile_.x >= 0) {
        Rectangle r = {selectedTile_.x * kTileSize, selectedTile_.y * kTileSize, kTileSize,
                       kTileSize};
        DrawRectangleLinesEx(r, 2.0f, GOLD);

        if (hoveredTile_.x >= 0 &&
            (hoveredTile_.x != selectedTile_.x || hoveredTile_.y != selectedTile_.y)) {
            Vector2 p1 = {(selectedTile_.x + 0.5f) * kTileSize,
                          (selectedTile_.y + 0.5f) * kTileSize};
            Vector2 p2 = {(hoveredTile_.x + 0.5f) * kTileSize, (hoveredTile_.y + 0.5f) * kTileSize};
            DrawLineEx(p1, p2, 2.5f, Color{255, 220, 50, 200});
            DrawCircleV(p2, 5.0f, RED);
        }
    }

    if (isTargetingNuke_ && hoveredTile_.x >= 0) {
        Vector2 sPos = {(nukeSiloTile_.x + 0.5f) * kTileSize, (nukeSiloTile_.y + 0.5f) * kTileSize};
        Vector2 tPos = {(hoveredTile_.x + 0.5f) * kTileSize, (hoveredTile_.y + 0.5f) * kTileSize};

        DrawLineEx(sPos, tPos, 2.0f, Color{255, 50, 50, 180});
        float blast = BombRadius(nukeBombType_) * kTileSize;
        DrawCircleLines(tPos.x, tPos.y, blast, RED);
        DrawCircle(tPos.x, tPos.y, blast, Color{255, 50, 50, 40});
    }
}

void Renderer::DrawFrontLines(const Simulation& sim) {
    float time = static_cast<float>(GetTime());

    for (const auto& front : sim.GetAttackFronts()) {
        if (front.finished) {
            continue;
        }

        const Player* att = sim.GetPlayer(front.attackerId);
        Color attCol = att ? att->color : WHITE;

        Vector2 targetWorld = {(front.targetX + 0.5f) * kTileSize,
                               (front.targetY + 0.5f) * kTileSize};

        DrawLineEx(front.frontCenter, targetWorld, 2.0f, Color{attCol.r, attCol.g, attCol.b, 120});

        bool isNaval = front.naval;

        if (isNaval) {
            Vector2 center = front.frontCenter;
            float shipSize = 14.0f;
            Vector2 dir = {targetWorld.x - center.x, targetWorld.y - center.y};
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0) {
                dir.x /= len;
                dir.y /= len;
            }
            DrawLineEx({center.x - dir.x * shipSize, center.y - dir.y * shipSize},
                       {center.x - dir.x * shipSize * 3, center.y - dir.y * shipSize * 3}, 3.0f,
                       Color{attCol.r, attCol.g, attCol.b, 80});
            Vector2 tip = {center.x + dir.x * shipSize, center.y + dir.y * shipSize};
            Vector2 left = {center.x - dir.y * shipSize * 0.6f, center.y + dir.x * shipSize * 0.6f};
            Vector2 right = {center.x + dir.y * shipSize * 0.6f,
                             center.y - dir.x * shipSize * 0.6f};
            DrawTriangle(tip, left, right, attCol);
            DrawTriangleLines(tip, left, right, WHITE);
            DrawCircleLines(center.x, center.y, shipSize + 2, WHITE);
        }

        // Troop movement trail
        float trailLen = 30.0f;
        Vector2 dir = {targetWorld.x - front.frontCenter.x, targetWorld.y - front.frontCenter.y};
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0) {
            dir.x /= len;
            dir.y /= len;
            for (int i = 0; i < 5; ++i) {
                float t = i / 5.0f;
                Vector2 pos = {front.frontCenter.x - dir.x * trailLen * t,
                               front.frontCenter.y - dir.y * trailLen * t};
                float alpha = (1.0f - t) * 180.0f;
                DrawCircle(pos.x, pos.y, 2.0f + (1.0f - t) * 2.0f,
                           Color{attCol.r, attCol.g, attCol.b, (unsigned char)alpha});
            }
        }

        float pulse = std::sin(time * 6.0f) * 2.0f;
        DrawCircleLines(targetWorld.x, targetWorld.y, 8.0f + pulse, attCol);
        DrawCircle(targetWorld.x, targetWorld.y, 3.0f, RED);

        char troopBuf[32];
        if (front.troops >= 1000.0) {
            std::snprintf(troopBuf, sizeof(troopBuf), "%.1fk ⚔", front.troops / 1000.0);
        } else {
            std::snprintf(troopBuf, sizeof(troopBuf), "%d ⚔", static_cast<int>(front.troops));
        }

        int textW = MeasureText(troopBuf, 11);
        Rectangle pill = {front.frontCenter.x - (textW + 10) * 0.5f, front.frontCenter.y - 16.0f,
                          static_cast<float>(textW + 10), 15.0f};
        DrawRectangleRounded(pill, 0.4f, 4, Color{15, 20, 30, 230});
        DrawRectangleRoundedLines(pill, 0.4f, 4, attCol);
        DrawTextCustom(troopBuf, static_cast<int>(pill.x + 5), static_cast<int>(pill.y + 2), 11,
                       WHITE);
    }
}

void Renderer::DrawMissiles(const Simulation& sim) {
    for (const auto& beam : sim.GetBeams()) {
        if (beam.finished) {
            continue;
        }
        DrawLineEx(beam.samPos, beam.missilePos, 2.5f, Color{60, 220, 255, 230});
    }

    for (const auto& m : sim.GetMissiles()) {
        if (m.exploded) {
            continue;
        }
        DrawLineEx(m.startPos, m.targetPos, 1.0f, Color{255, 255, 255, 70});
        DrawCircleV(m.currentPos, 4.0f, (m.type == BombType::HydrogenBomb) ? RED : ORANGE);
        DrawCircleLines(m.currentPos.x, m.currentPos.y, 4.5f, WHITE);
        DrawCircle(m.currentPos.x - 2, m.currentPos.y + 2, 2.0f, Color{200, 200, 200, 180});
    }
}

void Renderer::DrawExplosions(const Simulation& sim) {
    for (const auto& exp : sim.GetExplosions()) {
        if (exp.finished) {
            continue;
        }

        float alpha = 1.0f - (exp.elapsed / exp.duration);
        Color fillCol = exp.color;
        fillCol.a = static_cast<unsigned char>(alpha * 120.0f);

        Color ringCol = WHITE;
        ringCol.a = static_cast<unsigned char>(alpha * 240.0f);

        DrawCircle(exp.pos.x, exp.pos.y, exp.currentRadius, fillCol);
        DrawCircleLines(exp.pos.x, exp.pos.y, exp.currentRadius, ringCol);
    }
}

void Renderer::DrawParticlesAndFloatingText(const Simulation& sim) {
    for (const auto& p : sim.GetClashParticles()) {
        DrawCircleV(p.pos, 2.0f, p.color);
    }

    for (const auto& ft : sim.GetFloatingTexts()) {
        float alpha = 1.0f - (ft.age / ft.duration);
        Color col = ft.color;
        col.a = static_cast<unsigned char>(alpha * 255.0f);
        DrawTextCustom(ft.text.c_str(), static_cast<int>(ft.pos.x - 30), static_cast<int>(ft.pos.y),
                       12, col);
    }
}

void Renderer::DrawHUD(Simulation& sim, const NetworkManager& net) {
    DrawTopBar(sim, const_cast<NetworkManager&>(net));
    DrawBottomDock(sim, const_cast<NetworkManager&>(net));
    DrawLeaderboard(sim, net.GetLocalPlayerId());

    if (sim.IsGameOver()) {
        DrawGameOver(sim);
    }
}

void Renderer::DrawTopBar(const Simulation& sim, NetworkManager& net) {
    DrawRectangle(0, 0, screenWidth_, 46, Color{15, 20, 30, 240});
    DrawLine(0, 46, screenWidth_, 46, Color{40, 50, 70, 255});

    PlayerId localId = net.GetLocalPlayerId();
    const Player* localPlayer = sim.GetPlayer(localId);
    if (!localPlayer) {
        return;
    }

    // Faction Badge
    DrawRectangle(15, 10, 26, 26, localPlayer->color);
    DrawRectangleLines(15, 10, 26, 26, WHITE);
    DrawTitleCustom("OPENFRONT", 50, 13, 10, WHITE);

    // Troops Status
    char troopText[64];
    std::snprintf(troopText, sizeof(troopText), "TROOPS: %s / %s",
                  FormatBigCount(localPlayer->troops).c_str(),
                  FormatBigCount(localPlayer->maxTroops).c_str());
    DrawTextCustom(troopText, 200, 14, 15, Color{120, 220, 140, 255});

    float troopPct =
        static_cast<float>(localPlayer->troops / std::max(1.0, localPlayer->maxTroops));
    DrawRectangle(410, 16, 100, 14, Color{40, 50, 60, 255});
    DrawRectangle(410, 16, static_cast<int>(100 * std::clamp(troopPct, 0.0f, 1.0f)), 14,
                  Color{50, 200, 100, 255});
    DrawRectangleLines(410, 16, 100, 14, Color{80, 90, 110, 255});

    // Gold Status
    char goldText[32];
    std::snprintf(goldText, sizeof(goldText), "GOLD: $%s",
                  FormatBigCount(localPlayer->gold).c_str());
    DrawTextCustom(goldText, 530, 14, 15, Color{255, 215, 50, 255});

    // Territory %
    int totalLand = sim.GetMap().GetLandTilesCount();
    float landPct =
        (totalLand > 0) ? (static_cast<float>(localPlayer->tileCount) / totalLand * 100.0f) : 0.0f;
    char landText[32];
    std::snprintf(landText, sizeof(landText), "LAND: %.1f%%", landPct);
    DrawTextCustom(landText, 720, 14, 15, Color{100, 190, 255, 255});

    // Match Timer
    int matchSec = static_cast<int>(sim.GetMatchTime());
    int remSec = std::max(0, static_cast<int>(sim.GetMatchDuration()) - matchSec);
    char timerText[32];
    std::snprintf(timerText, sizeof(timerText), "%02d:%02d", remSec / 60, remSec % 60);
    DrawTextCustom(timerText, 890, 14, 15, Color{230, 230, 240, 255});

    // Multiplayer Status Badge
    Rectangle netBadge = {static_cast<float>(screenWidth_ - 290), 8.0f, 160.0f, 30.0f};
    char netStatus[64];
    Color badgeCol = GREEN;
    if (net.GetMode() == GameMode::SinglePlayer) {
        std::snprintf(netStatus, sizeof(netStatus), "[SP] LOCAL");
        badgeCol = Color{80, 180, 90, 255};
    } else if (net.GetMode() == GameMode::Host) {
        std::snprintf(netStatus, sizeof(netStatus), "[HOST] P:%d", net.GetServer().GetPort());
        badgeCol = GOLD;
    } else if (net.GetMode() == GameMode::Client) {
        std::snprintf(netStatus, sizeof(netStatus), "[ONLINE] %dms", net.GetPing());
        badgeCol = Color{40, 180, 240, 255};
    }
    DrawRectangleRec(netBadge, Color{25, 35, 50, 200});
    DrawRectangleLinesEx(netBadge, 1.0f, badgeCol);
    DrawTextCustom(netStatus, netBadge.x + 10, netBadge.y + 7, 13, badgeCol);

    // [ESC] Menu Button
    Rectangle menuBtn = {static_cast<float>(screenWidth_ - 110), 8.0f, 95.0f, 30.0f};
    bool hoverMenu = CheckCollisionPointRec(GetMousePosition(), menuBtn);
    DrawRectangleRec(menuBtn, hoverMenu ? Color{50, 140, 220, 240} : Color{35, 45, 65, 220});
    DrawRectangleLinesEx(menuBtn, 1.0f, hoverMenu ? WHITE : Color{70, 90, 130, 255});
    DrawTextCustom("[ESC] Menu", menuBtn.x + 12, menuBtn.y + 7, 13, WHITE);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverMenu) {
        PlaySoundEffect("ui_click");
        uiState_ = UIState::InGamePause;
    }
}

void Renderer::DrawBottomDock(Simulation& sim, NetworkManager& net) {
    float dockH = 68.0f;
    float dockY = screenHeight_ - dockH;

    DrawRectangle(0, dockY, screenWidth_, dockH, Color{15, 20, 30, 240});
    DrawLine(0, dockY, screenWidth_, dockY, Color{40, 50, 70, 255});

    PlayerId localId = net.GetLocalPlayerId();
    Player* localPlayer = sim.GetPlayer(localId);
    if (!localPlayer) {
        return;
    }

    // Attack Slider
    DrawTextCustom("PUSH ARMY:", 15, dockY + 12, 12, GRAY);

    const float ratios[] = {0.10f, 0.20f, 0.50f, 1.00f};
    const char* ratioLabels[] = {"[Z] 10%", "[X] 20%", "[C] 50%", "[V] 100%"};

    for (int i = 0; i < 4; ++i) {
        Rectangle btn = {90.0f + i * 66.0f, dockY + 8.0f, 60.0f, 24.0f};
        bool isActive = (std::abs(localPlayer->attackRatio - ratios[i]) < 0.03f);

        Color bg = isActive ? Color{40, 140, 250, 220} : Color{35, 45, 60, 220};
        DrawRectangleRec(btn, bg);
        DrawRectangleLinesEx(btn, 1.0f, isActive ? WHITE : Color{70, 80, 100, 255});
        DrawTextCustom(ratioLabels[i], btn.x + 5, btn.y + 6, 11, WHITE);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(GetMousePosition(), btn)) {
            PlaySoundEffect("ui_click");
            net.QueueSetAttackRatio(sim, localId, ratios[i]);
        }
    }

    // Build Buttons
    struct BuildBtn {
        BuildingType type;
        const char* label;
        const char* cost;
    };

    BuildBtn bBtns[] = {{BuildingType::City, "[1] City", "$40k"},
                        {BuildingType::Port, "[2] Port", "$35k"},
                        {BuildingType::Factory, "[3] Factory", "$30k"},
                        {BuildingType::DefensePost, "[4] Defense", "$25k"},
                        {BuildingType::SamLauncher, "[5] SAM", "$45k"},
                        {BuildingType::MissileSilo, "[6] Silo", "$75k"}};

    float startX = 380.0f;
    for (int i = 0; i < 6; ++i) {
        Rectangle btn = {startX + i * 105.0f, dockY + 8.0f, 98.0f, 48.0f};
        bool isSelected = (activeBuildMode_ == bBtns[i].type);
        bool canAfford = (localPlayer->gold >= BuildingCost(bBtns[i].type));

        Vector2 mousePos = GetMousePosition();
        bool isHovered = CheckCollisionPointRec(mousePos, btn);

        Color bg = isSelected
                       ? Color{50, 160, 100, 220}
                       : (canAfford ? (isHovered ? Color{45, 60, 80, 220} : Color{30, 40, 55, 220})
                                    : Color{25, 25, 30, 200});
        DrawRectangleRec(btn, bg);
        DrawRectangleLinesEx(btn, 1.0f,
                             isSelected ? WHITE
                                        : (isHovered ? Color{100, 130, 170, 255}
                                                     : (canAfford ? Color{60, 80, 110, 255}
                                                                  : Color{50, 50, 60, 255})));

        // Draw building icon
        if (buildingIcons_[i].id != 0) {
            DrawTexturePro(buildingIcons_[i],
                           {0, 0, (float)buildingIcons_[i].width, (float)buildingIcons_[i].height},
                           {btn.x + 4, btn.y + 4, 24, 24}, {0, 0}, 0.0f,
                           canAfford ? WHITE : Color{100, 100, 100, 150});
        }
        DrawTextCustom(bBtns[i].label, btn.x + 30, btn.y + 6, 11, canAfford ? WHITE : GRAY);
        DrawTextCustom(bBtns[i].cost, btn.x + 30, btn.y + 24, 12,
                       canAfford ? GOLD : Color{120, 110, 70, 255});

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            CheckCollisionPointRec(GetMousePosition(), btn)) {
            PlaySoundEffect("ui_click");
            if (activeBuildMode_ == bBtns[i].type) {
                activeBuildMode_ = BuildingType::None;
            } else {
                activeBuildMode_ = bBtns[i].type;
                isTargetingNuke_ = false;
            }
        }
    }

    // Nuke Button
    Rectangle nukeBtn = {startX + 6 * 105.0f + 15.0f, dockY + 8.0f, 105.0f, 48.0f};
    bool hasSilo = (localPlayer->siloCount > 0);
    bool canAffordNuke = hasSilo && (localPlayer->gold >= BombCost(BombType::AtomBomb));
    bool isNukeActive = isTargetingNuke_;

    Color nukeBg = isNukeActive ? Color{220, 50, 50, 240}
                                : (canAffordNuke ? Color{60, 30, 35, 220} : Color{30, 25, 25, 200});
    DrawRectangleRec(nukeBtn, nukeBg);
    DrawRectangleLinesEx(nukeBtn, 1.0f,
                         isNukeActive ? WHITE : (canAffordNuke ? RED : Color{70, 50, 50, 255}));
    DrawTextCustom("[N] Atom Bomb", nukeBtn.x + 6, nukeBtn.y + 6, 11, canAffordNuke ? WHITE : GRAY);
    DrawTextCustom("$100k", nukeBtn.x + 6, nukeBtn.y + 24, 13, canAffordNuke ? GOLD : GRAY);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), nukeBtn) && hasSilo) {
        if (isTargetingNuke_) {
            isTargetingNuke_ = false;
        } else {
            for (const auto& b : sim.GetBuildings()) {
                if (b.ownerId == localId && b.type == BuildingType::MissileSilo && b.active) {
                    isTargetingNuke_ = true;
                    nukeBombType_ = BombType::AtomBomb;
                    nukeSiloTile_ = {b.x, b.y};
                    activeBuildMode_ = BuildingType::None;
                    break;
                }
            }
        }
    }
}

void Renderer::DrawLeaderboard(const Simulation& sim, PlayerId localId) {
    float boardW = 220.0f;
    float boardH = 260.0f;
    float boardX = screenWidth_ - boardW - 15.0f;
    float boardY = 56.0f;

    DrawRectangle(boardX, boardY, boardW, boardH, Color{15, 20, 30, 220});
    DrawRectangleLines(boardX, boardY, boardW, boardH, Color{40, 50, 70, 255});
    DrawTextCustom("FACTION RANKINGS", boardX + 12, boardY + 10, 14, Color{200, 220, 240, 255});

    DrawLine(boardX + 10, boardY + 28, boardX + boardW - 10, boardY + 28, Color{50, 60, 80, 255});

    std::vector<const Player*> sorted;
    for (const auto& p : sim.GetPlayers()) {
        sorted.push_back(&p);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const Player* a, const Player* b) { return a->tileCount > b->tileCount; });

    int totalLand = sim.GetMap().GetLandTilesCount();

    int rowY = boardY + 36;
    for (size_t i = 0; i < sorted.size() && i < 8; ++i) {
        const Player* p = sorted[i];

        // Rank number
        char rankBuf[8];
        std::snprintf(rankBuf, sizeof(rankBuf), "#%d", (int)(i + 1));
        DrawTextCustom(rankBuf, boardX + 8, rowY + 2, 10, Color{120, 140, 170, 255});

        // Color indicator
        DrawCircle(boardX + 30, rowY + 8, 5.0f, p->color);
        DrawCircleLines(boardX + 30, rowY + 8, 5.0f, WHITE);

        Color nameCol =
            p->isAlive ? (p->id == localId
                              ? GOLD
                              : (p->type == FactionType::Human ? Color{80, 220, 255, 255} : WHITE))
                       : GRAY;
        std::string displayName =
            (p->type == FactionType::Human && p->id != localId) ? (p->name + " [P]") : p->name;
        if (displayName.length() > 12) {
            displayName = displayName.substr(0, 11) + ".";
        }
        DrawTextCustom(displayName.c_str(), boardX + 40, rowY + 2, 11, nameCol);

        float pct =
            (totalLand > 0) ? (static_cast<float>(p->tileCount) / totalLand * 100.0f) : 0.0f;
        char statsBuf[32];
        if (p->isAlive) {
            std::snprintf(statsBuf, sizeof(statsBuf), "%.0f%% %dk", pct,
                          static_cast<int>(p->troops / 1000.0));
        } else {
            std::snprintf(statsBuf, sizeof(statsBuf), "DEAD");
        }
        DrawTextCustom(statsBuf, boardX + 140, rowY + 2, 10,
                       p->isAlive ? Color{180, 200, 220, 255} : RED);

        // Territory progress bar
        if (p->isAlive) {
            Rectangle barBg = {boardX + 40, static_cast<float>(rowY + 16), 160.0f, 3.0f};
            DrawRectangleRec(barBg, Color{30, 40, 55, 255});
            Rectangle barFill = {boardX + 40, static_cast<float>(rowY + 16),
                                 160.0f * (pct / 100.0f), 3.0f};
            DrawRectangleRec(barFill, p->color);
        }

        rowY += 26;
    }
}

void Renderer::DrawGameOver(Simulation& sim) {
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Color{0, 0, 0, 200});

    if (!gameOverSoundPlayed_) {
        PlayerId winId = sim.GetWinnerId();
        if (winId == 1) {
            PlaySoundEffect("victory");
        } else {
            PlaySoundEffect("defeat");
        }
        gameOverSoundPlayed_ = true;
    }

    PlayerId winId = sim.GetWinnerId();
    const Player* win = sim.GetPlayer(winId);

    const char* title =
        (winId == localPlayerId_) ? "VICTORY! WORLD CONQUERED" : "DEFEAT! WORLD DOMINATION LOST";
    Color titleCol = (winId == localPlayerId_) ? GOLD : RED;

    int tw = MeasureText(title, 36);
    DrawTextCustom(title, (screenWidth_ - tw) / 2, screenHeight_ / 2 - 80, 36, titleCol);

    char winnerMsg[128];
    std::snprintf(winnerMsg, sizeof(winnerMsg), "Dominant Power: %s with %d tiles",
                  win ? win->name.c_str() : "Nobody", win ? win->tileCount : 0);
    int mw = MeasureText(winnerMsg, 20);
    DrawTextCustom(winnerMsg, (screenWidth_ - mw) / 2, screenHeight_ / 2 - 20, 20, WHITE);

    const char* sub = "Press [R] or [SPACE] to Restart Match";
    int sw = MeasureText(sub, 18);
    DrawTextCustom(sub, (screenWidth_ - sw) / 2, screenHeight_ / 2 + 40, 18,
                   Color{180, 220, 255, 255});

    if (controller_ && (controller_->isActionPressed("RESTART"))) {
        sim.Init(4, 4, static_cast<uint32_t>(GetTime() * 1000.0));
        Init(sim, 1);
    }
}

void Renderer::DrawStartMenu(Simulation& sim, NetworkManager& net) {
    // Semi-transparent glassmorphism backdrop
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Color{5, 10, 20, 180});

    float menuW = 480.0f;
    float menuH = 500.0f;
    float menuX = (screenWidth_ - menuW) * 0.5f;
    float menuY = (screenHeight_ - menuH) * 0.5f;

    Rectangle card = {menuX, menuY, menuW, menuH};
    DrawRectangleRec(card, Color{18, 24, 38, 240});
    DrawRectangleLinesEx(card, 2.0f, Color{40, 140, 250, 255});

    // Animated Title
    const char* title = "OPENFRONT";
    int tw = MeasureText(title, 48);
    DrawTextCustom(title, menuX + (menuW - tw) * 0.5f, menuY + 28, 48, GOLD);

    const char* sub = "REAL-TIME TERRITORY STRATEGY";
    int sw = MeasureText(sub, 13);
    DrawTextCustom(sub, menuX + (menuW - sw) * 0.5f, menuY + 84, 13, Color{160, 200, 240, 255});

    DrawLine(menuX + 30, menuY + 110, menuX + menuW - 30, menuY + 110, Color{40, 60, 90, 255});

    Vector2 mousePos = GetMousePosition();

    // Map selection
    DrawTextCustom("Map:", menuX + 30, menuY + 118, 12, Color{160, 200, 240, 255});
    const char* mapNames[] = {"Procedural", "Europe", "World", "Pangaea", "Archipelago"};
    Openfront::MapType mapTypes[] = {Openfront::MapType::Procedural, Openfront::MapType::Europe,
                                     Openfront::MapType::World, Openfront::MapType::Pangaea,
                                     Openfront::MapType::Archipelago};
    for (int i = 0; i < 5; ++i) {
        Rectangle mapBtn = {menuX + 80.0f + i * 78.0f, menuY + 116, 72, 22};
        bool isSel = (selectedMapType_ == mapTypes[i]);
        Color bg = isSel ? Color{40, 120, 220, 255} : Color{25, 35, 52, 255};
        DrawRectangleRec(mapBtn, bg);
        DrawRectangleLinesEx(mapBtn, 1.0f, isSel ? WHITE : Color{60, 80, 110, 255});
        DrawTextCustom(mapNames[i], mapBtn.x + 4, mapBtn.y + 6, 9, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, mapBtn)) {
            PlaySoundEffect("ui_click");
            selectedMapType_ = mapTypes[i];
        }
    }

    // Random spawn toggle
    Rectangle spawnToggle = {menuX + 40, menuY + 138, 20, 20};
    DrawRectangleRec(spawnToggle, randomSpawn_ ? Color{40, 150, 90, 230} : Color{60, 60, 70, 230});
    DrawRectangleLinesEx(spawnToggle, 1.0f, WHITE);
    DrawTextCustom("Random Spawn", menuX + 65, menuY + 140, 12, WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, spawnToggle)) {
        PlaySoundEffect("ui_click");
        randomSpawn_ = !randomSpawn_;
    }

    struct MenuBtn {
        const char* label;
        Color col;
        UIState next;
    };

    MenuBtn btns[] = {{"PLAY SINGLEPLAYER", Color{40, 150, 90, 230}, UIState::InGame},
                      {"HOST MULTIPLAYER", Color{35, 100, 180, 230}, UIState::HostLobby},
                      {"JOIN MULTIPLAYER", Color{30, 80, 160, 230}, UIState::JoinLobby},
                      {"HOW TO PLAY", Color{40, 50, 75, 230}, UIState::HowToPlay}};

    float startY = menuY + 165.0f;
    for (int i = 0; i < 4; ++i) {
        Rectangle btn = {menuX + 40.0f, startY + i * 62.0f, menuW - 80.0f, 48.0f};
        bool hover = CheckCollisionPointRec(mousePos, btn);

        Color fill = hover ? Color{static_cast<unsigned char>(btns[i].col.r + 30),
                                   static_cast<unsigned char>(btns[i].col.g + 30),
                                   static_cast<unsigned char>(btns[i].col.b + 30), 255}
                           : btns[i].col;
        DrawRectangleRec(btn, fill);
        DrawRectangleLinesEx(btn, 1.5f, hover ? WHITE : Color{70, 120, 190, 255});

        int lw = MeasureText(btns[i].label, 16);
        DrawTextCustom(btns[i].label, btn.x + (btn.width - lw) * 0.5f, btn.y + 15, 16, WHITE);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hover) {
            PlaySoundEffect("ui_click");
            if (btns[i].next == UIState::InGame) {
                int spawnX = -1, spawnY = -1;
                if (!randomSpawn_) {
                    int w = 0, h = 0;
                    Openfront::WorldMap::MapSizeFor(selectedMapType_, w, h);
                    spawnX = w / 2;
                    spawnY = h / 2;
                }
                net.StartSinglePlayer(sim, selectedMapType_, spawnX, spawnY);
                Init(sim, 1);
                uiState_ = UIState::InGame;
            } else {
                uiState_ = btns[i].next;
            }
        }
    }

    // Load Game button
    Rectangle loadBtn = {menuX + 40.0f, startY + 4 * 62.0f, (menuW - 80.0f) / 2 - 5, 40.0f};
    bool hoverLoad = CheckCollisionPointRec(mousePos, loadBtn);
    DrawRectangleRec(loadBtn, hoverLoad ? Color{40, 100, 180, 240} : Color{30, 75, 140, 220});
    DrawRectangleLinesEx(loadBtn, 1.0f, hoverLoad ? WHITE : Color{60, 140, 220, 255});
    int lw = MeasureText("LOAD GAME", 14);
    DrawTextCustom("LOAD GAME", loadBtn.x + (loadBtn.width - lw) * 0.5f, loadBtn.y + 13, 14, WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverLoad) {
        PlaySoundEffect("ui_click");
        if (sim.LoadGame("savegame.txt")) {
            Init(sim, 1);
            uiState_ = UIState::InGame;
        }
    }

    // Quit button
    Rectangle quitBtn = {menuX + 40.0f + (menuW - 80.0f) / 2 + 5, startY + 4 * 62.0f,
                         (menuW - 80.0f) / 2 - 5, 40.0f};
    bool hoverQuit = CheckCollisionPointRec(mousePos, quitBtn);
    DrawRectangleRec(quitBtn, hoverQuit ? Color{180, 40, 50, 220} : Color{30, 35, 45, 200});
    DrawRectangleLinesEx(quitBtn, 1.0f, hoverQuit ? WHITE : Color{60, 60, 70, 255});
    int qw = MeasureText("EXIT GAME", 14);
    DrawTextCustom("EXIT GAME", quitBtn.x + (quitBtn.width - qw) * 0.5f, quitBtn.y + 13, 14, GRAY);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverQuit) {
        // Don't CloseWindow() mid-frame: EndDrawing would then crash. Ask the
        // engine loop to stop and let Application tear everything down.
        if (registry_ != nullptr) {
            if (!registry_->HasResource<Engine::QuitRequest>()) {
                registry_->AddResource<Engine::QuitRequest>();
            }
            registry_->GetResource<Engine::QuitRequest>().requested = true;
        }
    }
}

void Renderer::DrawHostLobby(Simulation& sim, NetworkManager& net) {
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Color{5, 10, 20, 210});

    float menuW = 540.0f;
    float menuH = 520.0f;
    float menuX = (screenWidth_ - menuW) * 0.5f;
    float menuY = (screenHeight_ - menuH) * 0.5f;

    Rectangle card = {menuX, menuY, menuW, menuH};
    DrawRectangleRec(card, Color{18, 24, 38, 248});
    DrawRectangleLinesEx(card, 2.0f, Color{40, 140, 250, 255});

    DrawTextCustom("HOST MULTIPLAYER ROOM", menuX + 30, menuY + 22, 22, GOLD);
    DrawLine(menuX + 25, menuY + 54, menuX + menuW - 25, menuY + 54, Color{40, 60, 90, 255});

    Vector2 mousePos = GetMousePosition();
    bool isHosting = (net.GetMode() == GameMode::Host);

    // Player Name
    DrawTextCustom("Host Commander Name:", menuX + 30, menuY + 68, 13, WHITE);
    Rectangle nameBox = {menuX + 30, menuY + 86, menuW - 60, 30};
    DrawRectangleRec(nameBox,
                     (activeInputBox_ == 1) ? Color{35, 55, 85, 255} : Color{25, 35, 52, 255});
    DrawRectangleLinesEx(nameBox, 1.0f, (activeInputBox_ == 1) ? GOLD : Color{60, 80, 110, 255});
    DrawTextCustom(playerNameBuffer_, nameBox.x + 10, nameBox.y + 7, 14, WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, nameBox)) {
        activeInputBox_ = 1;
    }

    // Server Port & Host Toggle
    DrawTextCustom("UDP Port:", menuX + 30, menuY + 124, 13, WHITE);
    Rectangle portBox = {menuX + 30, menuY + 142, 130, 30};
    DrawRectangleRec(portBox,
                     (activeInputBox_ == 2) ? Color{35, 55, 85, 255} : Color{25, 35, 52, 255});
    DrawRectangleLinesEx(portBox, 1.0f, (activeInputBox_ == 2) ? GOLD : Color{60, 80, 110, 255});
    DrawTextCustom(hostPortBuffer_, portBox.x + 10, portBox.y + 7, 14, WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, portBox)) {
        activeInputBox_ = 2;
    }

    // Map selection
    DrawTextCustom("Map:", menuX + 30, menuY + 142, 13, WHITE);
    const char* mapNames[] = {"Procedural", "Europe", "World", "Pangaea", "Archipelago"};
    Openfront::MapType mapTypes[] = {Openfront::MapType::Procedural, Openfront::MapType::Europe,
                                     Openfront::MapType::World, Openfront::MapType::Pangaea,
                                     Openfront::MapType::Archipelago};
    float mapBtnX = menuX + 80;
    for (int i = 0; i < 5; ++i) {
        Rectangle mapBtn = {mapBtnX + i * 90.0f, menuY + 140, 85, 28};
        bool isSel = (selectedMapType_ == mapTypes[i]);
        Color bg = isSel ? Color{40, 120, 220, 255} : Color{25, 35, 52, 255};
        DrawRectangleRec(mapBtn, bg);
        DrawRectangleLinesEx(mapBtn, 1.0f, isSel ? WHITE : Color{60, 80, 110, 255});
        DrawTextCustom(mapNames[i], mapBtn.x + 4, mapBtn.y + 8, 10, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, mapBtn)) {
            PlaySoundEffect("ui_click");
            selectedMapType_ = mapTypes[i];
        }
    }

    if (!isHosting) {
        Rectangle createBtn = {menuX + 175, menuY + 180, menuW - 205, 30};
        bool hoverCreate = CheckCollisionPointRec(mousePos, createBtn);
        DrawRectangleRec(createBtn,
                         hoverCreate ? Color{40, 120, 220, 255} : Color{30, 90, 180, 230});
        DrawRectangleLinesEx(createBtn, 1.0f, hoverCreate ? WHITE : Color{60, 140, 240, 255});
        int cw = MeasureText("CREATE ROOM & LISTEN", 13);
        DrawTextCustom("CREATE ROOM & LISTEN", createBtn.x + (createBtn.width - cw) * 0.5f,
                       createBtn.y + 8, 13, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverCreate) {
            PlaySoundEffect("ui_click");
            int port = std::atoi(hostPortBuffer_);
            if (port <= 0) {
                port = 7777;
            }
            net.StartHost(sim, static_cast<uint16_t>(port), playerNameBuffer_, selectedMapType_);
        }
    } else {
        char portStatus[64];
        std::snprintf(portStatus, sizeof(portStatus), "● LISTENING ON PORT %s", hostPortBuffer_);
        DrawTextCustom(portStatus, menuX + 180, menuY + 150, 13, GREEN);
    }

    // Connected Players in Lobby box
    Rectangle lobbyBox = {menuX + 30, menuY + 185, menuW - 60, 175};
    DrawRectangleRec(lobbyBox, Color{12, 16, 26, 220});
    DrawRectangleLinesEx(lobbyBox, 1.0f, Color{45, 60, 85, 255});

    DrawTextCustom("LOBBY COMMANDERS (HUMAN & AI):", lobbyBox.x + 12, lobbyBox.y + 8, 12,
                   Color{140, 180, 230, 255});
    DrawLine(lobbyBox.x + 10, lobbyBox.y + 24, lobbyBox.x + lobbyBox.width - 10, lobbyBox.y + 24,
             Color{30, 45, 70, 255});

    if (isHosting) {
        auto lobbyPlayers = net.GetServer().GetLobbyPlayers();
        float py = lobbyBox.y + 32;

        for (size_t i = 0; i < lobbyPlayers.size(); ++i) {
            const auto& lp = lobbyPlayers[i];
            Color dotCol = (lp.id == 1) ? Color{40, 140, 250, 255} : Color{230, 45, 50, 255};
            DrawCircle(lobbyBox.x + 20, py + 8, 5.0f, dotCol);

            char pDesc[128];
            if (lp.isHost) {
                std::snprintf(pDesc, sizeof(pDesc), "Slot %u: %s (Host) [READY]", lp.id,
                              lp.name.c_str());
            } else {
                std::snprintf(pDesc, sizeof(pDesc), "Slot %u: %s (Guest) [READY]", lp.id,
                              lp.name.c_str());
            }
            DrawTextCustom(pDesc, lobbyBox.x + 35, py + 2, 13, WHITE);
            py += 22;
        }

        // Show remaining slots as AI
        const char* aiSlots[] = {"Aethelgard (AI Nation)", "Novaria (AI Nation)",
                                 "Valerius (AI Nation)", "Zephyria (AI Nation)"};
        for (size_t s = lobbyPlayers.size(); s < 4; ++s) {
            DrawCircle(lobbyBox.x + 20, py + 8, 4.0f, GRAY);
            char aiDesc[64];
            std::snprintf(aiDesc, sizeof(aiDesc), "Slot %zu: %s", s + 1, aiSlots[s % 4]);
            DrawTextCustom(aiDesc, lobbyBox.x + 35, py + 2, 12, Color{130, 140, 160, 255});
            py += 20;
        }

        char countBuf[64];
        std::snprintf(countBuf, sizeof(countBuf), "Total Humans Connected: %zu",
                      lobbyPlayers.size());
        DrawTextCustom(countBuf, lobbyBox.x + 12, lobbyBox.y + lobbyBox.height - 20, 12,
                       Color{100, 220, 140, 255});
    } else {
        DrawTextCustom("Click [CREATE ROOM & LISTEN] to begin hosting.", lobbyBox.x + 25,
                       lobbyBox.y + 70, 13, GRAY);
    }

    // Action button: [START MATCH]
    Rectangle actionBtn = {menuX + 30, menuY + 380, menuW - 60, 50};
    bool hoverAction = CheckCollisionPointRec(mousePos, actionBtn);

    if (isHosting) {
        DrawRectangleRec(actionBtn,
                         hoverAction ? Color{40, 175, 95, 255} : Color{30, 140, 75, 235});
        DrawRectangleLinesEx(actionBtn, 2.0f, hoverAction ? WHITE : Color{80, 210, 120, 255});
        const char* startLabel = "⚔  START MATCH (SPACE / ENTER)  ⚔";
        int slw = MeasureText(startLabel, 16);
        DrawTextCustom(startLabel, actionBtn.x + (actionBtn.width - slw) * 0.5f, actionBtn.y + 16,
                       16, WHITE);

        if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverAction) ||
            ((controller_ && controller_->isActionPressed("CONFIRM")) && activeInputBox_ == 0)) {
            PlaySoundEffect("ui_click");
            net.StartHostMatch(sim);
            Init(sim, 1);
            uiState_ = UIState::InGame;
        }
    } else {
        DrawRectangleRec(actionBtn, Color{30, 35, 45, 180});
        DrawRectangleLinesEx(actionBtn, 1.0f, Color{60, 70, 85, 255});
        const char* waitLabel = "ROOM NOT ACTIVE";
        int wlw = MeasureText(waitLabel, 14);
        DrawTextCustom(waitLabel, actionBtn.x + (actionBtn.width - wlw) * 0.5f, actionBtn.y + 17,
                       14, GRAY);
    }

    // Back button
    Rectangle backBtn = {menuX + 30, menuY + 448, menuW - 60, 38};
    bool hoverBack = CheckCollisionPointRec(mousePos, backBtn);
    DrawRectangleRec(backBtn, hoverBack ? Color{50, 60, 80, 220} : Color{28, 35, 48, 200});
    DrawRectangleLinesEx(backBtn, 1.0f, hoverBack ? WHITE : Color{55, 70, 95, 255});
    int bw = MeasureText("BACK TO MAIN MENU", 14);
    DrawTextCustom("BACK TO MAIN MENU", backBtn.x + (backBtn.width - bw) * 0.5f, backBtn.y + 12, 14,
                   GRAY);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverBack) {
        PlaySoundEffect("ui_click");
        net.Stop();
        uiState_ = UIState::StartMenu;
    }
}

void Renderer::DrawJoinLobby(Simulation& sim, NetworkManager& net) {
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Color{5, 10, 20, 210});

    float menuW = 540.0f;
    float menuH = 520.0f;
    float menuX = (screenWidth_ - menuW) * 0.5f;
    float menuY = (screenHeight_ - menuH) * 0.5f;

    Rectangle card = {menuX, menuY, menuW, menuH};
    DrawRectangleRec(card, Color{18, 24, 38, 248});
    DrawRectangleLinesEx(card, 2.0f, Color{40, 140, 250, 255});

    DrawTextCustom("JOIN MULTIPLAYER ROOM", menuX + 30, menuY + 22, 22, GOLD);
    DrawLine(menuX + 25, menuY + 54, menuX + menuW - 25, menuY + 54, Color{40, 60, 90, 255});

    Vector2 mousePos = GetMousePosition();
    bool isConnected = net.GetClient().IsConnected();
    bool isInLobby = net.GetClient().IsInLobby();
    PollRoomScan();

    // Player Name
    DrawTextCustom("Your Commander Name:", menuX + 30, menuY + 68, 13, WHITE);
    Rectangle nameBox = {menuX + 30, menuY + 86, menuW - 60, 30};
    DrawRectangleRec(nameBox,
                     (activeInputBox_ == 1) ? Color{35, 55, 85, 255} : Color{25, 35, 52, 255});
    DrawRectangleLinesEx(nameBox, 1.0f, (activeInputBox_ == 1) ? GOLD : Color{60, 80, 110, 255});
    DrawTextCustom(playerNameBuffer_, nameBox.x + 10, nameBox.y + 7, 14, WHITE);
    if (!isConnected && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mousePos, nameBox)) {
        activeInputBox_ = 1;
    }

    // Server IP & Port
    DrawTextCustom("Host IP Address:", menuX + 30, menuY + 124, 13, WHITE);
    Rectangle ipBox = {menuX + 30, menuY + 142, 260, 30};
    DrawRectangleRec(ipBox,
                     (activeInputBox_ == 3) ? Color{35, 55, 85, 255} : Color{25, 35, 52, 255});
    DrawRectangleLinesEx(ipBox, 1.0f, (activeInputBox_ == 3) ? GOLD : Color{60, 80, 110, 255});
    DrawTextCustom(joinIpBuffer_, ipBox.x + 10, ipBox.y + 7, 14, WHITE);
    if (!isConnected && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mousePos, ipBox)) {
        activeInputBox_ = 3;
    }

    DrawTextCustom("Port:", menuX + 310, menuY + 124, 13, WHITE);
    Rectangle portBox = {menuX + 310, menuY + 142, 170, 30};
    DrawRectangleRec(portBox,
                     (activeInputBox_ == 4) ? Color{35, 55, 85, 255} : Color{25, 35, 52, 255});
    DrawRectangleLinesEx(portBox, 1.0f, (activeInputBox_ == 4) ? GOLD : Color{60, 80, 110, 255});
    DrawTextCustom(joinPortBuffer_, portBox.x + 10, portBox.y + 7, 14, WHITE);
    if (!isConnected && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(mousePos, portBox)) {
        activeInputBox_ = 4;
    }

    // Lobby / Status box
    Rectangle statusBox = {menuX + 30, menuY + 185, menuW - 60, 175};
    DrawRectangleRec(statusBox, Color{12, 16, 26, 220});
    DrawRectangleLinesEx(statusBox, 1.0f, Color{45, 60, 85, 255});

    if (isConnected) {
        DrawTextCustom("CONNECTED TO SERVER LOBBY:", statusBox.x + 12, statusBox.y + 8, 12, GREEN);
        DrawLine(statusBox.x + 10, statusBox.y + 24, statusBox.x + statusBox.width - 10,
                 statusBox.y + 24, Color{30, 45, 70, 255});

        const auto& lobbyPlayers = net.GetClient().GetLobbyPlayers();
        float py = statusBox.y + 32;

        if (!lobbyPlayers.empty()) {
            for (const auto& lp : lobbyPlayers) {
                Color dotCol = (lp.id == net.GetLocalPlayerId()) ? Color{230, 45, 50, 255}
                                                                 : Color{40, 140, 250, 255};
                DrawCircle(statusBox.x + 20, py + 8, 5.0f, dotCol);

                char pDesc[128];
                if (lp.id == net.GetLocalPlayerId()) {
                    std::snprintf(pDesc, sizeof(pDesc), "Slot %u: %s (You) [READY]", lp.id,
                                  lp.name.c_str());
                } else if (lp.isHost) {
                    std::snprintf(pDesc, sizeof(pDesc), "Slot %u: %s (Host) [READY]", lp.id,
                                  lp.name.c_str());
                } else {
                    std::snprintf(pDesc, sizeof(pDesc), "Slot %u: %s (Guest) [READY]", lp.id,
                                  lp.name.c_str());
                }
                DrawTextCustom(pDesc, statusBox.x + 35, py + 2, 13, WHITE);
                py += 22;
            }
        } else {
            char selfDesc[128];
            std::snprintf(selfDesc, sizeof(selfDesc), "Slot %u: %s (You) [CONNECTED]",
                          net.GetLocalPlayerId(), playerNameBuffer_);
            DrawTextCustom(selfDesc, statusBox.x + 20, py + 4, 13, WHITE);
        }

        // Animated pulsing wait text
        float time = static_cast<float>(GetTime());
        float pulseAlpha = 0.5f + 0.5f * std::sin(time * 4.0f);
        Color pulseCol = Color{255, 220, 50, static_cast<unsigned char>(pulseAlpha * 255)};
        const char* waitText = ">>> WAITING FOR HOST TO START MATCH... <<<";
        int wtw = MeasureText(waitText, 14);
        DrawTextCustom(waitText, statusBox.x + (statusBox.width - wtw) * 0.5f,
                       statusBox.y + statusBox.height - 30, 14, pulseCol);
    } else {
        DrawTextCustom("STATUS: NOT CONNECTED", statusBox.x + 12, statusBox.y + 8, 12, GRAY);
        DrawLine(statusBox.x + 10, statusBox.y + 24, statusBox.x + statusBox.width - 10,
                 statusBox.y + 24, Color{30, 45, 70, 255});

        if (scanningRooms_) {
            float time = static_cast<float>(GetTime());
            std::string dots = std::string(1 + (static_cast<int>(time * 3.0f) % 4), '.');
            DrawTextCustom(("Scanning dedicated server rooms" + dots).c_str(), statusBox.x + 12,
                           statusBox.y + 40, 14, Color{120, 200, 255, 255});
        } else if (joinRooms_.empty()) {
            DrawTextCustom("Enter server IP and base port, then click [SCAN ROOMS].",
                           statusBox.x + 25, statusBox.y + 40, 13, Color{160, 180, 210, 255});
            DrawTextCustom(connectionStatusText_.c_str(), statusBox.x + 25, statusBox.y + 90, 13,
                           Color{230, 180, 60, 255});
        } else {
            DrawTextCustom("AVAILABLE SERVER ROOMS:", statusBox.x + 12, statusBox.y + 10, 12,
                           Color{120, 200, 255, 255});
            float rowY = statusBox.y + 32;
            for (auto& room : joinRooms_) {
                if (!room.reachable) {
                    continue;
                }
                Rectangle rowRect = {statusBox.x + 8, rowY - 2, statusBox.width - 16, 20};
                bool hover = CheckCollisionPointRec(mousePos, rowRect);
                if (hover) {
                    DrawRectangleRec(rowRect, Color{40, 80, 140, 120});
                }

                char title[96];
                std::snprintf(title, sizeof(title), "%s", room.info.name.c_str());
                char sub[96];
                std::snprintf(sub, sizeof(sub), "%s  %d/%d players", MapTypeName(room.info.mapType),
                              room.info.players, room.info.maxPlayers);
                const char* state = room.info.gameStarted ? "IN GAME" : "IN LOBBY";

                DrawTextCustom(title, statusBox.x + 16, rowY, 10, WHITE);
                DrawTextCustom(sub, statusBox.x + 180, rowY, 10, Color{160, 190, 220, 255});
                DrawTextCustom(state, statusBox.x + 400, rowY, 10,
                               room.info.gameStarted ? Color{255, 200, 60, 255}
                                                     : Color{110, 220, 140, 255});

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                    CheckCollisionPointRec(mousePos, rowRect)) {
                    JoinRoom(sim, net, room.port);
                }
                rowY += 22.0f;
            }
            if (rowY < statusBox.y + statusBox.height - 6) {
                connectionStatusText_ = (connectionStatusText_ == "Ready to connect")
                                            ? "Click a room to join it as a faction."
                                            : connectionStatusText_;
                DrawTextCustom(connectionStatusText_.c_str(), statusBox.x + 25,
                               statusBox.y + statusBox.height - 26, 12, Color{230, 180, 60, 255});
            }
        }
    }

    Rectangle connectBtn = {menuX + menuW / 2 - 20, menuY + 380, menuW / 2 - 10, 50};
    Rectangle scanBtn = {menuX + 30, menuY + 380, menuW / 2 - 20, 50};
    bool hoverConnect = CheckCollisionPointRec(mousePos, connectBtn);
    bool hoverScan = CheckCollisionPointRec(mousePos, scanBtn);

    if (!isConnected) {
        DrawRectangleRec(scanBtn, hoverScan ? Color{90, 110, 160, 255} : Color{60, 80, 110, 239});
        DrawRectangleLinesEx(scanBtn, 2.0f, hoverScan ? WHITE : Color{100, 120, 160, 255});
        const char* scanLabel = "SCAN ROOMS";
        int slw = MeasureText(scanLabel, 16);
        DrawTextCustom(scanLabel, scanBtn.x + (scanBtn.width - slw) * 0.5f, scanBtn.y + 16, 16,
                       WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverScan && !scanningRooms_) {
            BeginRoomScan(net);
        }

        DrawRectangleRec(connectBtn,
                         hoverConnect ? Color{40, 140, 240, 255} : Color{30, 100, 200, 235});
        DrawRectangleLinesEx(connectBtn, 2.0f, hoverConnect ? WHITE : Color{70, 160, 255, 255});
        const char* connLabel = "CONNECT";
        int clw = MeasureText(connLabel, 16);
        DrawTextCustom(connLabel, connectBtn.x + (connectBtn.width - clw) * 0.5f, connectBtn.y + 16,
                       16, WHITE);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverConnect) {
            int port = std::atoi(joinPortBuffer_);
            if (port <= 0) {
                port = 7777;
            }
            JoinRoom(sim, net, static_cast<uint16_t>(port));
        }
    } else {
        DrawRectangleRec(connectBtn, Color{25, 35, 50, 180});
        DrawRectangleLinesEx(connectBtn, 1.0f, Color{50, 80, 120, 255});
        const char* connLabel = "● CONNECTED (WAITING FOR LAUNCH)";
        int clw = MeasureText(connLabel, 14);
        DrawTextCustom(connLabel, connectBtn.x + (connectBtn.width - clw) * 0.5f, connectBtn.y + 17,
                       14, GREEN);
    }

    // Back / Disconnect button
    Rectangle backBtn = {menuX + 30, menuY + 448, menuW - 60, 38};
    bool hoverBack = CheckCollisionPointRec(mousePos, backBtn);
    DrawRectangleRec(backBtn, hoverBack ? Color{50, 60, 80, 220} : Color{28, 35, 48, 200});
    DrawRectangleLinesEx(backBtn, 1.0f, hoverBack ? WHITE : Color{55, 70, 95, 255});

    const char* backText = isConnected ? "DISCONNECT & RETURN TO MENU" : "BACK TO MAIN MENU";
    int bw = MeasureText(backText, 14);
    DrawTextCustom(backText, backBtn.x + (backBtn.width - bw) * 0.5f, backBtn.y + 12, 14, GRAY);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverBack) {
        PlaySoundEffect("ui_click");
        net.Stop();
        connectionStatusText_ = "Ready to connect";
        uiState_ = UIState::StartMenu;
    }
}

void Renderer::PollRoomScan() {
    if (roomScanFuture_.valid()) {
        auto status = roomScanFuture_.wait_for(std::chrono::seconds(0));
        if (status == std::future_status::ready) {
            joinRooms_ = roomScanFuture_.get();
            scanningRooms_ = false;
        }
    }
}

void Renderer::BeginRoomScan(NetworkManager& net) {
    if (scanningRooms_) {
        return;
    }
    PlaySoundEffect("ui_click");
    scanningRooms_ = true;
    joinRooms_.clear();
    connectionStatusText_ = "Ready to connect";

    int port = std::atoi(joinPortBuffer_);
    if (port <= 0) {
        port = 7777;
    }
    std::string ip = joinIpBuffer_;

    roomScanFuture_ = std::async(std::launch::async, [ip, port]() {
        return NetworkClient::ScanRooms(ip, static_cast<uint16_t>(port), 8, 300.0f);
    });
}

void Renderer::JoinRoom(Simulation& sim, NetworkManager& net, uint16_t port) {
    std::snprintf(joinPortBuffer_, sizeof(joinPortBuffer_), "%u", static_cast<unsigned>(port));
    connectionStatusText_ = "Connecting to room...";
    if (!net.StartClient(sim, joinIpBuffer_, port, playerNameBuffer_)) {
        connectionStatusText_ = "Connection failed";
        return;
    }
    PlaySoundEffect("ui_click");
    net.GetClient().SetOnGameStartedCallback([this, &sim, &net]() {
        Init(sim, net.GetLocalPlayerId());
        uiState_ = UIState::InGame;
    });
}

void Renderer::DrawHowToPlay() {
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Color{5, 10, 20, 230});

    float menuW = 760.0f;
    float menuH = 580.0f;
    float menuX = (screenWidth_ - menuW) * 0.5f;
    float menuY = (screenHeight_ - menuH) * 0.5f;

    Rectangle card = {menuX, menuY, menuW, menuH};
    DrawRectangleRec(card, Color{18, 24, 38, 250});
    DrawRectangleLinesEx(card, 2.0f, Color{40, 140, 250, 255});

    DrawTextCustom("HOW TO PLAY OPENFRONT", menuX + 30, menuY + 24, 24, GOLD);
    DrawLine(menuX + 25, menuY + 60, menuX + menuW - 25, menuY + 60, Color{40, 60, 90, 255});

    int y = menuY + 75;

    DrawTextCustom("1. BORDER-DRIVEN EXPANSION (NO DIRECT UNIT MOVEMENT)", menuX + 30, y, 16,
                   Color{120, 220, 140, 255});
    y += 24;
    DrawTextCustom("• Left-click ANY tile outside your borders to launch an expanding front wave.",
                   menuX + 45, y, 14, WHITE);
    y += 20;
    DrawTextCustom(
        "• Your committed army flows outward across your border, capturing tiles one by one.",
        menuX + 45, y, 14, WHITE);
    y += 20;
    DrawTextCustom("• Left-click an active front badge to REINFORCE it; Right-click to CANCEL.",
                   menuX + 45, y, 14, WHITE);

    y += 35;
    DrawTextCustom("2. ARMY COMMITMENT & COMBAT SLIDER", menuX + 30, y, 16,
                   Color{255, 215, 50, 255});
    y += 24;
    DrawTextCustom(
        "• Choose how much of your total army is sent into each push: [Z] 10%, [X] 20%, [C] "
        "50%, [V] 100%.",
        menuX + 45, y, 14, WHITE);
    y += 20;
    DrawTextCustom("• Defender's shield is their troop density (troops / tiles owned * 10).",
                   menuX + 45, y, 14, WHITE);
    y += 20;
    DrawTextCustom(
        "• Terrain matters: Plains (1.0x), Highlands (1.5x), Mountains (2.5x defense multiplier).",
        menuX + 45, y, 14, WHITE);

    y += 35;
    DrawTextCustom("3. ECONOMY & BUILDINGS", menuX + 30, y, 16, Color{100, 190, 255, 255});
    y += 24;
    DrawTextCustom("• [1] City ($40k): Boosts max population capacity by 25,000 per level.",
                   menuX + 45, y, 14, WHITE);
    y += 20;
    DrawTextCustom(
        "• [2] Port ($35k) & [3] Factory ($30k): Coastal trade ships and trains generate "
        "massive gold income.",
        menuX + 45, y, 14, WHITE);
    y += 20;
    DrawTextCustom(
        "• [4] Defense Post ($25k): Fortifies territory with a 5x attacker casualty aura "
        "within 30 tiles!",
        menuX + 45, y, 14, WHITE);
    y += 20;
    DrawTextCustom(
        "• [5] SAM ($45k) & [6] Silo ($75k): Intercept and launch devastating nuclear weapons.",
        menuX + 45, y, 14, WHITE);

    // Back button
    Rectangle backBtn = {menuX + 30, menuY + menuH - 60, menuW - 60, 40};
    bool hoverBack = CheckCollisionPointRec(GetMousePosition(), backBtn);
    DrawRectangleRec(backBtn, hoverBack ? Color{40, 140, 240, 240} : Color{30, 80, 160, 220});
    DrawRectangleLinesEx(backBtn, 1.5f, hoverBack ? WHITE : Color{70, 140, 240, 255});
    int bw = MeasureText("BACK TO MAIN MENU", 15);
    DrawTextCustom("BACK TO MAIN MENU", backBtn.x + (backBtn.width - bw) * 0.5f, backBtn.y + 12, 15,
                   WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverBack) {
        uiState_ = UIState::StartMenu;
    }
}

void Renderer::DrawPauseMenu(Simulation& sim, NetworkManager& net) {
    DrawRectangle(0, 0, screenWidth_, screenHeight_, Color{5, 10, 20, 180});

    float menuW = 340.0f;
    float menuH = 320.0f;
    float menuX = (screenWidth_ - menuW) * 0.5f;
    float menuY = (screenHeight_ - menuH) * 0.5f;

    Rectangle card = {menuX, menuY, menuW, menuH};
    DrawRectangleRec(card, Color{18, 24, 38, 245});
    DrawRectangleLinesEx(card, 2.0f, Color{40, 140, 250, 255});

    DrawTextCustom("PAUSE MENU", menuX + 105, menuY + 24, 22, GOLD);
    DrawLine(menuX + 25, menuY + 56, menuX + menuW - 25, menuY + 56, Color{40, 60, 90, 255});

    Vector2 mousePos = GetMousePosition();

    Rectangle resBtn = {menuX + 30, menuY + 75, menuW - 60, 42};
    bool hoverRes = CheckCollisionPointRec(mousePos, resBtn);
    DrawRectangleRec(resBtn, hoverRes ? Color{40, 150, 90, 240} : Color{30, 120, 70, 220});
    DrawRectangleLinesEx(resBtn, 1.0f, hoverRes ? WHITE : Color{60, 180, 100, 255});
    DrawTextCustom("RESUME BATTLE", resBtn.x + 65, resBtn.y + 13, 15, WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverRes) {
        PlaySoundEffect("ui_click");
        uiState_ = UIState::InGame;
    }

    Rectangle restartBtn = {menuX + 30, menuY + 130, menuW - 60, 42};
    bool hoverRest = CheckCollisionPointRec(mousePos, restartBtn);
    DrawRectangleRec(restartBtn, hoverRest ? Color{40, 100, 180, 240} : Color{30, 75, 140, 220});
    DrawRectangleLinesEx(restartBtn, 1.0f, hoverRest ? WHITE : Color{60, 140, 220, 255});
    DrawTextCustom("RESTART MATCH", restartBtn.x + 65, restartBtn.y + 13, 15, WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverRest) {
        PlaySoundEffect("ui_click");
        net.StartSinglePlayer(sim);
        Init(sim, 1);
        uiState_ = UIState::InGame;
    }

    Rectangle saveBtn = {menuX + 30, menuY + 185, (menuW - 60) / 2 - 5, 42};
    bool hoverSave = CheckCollisionPointRec(mousePos, saveBtn);
    DrawRectangleRec(saveBtn, hoverSave ? Color{40, 100, 180, 240} : Color{30, 75, 140, 220});
    DrawRectangleLinesEx(saveBtn, 1.0f, hoverSave ? WHITE : Color{60, 140, 220, 255});
    DrawTextCustom("SAVE", saveBtn.x + 25, saveBtn.y + 13, 15, WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverSave) {
        PlaySoundEffect("ui_click");
        if (sim.SaveGame("savegame.txt")) {
            connectionStatusText_ = "Game Saved!";
        }
    }

    Rectangle loadBtn = {menuX + 30 + (menuW - 60) / 2 + 5, menuY + 185, (menuW - 60) / 2 - 5, 42};
    bool hoverLoad = CheckCollisionPointRec(mousePos, loadBtn);
    DrawRectangleRec(loadBtn, hoverLoad ? Color{40, 100, 180, 240} : Color{30, 75, 140, 220});
    DrawRectangleLinesEx(loadBtn, 1.0f, hoverLoad ? WHITE : Color{60, 140, 220, 255});
    DrawTextCustom("LOAD", loadBtn.x + 25, loadBtn.y + 13, 15, WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverLoad) {
        PlaySoundEffect("ui_click");
        if (sim.LoadGame("savegame.txt")) {
            Init(sim, 1);
            uiState_ = UIState::InGame;
        }
    }

    Rectangle mmBtn = {menuX + 30, menuY + 240, menuW - 60, 42};
    bool hoverMM = CheckCollisionPointRec(mousePos, mmBtn);
    DrawRectangleRec(mmBtn, hoverMM ? Color{180, 40, 50, 240} : Color{130, 30, 40, 220});
    DrawRectangleLinesEx(mmBtn, 1.0f, hoverMM ? WHITE : Color{220, 60, 70, 255});
    DrawTextCustom("MAIN MENU", mmBtn.x + 85, mmBtn.y + 13, 15, WHITE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoverMM) {
        PlaySoundEffect("ui_click");
        net.Stop();
        uiState_ = UIState::StartMenu;
    }
}

} // namespace Openfront
