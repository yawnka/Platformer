#include "LevelB.h"
#include "Utility.h"
#include "Scene.h"
#include "Scene.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/player0.png",
           ENEMY_FILEPATH[]       = "assets/enemy.png";

unsigned int LEVELB_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2,
    3, 1, 1, 1, 1, 1, 1, 0, 1, 2, 2, 2, 2, 2,
    3, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2
};

LevelB::~LevelB()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelB::initialise()
{
    m_game_state.next_scene_id = -1;
    m_game_state.enemies_defeated = 0;
    
    GLuint map_texture_id = Utility::load_texture("assets/tileset_1.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f,3, 1);
    
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    // Existing
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    int player_walking_animation[4][4] =
    {
        { 0, 1, 2, 3 },  // for PLAYER to move to the left,
        { 4, 5, 6, 7 }, // for PLAYER to move to the right,
        { 8, 9, 10, 11 }, // for PLAYER to move upwards,
        { 12, 13, 14, 15 }   // for PLAYER to move downwards
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
    
    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        3.0f,                      // speed
        acceleration,              // acceleration
        4.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        4,                         // animation row amount
        0.65f,                      // width
        0.65f,                      // height
        PLAYER
    );
    
    m_game_state.player->m_visual_scale = 2.0f; // scaling player
        
    m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));

    // Jumping
    m_game_state.player->set_jumping_power(4.0f);
    
    /**
    Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    
    int enemy_walking_animation[4][4] = {
            {8, 9, 10, 11}, // Left
            {4, 5, 6, 7},   // Right
            {0, 1, 2, 3}, // Up
            {12, 13, 14, 15} // Down
    };

    m_game_state.enemies = new Entity[ENEMY_COUNT];
    glm::vec3 enemy_acceleration = glm::vec3(0.0f, -2.905f, 0.0f);
    
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
    m_game_state.enemies[i] =  Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    }
    
    for (int i = 0; i < ENEMY_COUNT; ++i) {
        m_game_state.enemies[i] = Entity(
            enemy_texture_id,          // texture id
            2.0f,                      // speed
            enemy_acceleration,        // acceleration
            1.0f,                      // jumping power (or adjust as needed)
            enemy_walking_animation,   // animation frames
            0.0f,                      // animation time
            4,                         // animation frame amount
            0,                         // current animation index
            4,                         // animation column amount
            4,                         // animation row amount
            0.65f,                     // width
            0.65f,                     // height
            ENEMY                      // type
        );
        m_game_state.enemies[i].m_visual_scale = 1.0f; // scale of enemies
    }


    m_game_state.enemies[0].set_position(glm::vec3(3.0f, -5.0f, 0.0f));
    m_game_state.enemies[0].set_ai_type(JUMPER);
    m_game_state.enemies[0].set_ai_state(JUMPING);
    m_game_state.enemies[0].set_jumping_power(5.0f);
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/dooblydoo.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(0.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelB::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (!m_game_state.enemies[i].is_active()) continue;
        
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);

        if (m_game_state.player->check_head_collision(&m_game_state.enemies[i]))
        {
            // Deactivate the enemy
            m_game_state.enemies[i].deactivate();

            // Bounce the player
            m_game_state.player->set_velocity(glm::vec3(m_game_state.player->get_velocity().x, 4.0f, 0.0f));

            // Increment local and global enemies_defeated counters
            m_game_state.enemies_defeated++;
            g_total_enemies_defeated++;

            Mix_PlayChannel(-1, m_game_state.jump_sfx, 0); // Play jump SFX
        }

    }
    if (m_game_state.player->get_position().y < -10.0f) m_game_state.next_scene_id = 1;
    
    if (g_total_enemies_defeated == g_total_enemies)
    {
        std::cout << "All enemies defeated! Pausing game." << std::endl;  // Debugging output
        g_app_status = PAUSED;
    }

}

void LevelB::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    for (int i = 0; i < m_number_of_enemies; i++)
    {
        if (m_game_state.enemies[i].is_active())
        {
            m_game_state.enemies[i].render(program);
        }
    }
}
