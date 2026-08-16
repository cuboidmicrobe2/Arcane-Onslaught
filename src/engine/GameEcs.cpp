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
        int projectilesFired = 0;  // Tracks how many projectiles have been fired in current sequence
        float staggerTimer = 0.0f; // Accumulates time for staggered firing
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

} // namespace

namespace GameEcs
{
    entt::entity ImportEntityFromTable(entt::registry &registry, lua_State *L, int entityIndex)
    {
        const int tableIndex = lua_absindex(L, entityIndex);
        if (!lua_istable(L, tableIndex))
        {
            return entt::null;
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

        float iFrameSeconds = 0.0f;
        if (lua_getfield(L, tableIndex, "i_frames") == LUA_TTABLE)
        {
            if (ReadFloatField(L, -1, "seconds", iFrameSeconds))
            {
                registry.emplace<GameEcs::IFrames>(entity, iFrameSeconds);
            }
        }
        lua_pop(L, 1);

        bool flag = false;
        if (ReadBoolField(L, tableIndex, "spell_tag", flag) && flag)
        {
            registry.emplace<GameEcs::SpellTag>(entity);
        }
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

        Color projectileColor = Color{255, 210, 86, 255};
        if (lua_getfield(L, tableIndex, "projectile_color") == LUA_TTABLE)
        {
            float r = 1.0f, g = 0.82f, b = 0.33f;
            ReadFloatField(L, -1, "r", r);
            ReadFloatField(L, -1, "g", g);
            ReadFloatField(L, -1, "b", b);
            projectileColor = Color{
                static_cast<unsigned char>(std::clamp(r, 0.0f, 1.0f) * 255.0f),
                static_cast<unsigned char>(std::clamp(g, 0.0f, 1.0f) * 255.0f),
                static_cast<unsigned char>(std::clamp(b, 0.0f, 1.0f) * 255.0f),
                255};
            registry.emplace<GameEcs::ProjectileColor>(entity, projectileColor);
        }
        lua_pop(L, 1);

        float width = 0.0f, height = 0.0f;
        if (lua_getfield(L, tableIndex, "rect_collider") == LUA_TTABLE)
        {
            if (ReadFloatField(L, -1, "width", width) && ReadFloatField(L, -1, "height", height))
            {
                registry.emplace<GameEcs::RectCollider>(entity, width, height);
            }
        }
        lua_pop(L, 1);

        if (ReadBoolField(L, tableIndex, "wall_tag", flag) && flag)
        {
            registry.emplace<GameEcs::WallTag>(entity);
        }

        return entity;
    }

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
            registry.emplace<Rotation>(projectile, 0.0f);
            registry.emplace<CircleCollider>(projectile, tuning.projectileSize * 0.5f); // Smaller collision radius
            registry.emplace<ProjectileColor>(projectile, tuning.projectileColor);
            registry.emplace<ProjectileDamage>(projectile, std::max(1, tuning.damage));
            registry.emplace<ProjectileRicochet>(projectile, tuning.ricochet ? 1 : 0);
        }

        // Helper to resolve circle-rectangle collision by pushing circle out
        void ResolveCircleRectCollision(Vector2 &circlePos, float circleRadius, const Vector2 &rectPos, float rectWidth, float rectHeight)
        {
            float rectLeft = rectPos.x - rectWidth * 0.5f;
            float rectRight = rectPos.x + rectWidth * 0.5f;
            float rectTop = rectPos.y - rectHeight * 0.5f;
            float rectBottom = rectPos.y + rectHeight * 0.5f;

            // Find closest point on rect to circle center
            float closestX = std::clamp(circlePos.x, rectLeft, rectRight);
            float closestY = std::clamp(circlePos.y, rectTop, rectBottom);

            float dx = circlePos.x - closestX;
            float dy = circlePos.y - closestY;
            float distSq = dx * dx + dy * dy;
            float minDist = circleRadius;

            if (distSq < minDist * minDist && distSq > 0.0001f)
            {
                float dist = std::sqrt(distSq);
                float overlap = minDist - dist;
                float pushX = (dx / dist) * overlap;
                float pushY = (dy / dist) * overlap;
                circlePos.x += pushX;
                circlePos.y += pushY;
            }
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
                    entt::entity e = ImportEntityFromTable(registry, L, -1);
                    (void)e; // Suppress unused warning
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
        auto walls = registry.view<WallTag, Position, RectCollider>();

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

            // Resolve wall collisions
            for (auto wallEntity : walls)
            {
                const auto &wallPos = walls.get<Position>(wallEntity);
                const auto &wallCol = walls.get<RectCollider>(wallEntity);
                ResolveCircleRectCollision(position.value, collider.radius, wallPos.value, wallCol.width, wallCol.height);
            }

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

        // If stagger delay is active and we're already firing a sequence, continue the sequence
        if (tuning.staggerDelay > 0.0f && cooldown.projectilesFired > 0)
        {
            cooldown.staggerTimer += frame.dt;
            if (cooldown.staggerTimer >= tuning.staggerDelay && cooldown.projectilesFired < tuning.projectileCount)
            {
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

                    // Spawn only the next projectile in the sequence
                    const int i = cooldown.projectilesFired;
                    const float angle = startAngle + (step * static_cast<float>(i));
                    Vector2 dir = RotateVector(forward, angle);
                    Vector2 spawnPos = Vector2{
                        playerPos.value.x + (dir.x * (playerCol.radius + tuning.projectileSize + 2.0f)),
                        playerPos.value.y + (dir.y * (playerCol.radius + tuning.projectileSize + 2.0f))};
                    SpawnProjectileEntity(registry, spawnPos, dir, tuning);

                    cooldown.projectilesFired++;
                    cooldown.staggerTimer = 0.0f;
                    break;
                }
            }

            // If all projectiles fired, reset for next fire event
            if (cooldown.projectilesFired >= tuning.projectileCount)
            {
                cooldown.seconds = kMaxPlayerFireRate * static_cast<float>(tuning.projectileCount);
                cooldown.projectilesFired = 0;
                cooldown.staggerTimer = 0.0f;
            }
            return;
        }

        // Normal (non-staggered) firing
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

            if (tuning.staggerDelay > 0.0f)
            {
                // Start staggered sequence: fire first projectile
                const float angle = startAngle;
                Vector2 dir = RotateVector(forward, angle);
                Vector2 spawnPos = Vector2{
                    playerPos.value.x + (dir.x * (playerCol.radius + tuning.projectileSize + 2.0f)),
                    playerPos.value.y + (dir.y * (playerCol.radius + tuning.projectileSize + 2.0f))};
                SpawnProjectileEntity(registry, spawnPos, dir, tuning);

                cooldown.projectilesFired = 1;
                cooldown.staggerTimer = 0.0f;
            }
            else
            {
                // Spawn all projectiles at once (no stagger)
                for (int i = 0; i < count; ++i)
                {
                    const float angle = startAngle + (step * static_cast<float>(i));
                    Vector2 dir = RotateVector(forward, angle);
                    Vector2 spawnPos = Vector2{
                        playerPos.value.x + (dir.x * (playerCol.radius + tuning.projectileSize + 2.0f)),
                        playerPos.value.y + (dir.y * (playerCol.radius + tuning.projectileSize + 2.0f))};
                    SpawnProjectileEntity(registry, spawnPos, dir, tuning);
                }
            }

            cooldown.seconds = kMaxPlayerFireRate * static_cast<float>(count);
            break;
        }
    }

    void UpdateProjectiles(entt::registry &registry, const FrameContext &frame)
    {
        auto projectiles = registry.view<ProjectileTag, Position, Velocity, CircleCollider, ProjectileRicochet, Rotation>();
        std::vector<entt::entity> toDestroy;

        for (auto entity : projectiles)
        {
            auto &pos = projectiles.get<Position>(entity);
            auto &vel = projectiles.get<Velocity>(entity);
            const auto &col = projectiles.get<CircleCollider>(entity);
            auto &ricochet = projectiles.get<ProjectileRicochet>(entity);
            auto &rot = projectiles.get<Rotation>(entity);

            pos.value.x += vel.value.x * frame.dt;
            pos.value.y += vel.value.y * frame.dt;

            // Rotate projectile
            rot.angle += 12.0f * frame.dt; // Rotate ~12 radians per second
            if (rot.angle > 2.0f * 3.14159f)
            {
                rot.angle -= 2.0f * 3.14159f;
            }

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
        auto enemies = registry.view<EnemyTag, Position, CircleCollider, Health>();

        std::vector<entt::entity> projectileDestroy;
        std::vector<entt::entity> enemyDestroy;

        for (auto projectile : projectiles)
        {
            auto &projPos = projectiles.get<Position>(projectile);
            const auto &projCol = projectiles.get<CircleCollider>(projectile);
            const auto &projDamage = projectiles.get<ProjectileDamage>(projectile);
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

                // Apply damage to enemy
                auto &enemyHealth = enemies.get<Health>(enemy);
                enemyHealth.hp -= projDamage.value;

                // Destroy enemy if health reaches 0 or below
                if (enemyHealth.hp <= 0)
                {
                    enemyDestroy.push_back(enemy);
                }

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

    void ResolveProjectileWallCollisions(entt::registry &registry)
    {
        auto projectiles = registry.view<ProjectileTag, Position, CircleCollider, Velocity, ProjectileRicochet>();
        auto walls = registry.view<WallTag, Position, RectCollider>();

        std::vector<entt::entity> projectileDestroy;

        for (auto projectile : projectiles)
        {
            auto &projPos = projectiles.get<Position>(projectile);
            const auto &projCol = projectiles.get<CircleCollider>(projectile);
            auto &projVel = projectiles.get<Velocity>(projectile);
            auto &ricochet = projectiles.get<ProjectileRicochet>(projectile);

            for (auto wall : walls)
            {
                const auto &wallPos = walls.get<Position>(wall);
                const auto &wallCol = walls.get<RectCollider>(wall);

                float rectLeft = wallPos.value.x - wallCol.width * 0.5f;
                float rectRight = wallPos.value.x + wallCol.width * 0.5f;
                float rectTop = wallPos.value.y - wallCol.height * 0.5f;
                float rectBottom = wallPos.value.y + wallCol.height * 0.5f;

                // Find closest point on rect to circle center
                float closestX = std::clamp(projPos.value.x, rectLeft, rectRight);
                float closestY = std::clamp(projPos.value.y, rectTop, rectBottom);

                float dx = projPos.value.x - closestX;
                float dy = projPos.value.y - closestY;
                float distSq = dx * dx + dy * dy;
                float minDist = projCol.radius;

                if (distSq < minDist * minDist && distSq > 0.0001f)
                {
                    // Collision detected
                    if (ricochet.bouncesLeft > 0)
                    {
                        // Bounce off the wall
                        float dist = std::sqrt(distSq);
                        float normalX = dx / dist;
                        float normalY = dy / dist;

                        // Reflect velocity: v' = v - 2(v·n)n
                        float dotProduct = projVel.value.x * normalX + projVel.value.y * normalY;
                        projVel.value.x = projVel.value.x - 2.0f * dotProduct * normalX;
                        projVel.value.y = projVel.value.y - 2.0f * dotProduct * normalY;

                        ricochet.bouncesLeft -= 1;

                        // Push projectile out of wall
                        float overlap = minDist - dist;
                        projPos.value.x += normalX * overlap;
                        projPos.value.y += normalY * overlap;
                    }
                    else
                    {
                        // Non-ricochet projectile: destroy on wall impact
                        projectileDestroy.push_back(projectile);
                    }
                    break; // Only collide with one wall per frame
                }
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
        const int fontSize = 72;
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
        // Get player position for dynamic following
        auto players = registry.view<PlayerTag, Position>();
        Vector2 playerPos{0.0f, 0.0f};
        bool playerExists = false;

        for (auto entity : players)
        {
            playerPos = players.get<Position>(entity).value;
            playerExists = true;
            break;
        }

        auto enemies = registry.view<EnemyTag, Position, Velocity, CircleCollider>();
        auto walls = registry.view<WallTag, Position, RectCollider>();

        for (auto entity : enemies)
        {
            auto &position = enemies.get<Position>(entity);
            auto &velocity = enemies.get<Velocity>(entity);
            const auto &collider = enemies.get<CircleCollider>(entity);

            // Dynamically update velocity to follow player
            if (playerExists)
            {
                float dx = playerPos.x - position.value.x;
                float dy = playerPos.y - position.value.y;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist > 0.1f)
                {
                    // Get current speed magnitude
                    float currentSpeed = std::sqrt(velocity.value.x * velocity.value.x + velocity.value.y * velocity.value.y);
                    if (currentSpeed < 0.1f)
                    {
                        currentSpeed = 150.0f; // Default speed
                    }

                    // Normalize direction and apply speed
                    velocity.value.x = (dx / dist) * currentSpeed;
                    velocity.value.y = (dy / dist) * currentSpeed;
                }
            }

            position.value.x += velocity.value.x * frame.dt;
            position.value.y += velocity.value.y * frame.dt;

            // Boundary collision for X axis
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

            // Boundary collision for Y axis
            if (position.value.y < collider.radius)
            {
                position.value.y = collider.radius;
                velocity.value.y = std::fabs(velocity.value.y);
            }
            else if (position.value.y > frame.height - collider.radius)
            {
                position.value.y = frame.height - collider.radius;
                velocity.value.y = -std::fabs(velocity.value.y);
            }

            // Resolve wall collisions
            for (auto wallEntity : walls)
            {
                const auto &wallPos = walls.get<Position>(wallEntity);
                const auto &wallCol = walls.get<RectCollider>(wallEntity);
                ResolveCircleRectCollision(position.value, collider.radius, wallPos.value, wallCol.width, wallCol.height);
            }
        }
    }

    void UpdateIFrames(entt::registry &registry, const FrameContext &frame)
    {
        auto players = registry.view<PlayerTag, IFrames>();

        for (auto entity : players)
        {
            auto &iFrames = players.get<IFrames>(entity);
            iFrames.seconds = std::max(0.0f, iFrames.seconds - frame.dt);
        }
    }

    void UpdateBehaviors(entt::registry &registry, lua_State *L, const FrameContext &frame)
    {
        if (L == nullptr)
        {
            return;
        }

        auto behaviors = registry.view<Behavior>();
        for (auto entity : behaviors)
        {
            auto &behavior = behaviors.get<Behavior>(entity);

            // Get co-routine from Lua registry
            lua_rawgeti(L, LUA_REGISTRYINDEX, behavior.coroutineRef);
            if (lua_isthread(L, -1))
            {
                lua_State *thread = lua_tothread(L, -1);
                int status = lua_status(thread);

                if (status == LUA_OK || status == LUA_YIELD)
                {
                    // Resume the co-routine
                    int nresults = 0;
                    int result = lua_resume(thread, L, 0, &nresults);

                    if (result == LUA_ERRRUN)
                    {
                        const char *err = lua_tostring(thread, -1);
                        fprintf(stderr, "Behavior co-routine error: %s\n", err != nullptr ? err : "Unknown error");
                        lua_pop(thread, 1);
                        // Unref the dead co-routine
                        luaL_unref(L, LUA_REGISTRYINDEX, behavior.coroutineRef);
                        behavior.coroutineRef = LUA_NOREF;
                        registry.erase<Behavior>(entity);
                    }
                    else if (result != LUA_YIELD)
                    {
                        // Co-routine finished, clean up the Behavior component (not the entity!)
                        luaL_unref(L, LUA_REGISTRYINDEX, behavior.coroutineRef);
                        behavior.coroutineRef = LUA_NOREF;
                        registry.erase<Behavior>(entity);
                    }
                }
            }
            lua_pop(L, 1);
        }
    }

    void ResolveEnemyPlayerCollisions(entt::registry &registry, lua_State *L)
    {
        auto players = registry.view<PlayerTag, Position, CircleCollider, Health, DamageCooldown, IFrames>();
        auto enemies = registry.view<EnemyTag, Position, CircleCollider>();

        for (auto playerEntity : players)
        {
            auto &playerPos = players.get<Position>(playerEntity);
            const auto &playerCol = players.get<CircleCollider>(playerEntity);
            auto &health = players.get<Health>(playerEntity);
            auto &cooldown = players.get<DamageCooldown>(playerEntity);
            auto &iFrames = players.get<IFrames>(playerEntity);

            if (cooldown.seconds > 0.0f || iFrames.seconds > 0.0f)
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
                iFrames.seconds = 1.5f;
            }
        }
    }

    void DrawEnemies(entt::registry &registry)
    {
        auto enemies = registry.view<EnemyTag, Position, CircleCollider, Health>();
        for (auto entity : enemies)
        {
            const auto &pos = enemies.get<Position>(entity);
            const auto &col = enemies.get<CircleCollider>(entity);
            const auto &health = enemies.get<Health>(entity);
            DrawCircleV(pos.value, col.radius, Color{222, 72, 86, 255});

            // Draw health above enemy
            const char *healthText = TextFormat("%d", health.hp);
            int textWidth = MeasureText(healthText, 20);
            DrawText(healthText, static_cast<int>(pos.value.x - textWidth * 0.5f), static_cast<int>(pos.value.y - col.radius - 28), 20, RAYWHITE);
        }
    }

    void DrawProjectiles(entt::registry &registry)
    {
        auto projectiles = registry.view<ProjectileTag, Position, CircleCollider, Rotation, ProjectileColor>();
        for (auto entity : projectiles)
        {
            const auto &pos = projectiles.get<Position>(entity);
            const auto &col = projectiles.get<CircleCollider>(entity);
            const auto &rot = projectiles.get<Rotation>(entity);
            const auto &projColor = projectiles.get<ProjectileColor>(entity);

            Vector2 center = pos.value;
            float radius = col.radius;
            float size = radius * 2.0f;

            // Create glow and line colors based on projectile color
            Color glowColor{
                static_cast<unsigned char>(projColor.value.r),
                static_cast<unsigned char>(projColor.value.g),
                static_cast<unsigned char>(projColor.value.b),
                80 // Semi-transparent outer glow
            };
            Color glowColorMid{
                static_cast<unsigned char>(projColor.value.r),
                static_cast<unsigned char>(projColor.value.g),
                static_cast<unsigned char>(projColor.value.b),
                120 // More opaque middle glow
            };
            Color quadColor = projColor.value; // Full color for quad
            Color lineColor{
                static_cast<unsigned char>(projColor.value.r),
                static_cast<unsigned char>(projColor.value.g),
                static_cast<unsigned char>(projColor.value.b),
                150 // Line color with slight transparency
            };

            // Glow offset - up and left (larger offset for visibility)
            Vector2 glowCenter = Vector2{center.x - radius * 0.8f, center.y - radius * 0.8f};

            // Outer glow ring (semi-transparent)
            DrawCircleV(glowCenter, radius * 1.8f, glowColor);

            // Middle glow ring (smaller, more opaque)
            DrawCircleV(glowCenter, radius * 1.2f, glowColorMid);

            // Inner rotating quad - use radius directly for precise centering
            Rectangle rect{center.x - radius, center.y - radius, size, size};
            DrawRectanglePro(rect, Vector2{radius, radius}, rot.angle * 57.2958f, quadColor);

            // Draw radiating lines for magical effect
            const float lineLength = radius * 1.5f;
            const int numLines = 4;
            for (int i = 0; i < numLines; ++i)
            {
                float angle = (rot.angle + (i * 3.14159f * 0.5f));
                float startX = center.x + std::cos(angle) * radius * 0.8f;
                float startY = center.y + std::sin(angle) * radius * 0.8f;
                float endX = center.x + std::cos(angle) * lineLength;
                float endY = center.y + std::sin(angle) * lineLength;
                DrawLineEx(Vector2{startX, startY}, Vector2{endX, endY}, 2.0f, lineColor);
            }
        }
    }

    void DrawWalls(entt::registry &registry)
    {
        auto walls = registry.view<WallTag, Position, RectCollider>();
        for (auto entity : walls)
        {
            const auto &pos = walls.get<Position>(entity);
            const auto &col = walls.get<RectCollider>(entity);
            float x = pos.value.x - col.width * 0.5f;
            float y = pos.value.y - col.height * 0.5f;
            DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(col.width), static_cast<int>(col.height), Color{150, 150, 150, 255});
            DrawRectangleLines(static_cast<int>(x), static_cast<int>(y), static_cast<int>(col.width), static_cast<int>(col.height), Color{200, 200, 200, 255});
        }
    }

    void DrawPlayersAndHud(entt::registry &registry)
    {
        auto players = registry.view<PlayerTag, Position, CircleCollider, Health, DamageCooldown, IFrames>();
        for (auto entity : players)
        {
            const auto &pos = players.get<Position>(entity);
            const auto &col = players.get<CircleCollider>(entity);
            const auto &health = players.get<Health>(entity);
            const auto &cooldown = players.get<DamageCooldown>(entity);
            const auto &iFrames = players.get<IFrames>(entity);

            Color playerColor = Color{74, 183, 255, 255};

            // Show damage cooldown color
            if (cooldown.seconds > 0.0f)
            {
                playerColor = Color{255, 203, 95, 255};
            }

            // Flash effect during i-frames - alternate visibility
            if (iFrames.seconds > 0.0f)
            {
                // Flash at 5Hz (0.2s interval)
                float flashCycle = std::fmod(iFrames.seconds, 0.2f);
                if (flashCycle < 0.1f)
                {
                    // Make player semi-transparent during flash
                    playerColor.a = 128;
                }
            }

            DrawCircleV(pos.value, col.radius, playerColor);

            DrawText(TextFormat("HP: %d", health.hp), 24, 24, 30, RAYWHITE);
            DrawText("Move: WASD or Arrows", 24, 60, 20, LIGHTGRAY);
            DrawText("Shoot: Mouse Left or Space", 24, 84, 20, LIGHTGRAY);
        }
    }
} // namespace GameEcs
