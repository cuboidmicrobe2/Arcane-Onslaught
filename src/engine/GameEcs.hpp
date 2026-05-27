#pragma once

#include <string>
#include <entt/entt.hpp>
#include <raylib.h>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

struct lua_State;

namespace GameEcs
{
    struct SpellTuning
    {
        int projectileCount = 1;
        float projectileSpeed = 320.0f;
        float projectileSize = 10.0f;
        bool ricochet = false;
        int damage = 18;
        Color projectileColor = Color{255, 210, 86, 255}; // Default golden yellow
    };

    struct Position
    {
        Vector2 value{};
    };

    struct Velocity
    {
        Vector2 value{};
    };

    struct Rotation
    {
        float angle = 0.0f; // In radians
    };

    struct CircleCollider
    {
        float radius = 10.0f;
    };

    struct RectCollider
    {
        float width = 50.0f;
        float height = 50.0f;
    };

    struct Health
    {
        int hp = 3;
    };

    struct PlayerTag
    {
    };

    struct EnemyTag
    {
    };

    struct WallTag
    {
    };

    struct DamageCooldown
    {
        float seconds = 0.0f;
    };

    struct IFrames
    {
        float seconds = 0.0f; // Duration of invincibility frames remaining
    };

    struct ProjectileTag
    {
    };

    struct ProjectileColor
    {
        Color value = Color{255, 210, 86, 255}; // Default golden yellow
    };

    struct ProjectileDamage
    {
        int value = 1;
    };

    struct ProjectileRicochet
    {
        int bouncesLeft = 0;
    };

    struct Behavior
    {
        int coroutineRef = LUA_NOREF; // Lua registry reference to co-routine
    };

    struct FrameContext
    {
        float dt = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    bool InitializeWorld(entt::registry &registry, lua_State *L);
    void SetSpellTuning(entt::registry &registry, const SpellTuning &tuning);

    // Exposed for entity spawning via Lua
    entt::entity ImportEntityFromTable(entt::registry &registry, lua_State *L, int entityIndex);
    void UpdateBehaviors(entt::registry &registry, lua_State *L, const FrameContext &frame);
    void UpdatePlayers(entt::registry &registry, const FrameContext &frame);
    void SpawnPlayerProjectiles(entt::registry &registry, const FrameContext &frame);
    void UpdateProjectiles(entt::registry &registry, const FrameContext &frame);
    void ResolveProjectileEnemyCollisions(entt::registry &registry);
    void ResolveProjectileWallCollisions(entt::registry &registry);
    bool HasDefeatedPlayer(entt::registry &registry);
    void DrawDefeatOverlay(const FrameContext &frame);
    bool IsAnyReturnInputPressed();
    void UpdateEnemies(entt::registry &registry, const FrameContext &frame);
    void UpdateIFrames(entt::registry &registry, const FrameContext &frame);
    void ResolveEnemyPlayerCollisions(entt::registry &registry, lua_State *L);
    void DrawEnemies(entt::registry &registry);
    void DrawProjectiles(entt::registry &registry);
    void DrawWalls(entt::registry &registry);
    void DrawPlayersAndHud(entt::registry &registry);
} // namespace GameEcs
