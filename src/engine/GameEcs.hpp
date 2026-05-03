#pragma once

#include <string>
#include <entt/entt.hpp>
#include <raylib.h>

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
    };

    struct Position
    {
        Vector2 value{};
    };

    struct Velocity
    {
        Vector2 value{};
    };

    struct CircleCollider
    {
        float radius = 10.0f;
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

    struct DamageCooldown
    {
        float seconds = 0.0f;
    };

    struct ProjectileTag
    {
    };

    struct ProjectileDamage
    {
        int value = 1;
    };

    struct ProjectileRicochet
    {
        int bouncesLeft = 0;
    };

    struct FrameContext
    {
        float dt = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    bool InitializeWorld(entt::registry &registry, lua_State *L);
    void SetSpellTuning(entt::registry &registry, const SpellTuning &tuning);
    void UpdatePlayers(entt::registry &registry, const FrameContext &frame);
    void SpawnPlayerProjectiles(entt::registry &registry, const FrameContext &frame);
    void UpdateProjectiles(entt::registry &registry, const FrameContext &frame);
    void ResolveProjectileEnemyCollisions(entt::registry &registry);
    bool HasDefeatedPlayer(entt::registry &registry);
    void DrawDefeatOverlay(const FrameContext &frame);
    bool IsAnyReturnInputPressed();
    void UpdateEnemies(entt::registry &registry, const FrameContext &frame);
    void ResolveEnemyPlayerCollisions(entt::registry &registry);
    void DrawEnemies(entt::registry &registry);
    void DrawProjectiles(entt::registry &registry);
    void DrawPlayersAndHud(entt::registry &registry);
} // namespace GameEcs
