#include "engine/GameEcs.hpp"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace
{
    struct FireCooldown
    {
        float seconds = 0.0f;
    };

    constexpr float kMaxPlayerFireRate = 0.14f;
    constexpr float kDefaultSpreadRadians = 0.44f;

    Vector2 NormalizeOrFallback(Vector2 value, Vector2 fallback)
    {
        const float lengthSq = (value.x * value.x) + (value.y * value.y);
        if (lengthSq <= 0.0001f)
        {
            return fallback;
        }

        const float invLength = 1.0f / std::sqrt(lengthSq);
        return Vector2{value.x * invLength, value.y * invLength};
    }

    Vector2 RotateVector(Vector2 v, float radians)
    {
        const float c = std::cos(radians);
        const float s = std::sin(radians);
        return Vector2{(v.x * c) - (v.y * s), (v.x * s) + (v.y * c)};
    }

    bool ReadVector2Field(lua_State *L, int index, Vector2 &value)
    {
        const int tableIndex = lua_absindex(L, index);
        if (!lua_istable(L, tableIndex))
        {
            return false;
        }

        lua_getfield(L, tableIndex, "x");
        lua_getfield(L, tableIndex, "y");
        const bool hasX = lua_isnumber(L, -2) != 0;
        const bool hasY = lua_isnumber(L, -1) != 0;
        if (hasX && hasY)
        {
            value.x = static_cast<float>(lua_tonumber(L, -2));
            value.y = static_cast<float>(lua_tonumber(L, -1));
            lua_pop(L, 2);
            return true;
        }

        lua_pop(L, 2);
        return false;
    }

    bool ReadFloatField(lua_State *L, int index, const char *field, float &value)
    {
        lua_getfield(L, index, field);
        const bool ok = lua_isnumber(L, -1) != 0;
        if (ok)
        {
            value = static_cast<float>(lua_tonumber(L, -1));
        }
        lua_pop(L, 1);
        return ok;
    }

    bool ReadIntField(lua_State *L, int index, const char *field, int &value)
    {
        lua_getfield(L, index, field);
        const bool ok = lua_isnumber(L, -1) != 0;
        if (ok)
        {
            value = static_cast<int>(lua_tointeger(L, -1));
        }
        lua_pop(L, 1);
        return ok;
    }

    bool ReadBoolField(lua_State *L, int index, const char *field, bool &value)
    {
        lua_getfield(L, index, field);
        const bool ok = lua_isboolean(L, -1) != 0;
        if (ok)
        {
            value = lua_toboolean(L, -1) != 0;
        }
        lua_pop(L, 1);
        return ok;
    }

    bool ImportEntityFromTable(entt::registry &registry, lua_State *L, int entityIndex)
    {
        const int tableIndex = lua_absindex(L, entityIndex);
        if (!lua_istable(L, tableIndex))
        {
            return false;
        }

        const entt::entity entity = registry.create();

        Vector2 position{};
        if (lua_getfield(L, tableIndex, "position") == LUA_TTABLE)
        {
            if (ReadVector2Field(L, -1, position))
            {
                registry.emplace<GameEcs::Position>(entity, position);
            }
        }
        lua_pop(L, 1);

        Vector2 velocity{};
        if (lua_getfield(L, tableIndex, "velocity") == LUA_TTABLE)
        {
            if (ReadVector2Field(L, -1, velocity))
            {
                registry.emplace<GameEcs::Velocity>(entity, velocity);
            }
        }
        lua_pop(L, 1);

        float radius = 0.0f;
        if (lua_getfield(L, tableIndex, "circle_collider") == LUA_TTABLE)
        {
            if (ReadFloatField(L, -1, "radius", radius))
            {
                registry.emplace<GameEcs::CircleCollider>(entity, radius);
            }
        }
        lua_pop(L, 1);

        int hp = 0;
        if (lua_getfield(L, tableIndex, "health") == LUA_TTABLE)
        {
            if (ReadIntField(L, -1, "hp", hp))
            {
                registry.emplace<GameEcs::Health>(entity, hp);
            }
        }
        lua_pop(L, 1);

        float cooldownSeconds = 0.0f;
        if (lua_getfield(L, tableIndex, "damage_cooldown") == LUA_TTABLE)
        {
            if (ReadFloatField(L, -1, "seconds", cooldownSeconds))
            {
                registry.emplace<GameEcs::DamageCooldown>(entity, cooldownSeconds);
            }
        }
        lua_pop(L, 1);

        bool flag = false;
        if (ReadBoolField(L, tableIndex, "player_tag", flag) && flag)
        {
            registry.emplace<GameEcs::PlayerTag>(entity);
        }
        if (ReadBoolField(L, tableIndex, "enemy_tag", flag) && flag)
        {
            registry.emplace<GameEcs::EnemyTag>(entity);
        }
        if (ReadBoolField(L, tableIndex, "projectile_tag", flag) && flag)
        {
            registry.emplace<GameEcs::ProjectileTag>(entity);
        }

        int damage = 0;
        if (lua_getfield(L, tableIndex, "projectile_damage") == LUA_TTABLE)
        {
            if (ReadIntField(L, -1, "value", damage))
            {
                registry.emplace<GameEcs::ProjectileDamage>(entity, damage);
            }
        }
        lua_pop(L, 1);

        int bounces = 0;
        if (lua_getfield(L, tableIndex, "projectile_ricochet") == LUA_TTABLE)
        {
            if (ReadIntField(L, -1, "bounces_left", bounces))
            {
                registry.emplace<GameEcs::ProjectileRicochet>(entity, bounces);
            }
        }
        lua_pop(L, 1);

        return true;
    }
} // namespace

namespace GameEcs
{
    namespace
    {
        Vector2 ReadPlayerMoveInput()
        {
            Vector2 input{0.0f, 0.0f};
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            {
                input.x -= 1.0f;
            }
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            {
                input.x += 1.0f;
            }
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
            {
                input.y -= 1.0f;
            }
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
            {
                input.y += 1.0f;
            }

            const float inputLengthSquared = (input.x * input.x) + (input.y * input.y);
            if (inputLengthSquared > 0.0001f)
            {
                const float invLength = 1.0f / std::sqrt(inputLengthSquared);
                input.x *= invLength;
                input.y *= invLength;
            }

            return input;
        }

        bool IsAnyGamepadButtonPressed()
        {
            for (int gamepad = 0; gamepad < 4; ++gamepad)
            {
                if (!IsGamepadAvailable(gamepad))
                {
                    continue;
                }

                if (IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_FACE_UP) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_FACE_DOWN) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_FACE_LEFT) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_UP) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_LEFT) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_MIDDLE_LEFT) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_1) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_2) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1) ||
                    IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_2))
                {
                    return true;
                }
            }

            return false;
        }

        void SpawnProjectileEntity(entt::registry &registry, Vector2 pos, Vector2 direction, const SpellTuning &tuning)
        {
            const entt::entity projectile = registry.create();
            registry.emplace<ProjectileTag>(projectile);
            registry.emplace<Position>(projectile, pos);
            registry.emplace<Velocity>(projectile, Vector2{direction.x * tuning.projectileSpeed, direction.y * tuning.projectileSpeed});
            registry.emplace<CircleCollider>(projectile, tuning.projectileSize);
            registry.emplace<ProjectileDamage>(projectile, std::max(1, tuning.damage));
            registry.emplace<ProjectileRicochet>(projectile, tuning.ricochet ? 3 : 0);
        }
    } // namespace

    bool InitializeWorld(entt::registry &registry, lua_State *L)
    {
        if (L == nullptr || !lua_istable(L, -1))
        {
            return false;
        }

        registry.clear();
        registry.ctx().insert_or_assign<SpellTuning>(SpellTuning{});
        registry.ctx().insert_or_assign<FireCooldown>(FireCooldown{});

        lua_getfield(L, -1, "ecs");
        if (lua_istable(L, -1))
        {
            const int ecsIndex = lua_absindex(L, -1);
            lua_getfield(L, ecsIndex, "entities");
            if (lua_istable(L, -1))
            {
                const int entitiesIndex = lua_absindex(L, -1);
                const int entityCount = static_cast<int>(lua_rawlen(L, -1));
                for (int i = 1; i <= entityCount; ++i)
                {
                    lua_rawgeti(L, entitiesIndex, i);
                    ImportEntityFromTable(registry, L, -1);
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        return true;
    }

    void SetSpellTuning(entt::registry &registry, const SpellTuning &tuning)
    {
        SpellTuning clamped = tuning;
        clamped.projectileCount = std::clamp(clamped.projectileCount, 1, 12);
        clamped.projectileSpeed = std::clamp(clamped.projectileSpeed, 120.0f, 900.0f);
        clamped.projectileSize = std::clamp(clamped.projectileSize, 4.0f, 36.0f);
        clamped.damage = std::max(1, clamped.damage);
        registry.ctx().insert_or_assign<SpellTuning>(std::move(clamped));
    }

    void UpdatePlayers(entt::registry &registry, const FrameContext &frame)
    {
        auto players = registry.view<PlayerTag, Position, Velocity, CircleCollider, DamageCooldown>();
        for (auto entity : players)
        {
            auto &position = players.get<Position>(entity);
            auto &velocity = players.get<Velocity>(entity);
            const auto &collider = players.get<CircleCollider>(entity);
            auto &cooldown = players.get<DamageCooldown>(entity);

            const Vector2 input = ReadPlayerMoveInput();
            velocity.value = Vector2{input.x * 260.0f, input.y * 260.0f};

            position.value.x += velocity.value.x * frame.dt;
            position.value.y += velocity.value.y * frame.dt;

            position.value.x = std::clamp(position.value.x, collider.radius, frame.width - collider.radius);
            position.value.y = std::clamp(position.value.y, collider.radius, frame.height - collider.radius);

            cooldown.seconds = std::max(0.0f, cooldown.seconds - frame.dt);
        }
    }

    void SpawnPlayerProjectiles(entt::registry &registry, const FrameContext &frame)
    {
        auto *tuningPtr = registry.ctx().find<SpellTuning>();
        auto *cooldownPtr = registry.ctx().find<FireCooldown>();
        if (tuningPtr == nullptr || cooldownPtr == nullptr)
        {
            return;
        }

        auto &tuning = *tuningPtr;
        auto &cooldown = *cooldownPtr;
        cooldown.seconds = std::max(0.0f, cooldown.seconds - frame.dt);

        const bool firePressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE);
        if (!firePressed || cooldown.seconds > 0.0f)
        {
            return;
        }

        auto players = registry.view<PlayerTag, Position, CircleCollider>();
        for (auto player : players)
        {
            const auto &playerPos = players.get<Position>(player);
            const auto &playerCol = players.get<CircleCollider>(player);

            Vector2 target = GetMousePosition();
            Vector2 toTarget = Vector2{target.x - playerPos.value.x, target.y - playerPos.value.y};
            Vector2 forward = NormalizeOrFallback(toTarget, Vector2{0.0f, -1.0f});

            const int count = std::clamp(tuning.projectileCount, 1, 12);
            const float totalSpread = kDefaultSpreadRadians + (0.02f * static_cast<float>(count - 1));
            const float startAngle = -totalSpread * 0.5f;
            const float step = (count > 1) ? (totalSpread / static_cast<float>(count - 1)) : 0.0f;

            for (int i = 0; i < count; ++i)
            {
                const float angle = startAngle + (step * static_cast<float>(i));
                Vector2 dir = RotateVector(forward, angle);
                Vector2 spawnPos = Vector2{
                    playerPos.value.x + (dir.x * (playerCol.radius + tuning.projectileSize + 2.0f)),
                    playerPos.value.y + (dir.y * (playerCol.radius + tuning.projectileSize + 2.0f))};
                SpawnProjectileEntity(registry, spawnPos, dir, tuning);
            }

            cooldown.seconds = kMaxPlayerFireRate;
            break;
        }
    }

    void UpdateProjectiles(entt::registry &registry, const FrameContext &frame)
    {
        auto projectiles = registry.view<ProjectileTag, Position, Velocity, CircleCollider, ProjectileRicochet>();
        std::vector<entt::entity> toDestroy;

        for (auto entity : projectiles)
        {
            auto &pos = projectiles.get<Position>(entity);
            auto &vel = projectiles.get<Velocity>(entity);
            const auto &col = projectiles.get<CircleCollider>(entity);
            auto &ricochet = projectiles.get<ProjectileRicochet>(entity);

            pos.value.x += vel.value.x * frame.dt;
            pos.value.y += vel.value.y * frame.dt;

            bool bounced = false;

            if (pos.value.x < col.radius)
            {
                pos.value.x = col.radius;
                vel.value.x = std::fabs(vel.value.x);
                bounced = true;
            }
            else if (pos.value.x > frame.width - col.radius)
            {
                pos.value.x = frame.width - col.radius;
                vel.value.x = -std::fabs(vel.value.x);
                bounced = true;
            }

            if (pos.value.y < col.radius)
            {
                pos.value.y = col.radius;
                vel.value.y = std::fabs(vel.value.y);
                bounced = true;
            }
            else if (pos.value.y > frame.height - col.radius)
            {
                pos.value.y = frame.height - col.radius;
                vel.value.y = -std::fabs(vel.value.y);
                bounced = true;
            }

            if (bounced)
            {
                if (ricochet.bouncesLeft > 0)
                {
                    ricochet.bouncesLeft -= 1;
                }
                else
                {
                    toDestroy.push_back(entity);
                }
            }
        }

        for (entt::entity entity : toDestroy)
        {
            if (registry.valid(entity))
            {
                registry.destroy(entity);
            }
        }
    }

    void ResolveProjectileEnemyCollisions(entt::registry &registry)
    {
        auto projectiles = registry.view<ProjectileTag, Position, CircleCollider, ProjectileDamage, ProjectileRicochet>();
        auto enemies = registry.view<EnemyTag, Position, CircleCollider>();

        std::vector<entt::entity> projectileDestroy;
        std::vector<entt::entity> enemyDestroy;

        for (auto projectile : projectiles)
        {
            auto &projPos = projectiles.get<Position>(projectile);
            const auto &projCol = projectiles.get<CircleCollider>(projectile);
            auto &ricochet = projectiles.get<ProjectileRicochet>(projectile);
            bool hitSomething = false;

            for (auto enemy : enemies)
            {
                const auto &enemyPos = enemies.get<Position>(enemy);
                const auto &enemyCol = enemies.get<CircleCollider>(enemy);
                if (!CheckCollisionCircles(projPos.value, projCol.radius, enemyPos.value, enemyCol.radius))
                {
                    continue;
                }

                enemyDestroy.push_back(enemy);
                hitSomething = true;
                if (ricochet.bouncesLeft > 0)
                {
                    ricochet.bouncesLeft -= 1;
                }
                else
                {
                    projectileDestroy.push_back(projectile);
                }
                break;
            }

            if (hitSomething && ricochet.bouncesLeft > 0)
            {
                auto *vel = registry.try_get<Velocity>(projectile);
                if (vel != nullptr)
                {
                    vel->value = RotateVector(vel->value, 0.25f);
                }
            }
        }

        for (entt::entity enemy : enemyDestroy)
        {
            if (registry.valid(enemy))
            {
                registry.destroy(enemy);
            }
        }

        for (entt::entity projectile : projectileDestroy)
        {
            if (registry.valid(projectile))
            {
                registry.destroy(projectile);
            }
        }
    }

    bool HasDefeatedPlayer(entt::registry &registry)
    {
        auto players = registry.view<PlayerTag, Health>();
        for (auto entity : players)
        {
            if (players.get<Health>(entity).hp <= 0)
            {
                return true;
            }
        }

        return false;
    }

    void DrawDefeatOverlay(const FrameContext &frame)
    {
        const char *defeatedText = "You Died!";
        const int fontSize = 28;
        const int textWidth = MeasureText(defeatedText, fontSize);
        const int x = static_cast<int>((frame.width - static_cast<float>(textWidth)) * 0.5f);
        const int y = static_cast<int>((frame.height - static_cast<float>(fontSize)) * 0.5f);
        DrawText(defeatedText, x, y, fontSize, ORANGE);
    }

    bool IsAnyReturnInputPressed()
    {
        return (GetKeyPressed() != 0) ||
               IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
               IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
               IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) ||
               IsAnyGamepadButtonPressed();
    }

    void UpdateEnemies(entt::registry &registry, const FrameContext &frame)
    {
        auto enemies = registry.view<EnemyTag, Position, Velocity, CircleCollider>();
        for (auto entity : enemies)
        {
            auto &position = enemies.get<Position>(entity);
            auto &velocity = enemies.get<Velocity>(entity);
            const auto &collider = enemies.get<CircleCollider>(entity);

            position.value.x += velocity.value.x * frame.dt;
            if (position.value.x < collider.radius)
            {
                position.value.x = collider.radius;
                velocity.value.x = std::fabs(velocity.value.x);
            }
            else if (position.value.x > frame.width - collider.radius)
            {
                position.value.x = frame.width - collider.radius;
                velocity.value.x = -std::fabs(velocity.value.x);
            }
        }
    }

    void ResolveEnemyPlayerCollisions(entt::registry &registry)
    {
        auto players = registry.view<PlayerTag, Position, CircleCollider, Health, DamageCooldown>();
        auto enemies = registry.view<EnemyTag, Position, CircleCollider>();

        for (auto playerEntity : players)
        {
            auto &playerPos = players.get<Position>(playerEntity);
            const auto &playerCol = players.get<CircleCollider>(playerEntity);
            auto &health = players.get<Health>(playerEntity);
            auto &cooldown = players.get<DamageCooldown>(playerEntity);

            if (cooldown.seconds > 0.0f)
            {
                continue;
            }

            bool tookDamage = false;
            for (auto enemyEntity : enemies)
            {
                const auto &enemyPos = enemies.get<Position>(enemyEntity);
                const auto &enemyCol = enemies.get<CircleCollider>(enemyEntity);
                if (CheckCollisionCircles(playerPos.value, playerCol.radius, enemyPos.value, enemyCol.radius))
                {
                    tookDamage = true;
                    break;
                }
            }

            if (tookDamage)
            {
                health.hp = std::max(0, health.hp - 1);
                cooldown.seconds = 0.65f;
            }
        }
    }

    void DrawEnemies(entt::registry &registry)
    {
        auto enemies = registry.view<EnemyTag, Position, CircleCollider>();
        for (auto entity : enemies)
        {
            const auto &pos = enemies.get<Position>(entity);
            const auto &col = enemies.get<CircleCollider>(entity);
            DrawCircleV(pos.value, col.radius, Color{222, 72, 86, 255});
        }
    }

    void DrawProjectiles(entt::registry &registry)
    {
        auto projectiles = registry.view<ProjectileTag, Position, CircleCollider>();
        for (auto entity : projectiles)
        {
            const auto &pos = projectiles.get<Position>(entity);
            const auto &col = projectiles.get<CircleCollider>(entity);
            DrawCircleV(pos.value, col.radius, Color{255, 210, 86, 255});
        }
    }

    void DrawPlayersAndHud(entt::registry &registry)
    {
        auto players = registry.view<PlayerTag, Position, CircleCollider, Health, DamageCooldown>();
        for (auto entity : players)
        {
            const auto &pos = players.get<Position>(entity);
            const auto &col = players.get<CircleCollider>(entity);
            const auto &health = players.get<Health>(entity);
            const auto &cooldown = players.get<DamageCooldown>(entity);

            Color playerColor = Color{74, 183, 255, 255};
            if (cooldown.seconds > 0.0f)
            {
                playerColor = Color{255, 203, 95, 255};
            }
            DrawCircleV(pos.value, col.radius, playerColor);

            DrawText(TextFormat("HP: %d", health.hp), 24, 24, 30, RAYWHITE);
            DrawText("Move: WASD or Arrows", 24, 60, 20, LIGHTGRAY);
            DrawText("Shoot: Mouse Left or Space", 24, 84, 20, LIGHTGRAY);
        }
    }
} // namespace GameEcs
